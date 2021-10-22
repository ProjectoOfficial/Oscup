/*
* Oscup: Open Source Custom Uart Protocol
* This Software was release under: GPL-3.0 License
* Copyright � 2021 Daniel Rossi & Riccardo Salami
* Version: ALPHA 0.1.1
*/

#include "oscup.h"

Oscup::Oscup(uint8_t id, uint32_t baudrate) {
    /* @brief initializes the UART of ESP32 by defining the hardware port and hardware pins
    *         and other UART's parameters
    *  
    *  @param    id of this device
    *  @param    communication baudrate
    */

    _id = id;
    _baudrate = baudrate;

    // UART init
    uart_port = uart_port_t::UART_NUM_0;
    uart_rxd_pin = UART_RXD_PIN;
    uart_txd_pin = UART_TXD_PIN;
    uart_rts_pin = UART_RTS_PIN;
    uart_rts_pin = UART_CTS_PIN;
    intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    _uart_config = {
        .baud_rate = baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    // TIMER init
    _timer_info.group = TIMER_GROUP_1;
    _timer_info.index = TIMER_1;
    _timer_info.auto_reload = true;
    _timer_info.alarm_value = 0;
    tim_init(TIMER_PRESCALER_80MHZ); //APB should be 80MHz
}

void Oscup::begin(){
    uart_driver_install(uart_port, MAX_PAYLOAD_LENGTH + 1, MAX_PAYLOAD_LENGTH + 1, 0, NULL, intr_alloc_flags);
    uart_param_config(uart_port, &_uart_config);
    uart_set_pin(uart_port, uart_txd_pin, uart_rxd_pin, uart_rts_pin, uart_cts_pin);
}


void Oscup::tim_init(int prescaler){
    /* @brief initializes the timer and its variables
    *   
    *  @param prescaler used for APB frequency scaling
    */

    _timer_config.divider = prescaler;
    _timer_config.counter_dir = TIMER_COUNT_UP;
    _timer_config.counter_en = true;
    _timer_config.alarm_en = true;
    _timer_config.auto_reload = _timer_info.auto_reload;

    timer_init(_timer_info.group, _timer_info.index, &_timer_config);

    timer_set_counter_value(_timer_info.group, _timer_info.index, 0);
    timer_set_alarm_value(_timer_info.group, _timer_info.index, _timer_info.alarm_value);
    timer_set_alarm(_timer_info.group, _timer_info.index, TIMER_ALARM_EN);
    timer_set_counter_value(_timer_info.group, _timer_info.index, 0);
    timer_start(_timer_info.group, _timer_info.index);

}


uint8_t Oscup::testWrite() {
    /*  @brief test write function which is usefull to see if dependencies are working
    *
    *   @return it returns an error code about write
    */

    uart_write_bytes(uart_port, (const char*)"prova\r\n", 8);
    return (uint8_t)ErrorCodes::OK;
}


uint8_t Oscup::write(uint8_t command, uint8_t length, char* payload) {
    /* @brief Writes data on Uart. 
    *         If ACK does not arrive, it will retry to send data again
    *         If NACK arrives, it will resend and delay the resend's stop time
    * 
    * @param command it is command to execute on receiver
    * @param length payload length
    * @param payload the payload buffer
    * 
    * @return it returns feedback on writing result
    */

    if (length > MAX_PAYLOAD_LENGTH)
        return (uint8_t)ErrorCodes::LENGTH_ERROR;

    if(payload == NULL)
        return (uint8_t)ErrorCodes::NULLPOINTER;

    uint8_t error = pack(command, length, payload);

    if (error)
        return error;

    bufferize(&_packet_tx);
    uart_write_bytes(uart_port, (const char*)_TXBuffer, _packet_tx.length + 5);

    uint16_t crc;
    int cont = 0;
    uint64_t time_limit = MAX_ACK_WAIT;
    uint64_t start_time = get_timer();
    while(_packet_rx.crc != crc && (get_timer() - start_time) < time_limit && cont < MAX_ATTEMPTS) {
        if((get_timer() - start_time) > RETRY_INTERVAL * cont){
            uint16_t len = uart_read_bytes(uart_port, (uint8_t *) &_RXBuffer, MAX_PAYLOAD_LENGTH + 5, 20);
            unpack(len);
            crc = computeCRC(_RXBuffer, _packet_rx.length + 3);
            if(_packet_rx.crc != crc)
                uart_write_bytes(uart_port, (const char*)_TXBuffer, _packet_tx.length + 5);
            else if(_packet_rx.command == (uint8_t)RxCommands::NACK){
                time_limit += RETRY_INTERVAL;
                crc = 0; //resetting crc allows another while cycle
            }
            cont++;
        }
    } 

    if(_packet_rx.crc != crc)
        return (uint8_t)ErrorCodes::ACK_TIMEOUT;

    return (uint8_t)ErrorCodes::OK;
}


uint8_t Oscup::pack(uint8_t command, uint8_t length, char *buffer) {
    /* @brief prepares data to be sent and obtains the crc
    * 
    *  @param id id of the device which has to receive the packet
    *  @param command command to execute on the receiver
    *  @param length length of the payload
    *  @param buffer payload containing data 
    * 
    *  @return it returns feedback on writing result
    */

    _packet_tx.command = command;
    _packet_tx.length = length;
    try {
        memmove(&_packet_tx.payload, buffer, length);
    } 
    catch(int e){return (uint8_t)ErrorCodes::PACKMEMMOVE_ERROR;}

    bufferize(&_packet_tx);
    _packet_tx.crc = computeCRC(_TXBuffer, length + 3);

    return (uint8_t)ErrorCodes::OK;
}


void Oscup::bufferize(packet_t *packet) {
    /* @brief This function converts packet's struct into an array, attribute of the class
     *        buffer will become: [ID,Command,Len, ...... payload ...., CRC (if present))]
     * 
     * @param packet a not empty packet
    */

   int len;

   if(packet == NULL)
       return;

    if (packet->crc)
        len = 5 + packet->length; // with crc
    else
        len = 3 + packet->length; // without crc

    _TXBuffer[0] = packet->id = _id;
    _TXBuffer[1] = packet->command;
    _TXBuffer[2] = packet->length;
    for (uint8_t i = 3; i < 3 + packet->length; i++)
        _TXBuffer[i] = packet->payload[i - 3];

    if (packet->crc) {
        _TXBuffer[len - 2] = packet->crc >> 8;
        _TXBuffer[len - 1] = packet->crc & 0xFF;
    }
}


uint8_t Oscup::read(packet_t *packet) {
    /* @brief this function reads data incoming from UART and put them inside the packet.
    * 
    *  @param *packet packet struct where will be available the readed data
    * 
    *  @return it returns feedback on writing result
    */

    uint16_t len;
    len = uart_read_bytes(uart_port, (uint8_t *) &_RXBuffer, MAX_PAYLOAD_LENGTH + 5, 20);
    unpack(len);

    packet->id = _packet_rx.id;
    packet->command = _packet_rx.command;
    packet->length = len - 5;
    memmove(packet->payload, &_packet_rx.payload, packet->length);
    packet->crc = _packet_rx.crc;
    return (uint8_t)ErrorCodes::OK;
}


void Oscup::unpack(uint16_t len) {
    /* @brief it unpacks data incoming from UART
    *
    *  @param len it is the lenght of the received buffer
    */

    _packet_rx.id = _RXBuffer[0];
    _packet_rx.command = _RXBuffer[1];
    _packet_rx.length = _RXBuffer[2];
    for (int i = 3; i < len - 3; i++)
        _packet_rx.payload[i - 3] = _RXBuffer[i];
    _packet_rx.crc = (_RXBuffer[len - 2] << 8) | _RXBuffer[len - 1];
}


uint16_t Oscup::computeCRC(char *buffer, uint16_t len) {
    /* @brief This function calculates the CRC on the packet. 
    *         Only the last two bytes are not considered. 
    *         Max packet length is 256 bytes
    * 
    *  @param buffer byte array containing the full packet - last 2 bytes
    *  @param len array length
    * 
    *  @return crc calculated on the entire packet
    */
   
  uint16_t crc = 0xFFFF;

  if(buffer == NULL)
    return (uint8_t)ErrorCodes::NULLPOINTER;

  for(uint16_t j = 0; j < len; j++) {
    uint8_t byteValue = buffer[j];
    byteValue &= 0xff;
    
    crc = (crc ^ byteValue) & 0xffff;

    for (int i = 0; i < 8;i++) {
      if ((crc & 0x0001) != 0)
          crc = (crc >> 1) ^ 49061;
      else
          crc >>= 1;
    }
  }
  return crc;
}

uint64_t Oscup::get_timer(){
    /* @brief returns the value of the hardware counter timer
    *  
    *  @return uint64_t counter value    
    */

    uint64_t timval = 0;
    timer_get_counter_value(_timer_info.group, _timer_info.index, &timval);  
    return timval;
}

uint64_t Oscup::get_APB_clk(){
    /* @brief returns the APB Clock Frequency
    *  
    *  @return APB clock Freq
    */
    return (APB_CLK_FREQ);
}