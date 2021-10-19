#include <stdlib.h>
#include "oscup.h"

uint8_t id = 0x5D;

Oscup oscup = Oscup(id, 115200);

void setup() {
  oscup.begin();
}

void loop() {
  for (int i = 0; i < 10; i++) {
    uint64_t tim = oscup.get_timer();
    char *arr = uint64_toBytes(tim);
    uint8_t errore = oscup.write(0xF1, sizeof(uint64_t), arr);
    delay(1);
    free(arr);
  }
  delay(1000);
  uint64_t tim = oscup.get_APB_clk();
  char *arr2 = uint64_toBytes(tim);
  uint8_t errore = oscup.write(0xF1, sizeof(uint64_t), arr2);
  free(arr2);
  delay(5000);
}

char *uint64_toBytes(uint64_t number) {
  size_t dim = sizeof(uint64_t);
  char *buff = (char *)calloc(dim, sizeof(char));
  for (int i = 0 ; i < dim; i++) {
    buff[i] = (number >> (8 * i)) & 0xFF;
  }
  return buff;
}
