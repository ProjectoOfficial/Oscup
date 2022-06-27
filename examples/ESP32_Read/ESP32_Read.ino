/* This is a demo of OSCUP protocol, in this example you have to flash this
    firmware on the ESP32.

    This software is provided AS IS, no Warranties!

    Copyright © 2022 Projecto - Dott. Daniel Rossi
    License GPL-V3

    @version: 1.2.4

    For running this example you must use 2 ESP32 connected one to each other.
    Let's call them ESP1 and ESP2. 

    What about Hardware connections?
    - ESP1 RX pin must be connected to ESP2 TX pin (so ESP1 pin 17 to ESP2 pin 16)
    - ESP1 TX pin must be connected to ESP2 RX pin (so ESP1 pin 16 to ESP2 pin 17)
    - Then connect ESP1 GND to ESP2 GND

    Subsequently flash the firmware (ESP32_Read on one and ESP32_Write on the other) and just open the
    serial monitor on the ESP processor on which you've flashed the ESP32_Read Firmware
*/

#include <stdlib.h>
#include "oscup.h"

uint8_t id = 0x5F;

//constructor takes device's ID, uart_port (defined in /driver/uart.h), Rx Pin, Tx Pin
Oscup oscup = Oscup(id, UART_NUM_2, 17, 16);

void setup() {
  //it is mandatory to call begin() for starting UART
  oscup.begin(115200);
  Serial.begin(115200);
}

unsigned long start_time = millis();

void loop() {
  packet_t packet;
  uint8_t errore = 255;

  start_time = millis();
  while (errore != (uint8_t)ErrorCodes::OK && millis() - start_time < 200) {
    errore = oscup.read(&packet);
  }

  delay(5);

  if (errore != (uint8_t)ErrorCodes::OK) {
    Serial.print("Error: "); // remove this print if you only want to see just the data incoming
    Serial.println(errore);
  } else {
    if (packet.length == 0) {
      Serial.println("empty packet");
    } else {
      Serial.println("Data:");
      Serial.print("Packet length: ");
      Serial.println(packet.length);
      uint64_t val = bytes_to_uint64(packet.payload);
      Serial.println(val);
    }
  }
  delay(100);
}

uint64_t bytes_to_uint64(char *payload) {
  /*@brief converts a char * pointer into uint_64t (it could be done better, but this is enough for example purposes)

     @param payload chars to be converter

     @return the corresponding uint64_t value
  */
  union bytes {
    unsigned char c[8];
    uint64_t l;
} var;

  for(int i=0; i<8; ++i)
    var.c[i] = payload[i];

  return var.l;
}