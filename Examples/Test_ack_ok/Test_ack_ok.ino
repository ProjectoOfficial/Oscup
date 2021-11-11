/* This is a demo of OSCUP protocol, in this example you have to flash this
    firmware on the ESP32 and then run the python code.

    This software is provided AS IS, no Warranties!

    Copyright © 2021 Projecto - Dott. Daniel Rossi, Dott. Riccardo Salami
    License GPL-V3

    Version: 1.2.0
*/

#include <stdlib.h>
#include "oscup.h"

uint8_t id = 0x5D;

//constructor takes device's ID and baudrate
Oscup oscup = Oscup(id, 115200);

void setup() {
  //it is mandatory to call begin() for starting UART
  oscup.begin();


  /*This code will run once, if ack works, it will resend the packet if 
   * ESP32 does not receive an ack after writing
   */

  delay(2000);
  //this function returns the APB clock frequency (set inside ESP32 libraries)
  uint64_t tim = oscup.get_APB_clk();
  
  //convert uint64_t to byte array
  char *arr2 = uint64_toBytes(tim);
  
  //write the packet, parameters: command(uint8_t), length of the payload(uint8_t), payload(char *)
  uint8_t errore = oscup.write((uint8_t)TxCommands::SHARE, sizeof(uint64_t), arr2);
 
  //remember always to free dangling pointers!
  free(arr2);
}

void loop() {
}

char *uint64_toBytes(uint64_t number) {
  /*@brief converts a uint64_t into a char * pointer
   * 
   * @param number uint64_t you want to convert
   * 
   * @return the pointer containing the new array of chars
  */
  size_t dim = sizeof(uint64_t);
  char *buff = (char *)calloc(dim, sizeof(char));
  for (int i = 0 ; i < dim; i++) {
    buff[i] = (number >> (8 * i)) & 0xFF;
  }
  return buff;
}