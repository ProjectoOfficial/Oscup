/* This is a demo of OSCUP protocol, in this example you have to flash this
    firmware on the ESP32 and then run the python code.

    This software is provided AS IS, no Warranties!

    Copyright © 2021 Projecto - Dott. Daniel Rossi, Dott. Riccardo Salami
    License GPL-V3

    Version: 1.2.0
*/

#include <stdlib.h>
#include "Oscup.h"

uint8_t id = 0x5D;
packet_t packet;
uint8_t errore = 255;

//constructor takes device's ID and baudrate
Oscup oscup = Oscup(id);

void setup() {
  //it is mandatory to call begin() for starting UART
  oscup.begin(115200);
}

void loop() {
  errore = oscup.read(&packet);

  delay(3000);

}
