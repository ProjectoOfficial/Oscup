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

    _hardware_serial = new HardwareSerial(baudrate);
    _hardware_serial->begin(baudrate);

}

uint8_t Prot::write(uint8_t command, uint8_t length, uint8_t* payload) {
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

    uint8_t* buff = bufferize(&_packet_tx);
    try {
        _hardware_serial->write(buff, _packet_tx.length + 5);
    }
    catch (int e) { return (uint8_t)ErrorCodes::WRITE_ERROR;}
    free(buff);
    return (uint8_t)ErrorCodes::OK;
}

uint8_t Prot::pack(uint8_t command, uint8_t length, uint8_t *buffer) {
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

    uint8_t* buff = bufferize(&_packet_tx);
    _packet_tx.crc = computeCRC(buff, length + 3);
    free(buff);

    return (uint8_t)ErrorCodes::OK;
}


uint8_t *Prot::bufferize(packet_t *packet) {
    /*
     * This function converts packet's struct into an array
     * buffer will become: [ID,Command,Len, ...... payload ...., CRC (if present))]
     * 
     * input: a not empty packet
    */
    uint8_t* buff;
    uint16_t len = 0;
    
   if(_packet_tx == NULL)
       return (uint8_t)ErrorCodes::NULLPOINTER;

    if (packet->crc)
        len = 5 + packet->length; // with crc
    else
        len = 3 + packet->length; // without crc

    buff = new uint8_t(len);
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


uint16_t Prot::computeCRC(uint8_t *buffer, uint8_t len) {
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
