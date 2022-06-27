/*
* Oscup: Open Source Custom Uart Protocol
* This Software was release under: GPL-3.0 License
* Copyright � 2022 Daniel Rossi
* Version: 1.2.4
*/

#include "Oscup.h"

Oscup::Oscup(uint8_t id, uart_port_t port, int RXPin, int TXPin) {
    /* @brief initializes the UART of ESP32 by defining the hardware port and hardware pins
    *         and other UART's parameters
    *  
    *  @param    id of this device, 
    *            port UART communication port (let user choose wether to communicate with PC or with another MCU)
    *            RXPin Rx Pin
    *            TXPin Tx pin
    *  @param    communication baudrate
    */

    id_ = id;

    // UART init
    uart_port_ = port;
    uart_rxd_pin_ = RXPin;
    uart_txd_pin_ = TXPin;
    uart_rts_pin_ = UART_RTS_PIN;
    uart_cts_pin_ = UART_CTS_PIN;
    intr_alloc_flags_ = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags_ = ESP_INTR_FLAG_IRAM;
#endif

    // TIMER init
    timer_info_.group = TIMER_GROUP_1;
    timer_info_.index = TIMER_1;
    timer_info_.auto_reload = true;
    timer_info_.alarm_value = 0;
    tim_init(TIMER_PRESCALER_80MHZ); //APB should be 80MHz
}

