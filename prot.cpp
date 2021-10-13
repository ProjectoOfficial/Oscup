#include "prot.h"

Prot::Prot(uint8_t ID) {}

uint16_t Prot::ComputeCRC(uint8_t *buff, uint8_t len) {
  //funzione che calcola il CRC degli ultimi due byte
  uint16_t crc = 0xFFFF;
  int j = 0;
  while (j < len) {
    uint8_t byteValue = buff[j];
    byteValue = byteValue & 0xff;
    uint16_t tmpCrc = (crc ^ byteValue) & 0xffff;

    uint8_t n = 0;
    while (n <= 7) {
      if ((tmpCrc & 0x0001) != 0){
        tmpCrc = tmpCrc >> 1;
        tmpCrc = tmpCrc ^ 49061;
      }
      else
        tmpCrc = tmpCrc >> 1;
      n += 1;
    }
    crc = tmpCrc;
    j += 1;
  }
  return crc;
}
