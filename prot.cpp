#include "prot.h"

Prot::Prot(uint8_t id, uint32_t baudrate) {
    /*
    * Class Constructor: initializes the UART of ESP32
    * by defining the hardware port and hardware pins
    * and other UART's parameters
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

    bufferize(&_packet_tx);
    try {
        uart_write_bytes(uart_port, (const char*)_TXBuffer, _packet_tx.length + 5);
    }
    catch (int e) { return (uint8_t)ErrorCodes::WRITE_ERROR; }
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

    bufferize(&_packet_tx);
    _packet_tx.crc = computeCRC(_TXBuffer, length + 3);

    return (uint8_t)ErrorCodes::OK;
}


void Prot::bufferize(packet_t *packet) {
    /*
     * This function converts packet's struct into an array, attribute of the class
     * buffer will become: [ID,Command,Len, ...... payload ...., CRC (if present))]
     * 
     * input: a not empty packet
    */
    int len;

   if(packet == NULL)
       return;

    if (packet->crc)
        len = 5 + packet->length; // with crc
    else
        len = 3 + packet->length; // without crc

    _TXBuffer[0] = packet->id;
    _TXBuffer[1] = packet->command;
    _TXBuffer[2] = packet->length;
    for (uint8_t i = 3; i < 3 + packet->length; i++)
        _TXBuffer[i] = packet->payload[i - 3];

    if (packet->crc) {
        _TXBuffer[len - 2] = packet->crc & 0xFF00;
        _TXBuffer[len - 1] = packet->crc & 0x00FF;
    }
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
