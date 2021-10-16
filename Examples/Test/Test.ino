#include "oscup.h"

uint8_t id = 0x01;
uint8_t len = 10;
char buff[] = {1,2,3,4,5,6,7,8,9,10};

Oscup oscup = Oscup(id, 115200);

void setup(){}
void loop(){
	uint8_t errore = oscup.write(id, len, buff);
	delay(3000);
}