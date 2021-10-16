# Oscup: Open Source Custom UART Protocol
 [![Build Status](https://travis-ci.com/alichtman/shallow-backup.svg?branch=master)](https://travis-ci.com/alichtman/shallow-backup)
 [![Codacy Badge](https://api.codacy.com/project/badge/Grade/1719da4d7df5455d8dbb4340c428f851)](https://www.codacy.com/app/alichtman/shallow-backup?)
 [![License](https://poser.pugx.org/aimeos/aimeos-typo3/license.svg)](https://packagist.org/packages/aimeos/aimeos-typo3)
 [![Latest Unstable Version](http://poser.pugx.org/pierreboissinot/gsheet-adapter/v/unstable)](https://packagist.org/packages/pierreboissinot/gsheet-adapter)
 [![Check Arduino status](https://github.com/arduino-libraries/NTPClient/actions/workflows/check-arduino.yml/badge.svg)](https://github.com/arduino-libraries/NTPClient/actions/workflows/check-arduino.yml)
 Contents
========

* [Description](#Description)
* [How it Works](#How-it-Works)
* [Lets Keep in Touch](#Lets-Keep-in-Touch)
* [About](#About)

 ### Description
 `OSCUP` is an open source UART protocolo designed for working with ESP32 ARM Processors. The code well documented helps to figure out how to implement a custom protocol on UART port. 
 With this protocol you can transmit and receive data wich are tracked by a fixed preamble. Next implementation will ad ACK and NACK for each packet sent.

 ### How it Works
 It relies on primitive libraries provided by Espressiff, such as /device/uart.h for interfacing with the low level hardware and functions. The protocol provides two function  `write` and  `read` for sending and receiving data.
 When you create a new Oscup object you need to pass it an ID which keeps track of the device who sent the packet, and the communication's baudrate. Once the object is created, you can call write and read functions.

  - `Write`: the write function takes a command "user-defined", the length of the payload and the effective payload, which has a fixed length of 255 bytes. This function writes this data on UART and returns an error code.
  - `Read`: !!still not tested!! It reads the data incoming and insert them in a packet_t struct, defined in .h file. It returs the lenght of the readed buffer if everything works fine, otherwise a negative error.

 ### Lets Keep in Touch
 Follow us on YouTube: Https://youtube.com/c/ProjectoOfficial <br/>
 Follow us on Instagram: https://www.instagram.com/OfficialProjecTo/ <br/>
 Follow us on Facebook: https://www.facebook.com/MiniProjectsOfficial <br/>

 ### About
 This software is provided as it is, without any warranties. <br/>
 Authors: Dott. Daniel Rossi, Dott. Riccardo Salami