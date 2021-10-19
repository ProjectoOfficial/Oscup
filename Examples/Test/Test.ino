#include "oscup.h"

uint8_t id = 0x5D;

Oscup oscup = Oscup(id, 115200);

void setup() {
}
void loop() {
  //delay(3000);
  uint64_t tim = oscup.get_timer();
  char arr[sizeof(tim)];
  memcpy(arr,&tim,sizeof(tim));
  uint8_t errore = oscup.write(0xF1, sizeof(tim), arr);
  delay(1);
}