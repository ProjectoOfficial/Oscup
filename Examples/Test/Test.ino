#include "prot.h"

uint8_t id = 0x01;
uint8_t len = 10;
uint8_t buff[] = {1,2,3,4,5,6,7,8,9,10};

Prot prot = Prot(id, 115200);

void setup(){}
void loop(){
	prot.write(id, len, buff);
}