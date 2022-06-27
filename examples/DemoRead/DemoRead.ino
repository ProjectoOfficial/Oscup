/* This is a demo of OSCUP protocol, in this example you have to flash this
    firmware on the ESP32 and then run the python code.

    This software is provided AS IS, no Warranties!

    Copyright © 2022 Projecto - Dott. Daniel Rossi
    License GPL-V3

    @version: 1.2.4
*/

#include <stdlib.h>
#include "oscup.h"
#define LED 19

uint8_t id = 0x5D;

//constructor takes device's ID 
Oscup oscup = Oscup(id);

void setup() {
  //it is mandatory to call begin() for starting UART
  oscup.begin(115200);
  pinMode(LED, OUTPUT);

  for(int i=0;i<10;++i){
    digitalWrite(LED, !digitalRead(LED));
    delay(100);
  }
  
  digitalWrite(LED, LOW);
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
    uint64_t err = errore;
    char *arr3 = uint64_toBytes(err);
    oscup.write((uint8_t)0x03, sizeof(uint64_t), arr3);
  } 
  else {
    if (packet.length == 0) {
      String str = "empty";
      oscup.write((uint8_t)TxCommands::SHARE, str.length() , (char *)str.c_str());
      digitalWrite(LED, LOW);
    } 
    else {
      uint64_t l = packet.length;
      char *arr2 = uint64_toBytes(l);
      oscup.write((uint8_t)TxCommands::CONFIRM, packet.length, packet.payload);
      digitalWrite(LED, HIGH);
    }
  }
  delay(1000);
}

char *uint64_toBytes(uint64_t number) {
  /*@brief converts a uint64_t into a char * pointer

     @param number uint64_t you want to convert

     @return the pointer containing the new array of chars
  */
  size_t dim = sizeof(uint64_t);
  char *buff = (char *)calloc(dim, sizeof(char));
  for (int i = 0 ; i < dim; i++) {
    buff[i] = (number >> (8 * i)) & 0xFF;
  }
  return buff;
}