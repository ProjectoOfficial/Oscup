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

uint8_t id = 0x5D;

//constructor takes device's ID, Uart Port, RX Pin, TX Pin
Oscup oscup = Oscup(id, UART_NUM_2, 17, 16);

void setup() {
  //it is mandatory to call begin() for starting UART
  oscup.begin(115200);
}

unsigned long wait_time = millis();

void loop() {
  //this function returns the APB clock frequency (set inside ESP32 libraries)
  uint64_t tim = oscup.get_APB_clk();

  //convert uint64_t to byte array
  char *arr2 = uint64_toBytes(tim);

  //write the packet, parameters: command(uint8_t), length of the payload(uint8_t), payload(char *)
  uint8_t error = oscup.write((uint8_t)TxCommands::SHARE, sizeof(uint64_t), arr2);

  //remember always to free dangling pointers!
  free(arr2);
  delay(2000);
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