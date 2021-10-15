#ifndef PROT_H_
#define PROT_H_

#include "hardwareSerial.h"
#include <stdlib.h>
#include <stdint.h>

#define MAX_PAYLOAD_LENGTH 255

typedef struct {
  uint8_t id;
  uint8_t command;
  uint8_t length;
  uint8_t payload[MAX_PAYLOAD_LENGTH];
  uint16_t crc;
} packet_t;

enum class RxCommands : uint8_t
{
  ACK = 0x0A,
  NACK = 0x0B,
};

enum class TxCommands : uint8_t
{
    READ = 0x01,
    REBOOT = 0x05,
};

enum class ErrorCodes : uint8_t
{
    OK = 0,
    LENGTH_ERROR,
    PACKMEMMOVE_ERROR,
    WRITE_ERROR,
    NULLPOINTER,
};

class Prot {
    public:
        Prot(uint8_t id, uint32_t baudrate);
        uint8_t write(uint8_t command, uint8_t length, uint8_t* buffer);

    private:
        uint8_t _id;
        uint32_t _baudrate;
        HardwareSerial *_hardware_serial;

        packet_t _packet_rx;
        packet_t _packet_tx;

        uint8_t pack(uint8_t command, uint8_t length, uint8_t* buffer);
        uint8_t *bufferize(packet_t *packet);
        uint16_t computeCRC(uint8_t* buff, uint8_t len);
};

#endif /*PROT_H*/
