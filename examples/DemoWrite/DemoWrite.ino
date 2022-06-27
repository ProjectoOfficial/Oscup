/* This is a demo of OSCUP protocol, in this example you have to flash this
    firmware on the ESP32 and then run the python code.

    This software is provided AS IS, no Warranties!

    Copyright © 2022 Projecto - Dott. Daniel Rossi
    License GPL-V3

    @version 1.2.4

    @brief this example sketch uses oscup for writing data on UART. It initializes Oscup with a
    fixed ID and Baudrate and initializes the protocol inside void setup function.
    In the void loop it retrieves the value of the APB clock as an uint64_t, then it calls a function
    which converts this value into an array of character and after that it writes this data on UART.
    Finishing, it calls a free on the buffer.
*/

#include <stdlib.h>
#include "oscup.h"

uint8_t id = 0x5D;

//constructor takes device's ID 
Oscup oscup = Oscup(id);

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