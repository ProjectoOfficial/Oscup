#include "oscup.h"

uint8_t id = 0x5D;

Oscup oscup = Oscup(id, 115200);

void setup() {
}
void loop() {
  //delay(3000);
  uint64_t tim = oscup.get_timer();
  uint8_t len = 8;
  char buff[] = {0, 0, 0, 0, 0, 0, 0, 0};
  buff[0]=  tim & 0xFF00000000000000;
  buff[1]=  tim & 0x00FF000000000000;
  buff[2]=  tim & 0x0000FF0000000000;
  buff[3]=  tim & 0x000000FF00000000;
  buff[4]=  tim & 0x00000000FF000000;
  buff[5]=  tim & 0x0000000000FF0000;
  buff[6]=  tim & 0x000000000000FF00;
  buff[7]=  tim & 0x00000000000000FF;
  uint8_t errore = oscup.write(0xF1, len, buff);
  delay(1);
}