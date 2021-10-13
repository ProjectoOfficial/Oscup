#ifndef PROT_H_
#define PROT_H_

#include <stdlib.h>
#include <stdint.h>

typedef struct packet {
  uint8_t id;
  uint8_t command;
  uint8_t length;
  uint8_t paytload;
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

class Prot {
  public:
    Prot(uint8_t ID);
    uint16_t ComputeCRC(uint8_t *buff, uint8_t len);
};

#endif /*PROT_H*/