void Oscup::begin(const uint32_t baudrate){
    baudrate_ = baudrate;

    uart_config_ = {
        .baud_rate = baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_driver_install(uart_port_, UART_BUFFER_LENGTH, UART_BUFFER_LENGTH, 0, NULL, intr_alloc_flags_);
    uart_param_config(uart_port_, &uart_config_);
    uart_set_pin(uart_port_, uart_txd_pin_, uart_rxd_pin_, uart_rts_pin_, uart_cts_pin_);
}


void Oscup::tim_init(const int prescaler){
    /* @brief initializes the timer and its variables
    *   
    *  @param prescaler used for APB frequency scaling
    */

    timer_config_.divider = prescaler;
    timer_config_.counter_dir = TIMER_COUNT_UP;
    timer_config_.counter_en = true;
    timer_config_.alarm_en = true;
    timer_config_.auto_reload = timer_info_.auto_reload;

    timer_init(timer_info_.group, timer_info_.index, &timer_config_);

    timer_set_counter_value(timer_info_.group, timer_info_.index, 0);
    timer_set_alarm_value(timer_info_.group, timer_info_.index, timer_info_.alarm_value);
    timer_set_alarm(timer_info_.group, timer_info_.index, TIMER_ALARM_EN);
    timer_set_counter_value(timer_info_.group, timer_info_.index, 0);
    timer_start(timer_info_.group, timer_info_.index);

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

    bufferize(&packet_tx_);
    uart_write_bytes(uart_port_, (const char*)TXBuffer_, FIX_PACKET_LENGTH);

    uint16_t crc;
    int cont = 0;
    uint64_t start_time = get_timer();

    while((packet_rx_.crc != crc || cont == 0) && (get_timer() - start_time) <= MAX_ACK_WAIT && cont <= MAX_ATTEMPTS) {
        resetRX();

        if((get_timer() - start_time) >= RETRY_INTERVAL * cont){
            uart_read_bytes(uart_port_, (uint8_t *) &RXBuffer_, FIX_PACKET_LENGTH, 10);
            sleep(1);
            unpack();
            crc = computeCRC(RXBuffer_, FIX_PACKET_LENGTH - 2);
            
            if (packet_rx_.command == (uint8_t)RxCommands::ACK)
                break;
            sleep(10);
            uart_write_bytes(uart_port_, (const char*)TXBuffer_, FIX_PACKET_LENGTH); //if NACK or empty
            cont++;
        }
    } 

    resetTX();
    resetRX();
    if(packet_rx_.crc != crc)
        return (uint8_t)ErrorCodes::CRC_ERROR;

    return (uint8_t)ErrorCodes::OK;
}


uint8_t Oscup::pack(uint8_t command, uint8_t length, char *buffer) {
    /* @brief prepares data to be sent and obtains the crc
    * 
    *  @param command command to execute on the receiver
    *  @param length length of the payload
    *  @param buffer payload containing data 
    * 
    *  @return it returns feedback on writing result
    */

    packet_tx_.command = command;
    packet_tx_.length = length;
    try {
        memmove(&packet_tx_.payload, buffer, length);
    } 
    catch(int e){return (uint8_t)ErrorCodes::PACKMEMMOVE_ERROR;}

    bufferize(&packet_tx_);
    packet_tx_.crc = computeCRC(TXBuffer_, FIX_PACKET_LENGTH - 2);

    return (uint8_t)ErrorCodes::OK;
}


void Oscup::bufferize(packet_t *packet) {
    /* @brief This function converts packet's struct into an array, attribute of the class
     *        buffer will become: [ID,Command,Len, ...... payload ...., CRC (if present))]
     * 
     * @param packet a not empty packet
    */

   if(packet == NULL)
       return;

    TXBuffer_[0] = packet->id = id_;
    TXBuffer_[1] = packet->command;
    TXBuffer_[2] = packet->length;
    for (uint8_t i = 3; i < 3 + packet->length; i++)
        TXBuffer_[i] = packet->payload[i - 3];

    if (packet->crc) {
        TXBuffer_[FIX_PACKET_LENGTH - 1] = packet->crc >> 8;
        TXBuffer_[FIX_PACKET_LENGTH - 2] = packet->crc & 0xFF;
    }
}


uint8_t Oscup::read(packet_t *packet) {
    /* @brief this function reads data incoming from UART and put them inside the packet.
    * 
    *  @param *packet packet struct where will be available the readed data
    * 
    *  @return it returns feedback on reading result
    */
    uart_flush(uart_port_);
    uint16_t crc;
    resetRX(); 
    resetTX();

    packet_tx_.id = id_;
    packet_tx_.command = (uint8_t)RxCommands::NACK;

    int exit_code = uart_read_bytes(uart_port_, (uint8_t*) RXBuffer_, FIX_PACKET_LENGTH, 100); 

    if (exit_code == ESP_FAIL) {
        bufferize(&packet_tx_);
        uart_write_bytes(uart_port_, (const char*)TXBuffer_, FIX_PACKET_LENGTH);
        return (uint8_t)ErrorCodes::NO_DATA;
    }

    unpack();
    sleep(5);

    if(packet_rx_.length <= FIX_PACKET_LENGTH - MAX_PAYLOAD_LENGTH)
        return (uint8_t)ErrorCodes::LENGTH_ERROR;

    crc = computeCRC(RXBuffer_, FIX_PACKET_LENGTH - 2);

    if (packet_rx_.crc != crc) {
        bufferize(&packet_tx_);
        uart_write_bytes(uart_port_, (const char*)TXBuffer_, FIX_PACKET_LENGTH);
        return (uint8_t)ErrorCodes::CRC_ERROR;
    }
    else {
        packet_tx_.command = (uint8_t)RxCommands::ACK;
        bufferize(&packet_tx_);
        uart_write_bytes(uart_port_, (const char*)TXBuffer_, FIX_PACKET_LENGTH);
    }

    packet->id = packet_rx_.id;
    packet->command = packet_rx_.command;
    packet->length = packet_rx_.length;
    memmove(packet->payload, packet_rx_.payload, packet->length);
    packet->crc = packet_rx_.crc;

    return (uint8_t)ErrorCodes::OK;
}


void Oscup::unpack() {
    /* @brief it unpacks data incoming from UART
    *
    *  @param len it is the lenght of the received buffer
    */
    if (RXBuffer_[2] < 5)
        return;

    packet_rx_.id = RXBuffer_[0];
    packet_rx_.command = RXBuffer_[1];
    packet_rx_.length = RXBuffer_[2];
    for (int i = 0; i < packet_rx_.length; i++)
        packet_rx_.payload[i] = RXBuffer_[i + 3];
    packet_rx_.crc = (RXBuffer_[FIX_PACKET_LENGTH - 1] << 8) | RXBuffer_[FIX_PACKET_LENGTH - 2];
}


void Oscup::resetRX() {
    /*
    * @brief resets RXBuffer and packet_rx_
    */
    packet_rx_.id = 0;
    packet_rx_.command = 0;
    packet_rx_.length = 0;
    for (int i = 0; i < MAX_PAYLOAD_LENGTH ; i++)
        packet_tx_.payload[i] = 0;
    packet_rx_.crc = 0;

    for (int i = 0; i < FIX_PACKET_LENGTH; i++)
        RXBuffer_[i] = 0;
}


void Oscup::resetTX() {
    /*
    * @brief resets TXBuffer and packet_tx_
    */
    packet_tx_.command = 0;
    packet_tx_.length = 0;
    for (int i = 0; i < MAX_PAYLOAD_LENGTH; i++)
        packet_tx_.payload[i] = 0;
    packet_tx_.crc = 0;

    for (int i = 0; i < FIX_PACKET_LENGTH; i++)
        TXBuffer_[i] = 0;
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

void Oscup::sleep(const uint64_t ms) {
    uint64_t start_time = get_timer();
    while (get_timer() - start_time < ms * 10);
}

uint64_t Oscup::get_timer() const{
    /* @brief returns the value of the hardware counter timer
    *  
    *  @return uint64_t counter value    
    */

    uint64_t timval = 0;
    timer_get_counter_value(timer_info_.group, timer_info_.index, &timval);  
    return timval;
}

uint64_t Oscup::get_APB_clk() const{
    /* @brief returns the APB Clock Frequency
    *  
    *  @return APB clock Freq
    */
    return (APB_CLK_FREQ);
}

