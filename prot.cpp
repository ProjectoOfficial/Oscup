#include "prot.h"

Prot::Prot(uint8_t id, uint32_t baudrate) {
    /*
    * Class Constructor: creates a new HardwareSerial obj
    * and starts the communication through begin
    * input: 
    *       - id of this device
    *       - communication baudrate
    */
    _id = id;
    _baudrate = baudrate;

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
    uart_driver_install(uart_port, MAX_PAYLOAD_LENGTH + 1, MAX_PAYLOAD_LENGTH + 1, 0, NULL, intr_alloc_flags);
    uart_param_config(uart_port, &_uart_config);
    uart_set_pin(uart_port, uart_txd_pin, uart_rxd_pin, uart_rts_pin, uart_cts_pin);
}

uint8_t Prot::write(uint8_t command, uint8_t length, char* payload) {
    /*
    * Writes data on Uart
    * 
    * Input: 
    *       - command to execute on receiver
    *       - payload length
    *       - payload
    */

    if (length > MAX_PAYLOAD_LENGTH)
        return (uint8_t)ErrorCodes::LENGTH_ERROR;

    if(payload == NULL)
        return (uint8_t)ErrorCodes::NULLPOINTER;

    uint8_t error = pack(command, length, payload);

    if (error)
        return error;

    char* buff = bufferize(&_packet_tx);
    if(buff == NULL){
        free(buff);
        return (uint8_t)ErrorCodes::NULLPOINTER;
    }
    else {
        try {
           uart_write_bytes(uart_port, (const char*)buff, _packet_tx.length + 5);
        }
        catch (int e) { return (uint8_t)ErrorCodes::WRITE_ERROR; }
    }
    free(buff);
    return (uint8_t)ErrorCodes::OK;
}

uint8_t Prot::testWrite() {
    uart_write_bytes(uart_port, (const char*)"prova\r\n", 8);
    return (uint8_t)ErrorCodes::OK;
}

uint8_t Prot::pack(uint8_t command, uint8_t length, char *buffer) {
    /* prepares data to be sent and obtains the crc
    * input:
    *       - id: id of the device which has to receive the packet
    *       - command: command to execute on the receiver
    *       - length: length of the payload
    *       - buffer: payload containing data 
    */

    _packet_tx.command = command;
    _packet_tx.length = length;
    try {
        memmove(&_packet_tx.payload, buffer, length);
    } 
    catch(int e){return (uint8_t)ErrorCodes::PACKMEMMOVE_ERROR;}

    char* buff = bufferize(&_packet_tx);
    if(buff == NULL){
        free(buff);
        return (uint8_t)ErrorCodes::NULLPOINTER;
    }
    _packet_tx.crc = computeCRC(buff, length + 3);
    free(buff);

    return (uint8_t)ErrorCodes::OK;
}


char *Prot::bufferize(packet_t *packet) {
    /*
     * This function converts packet's struct into an array
     * buffer will become: [ID,Command,Len, ...... payload ...., CRC (if present))]
     * 
     * input: a not empty packet
    */
    char* buff;
    uint16_t len = 0;
    
   if(packet == NULL)
       return NULL;

    if (packet->crc)
        len = 5 + packet->length; // with crc
    else
        len = 3 + packet->length; // without crc

    buff = new char(len);
    buff[0] = packet->id;
    buff[1] = packet->command;
    buff[2] = packet->length;
    for (uint8_t i = 3; i < 3 + packet->length; i++)
        buff[i] = packet->payload[i - 3];

    if (packet->crc) {
        buff[len - 2] = packet->crc & 0xFF00;
        buff[len - 1] = packet->crc & 0x00FF;
    }

    return buff;
}


uint16_t Prot::computeCRC(char *buffer, uint8_t len) {
    /*
    * This function calculates the CRC on the packet. 
    * Only the last two bytes are not considered. 
    * Max packet length is 256 bytes
    * input: 
    *       - byte array containing the full packet - last 2 bytes
    *       - array length
    * output: crc calculated on the entire packet
    */
  uint16_t crc;

  if(buffer == NULL)
    return (uint8_t)ErrorCodes::NULLPOINTER;

  for(uint8_t j = 0; j < len; j++) {
    uint8_t byteValue = buffer[j];
    byteValue = byteValue & 0xff;
    crc = (0xFFFF ^ byteValue) & 0xffff;

    uint8_t n = 0;
    while (n <= 7) {
      if ((crc & 0x0001) != 0)
          crc = (crc >> 1) ^ 49061;
      else
          crc = crc >> 1;
      n++;
    }
  }
  return crc;
}
