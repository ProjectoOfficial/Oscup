/*
* Oscup: Open Source Custom Uart Protocol
* This Software was release under: GPL-3.0 License
* Copyright © 2021 Daniel Rossi & Riccardo Salami
*/
#ifndef OSCUP_H_
#define OSCUP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "driver/uart.h"
#define UART_RXD_PIN 3
#define UART_TXD_PIN 1
#define UART_RTS_PIN 0
#define UART_CTS_PIN 0

#define MAX_PAYLOAD_LENGTH 255

typedef struct {
  uint8_t id;
  uint8_t command;
  uint8_t length;
  char payload[MAX_PAYLOAD_LENGTH];
  uint16_t crc;
} packet_t;

enum class RxCommands : uint8_t
{
  ACK = 0x0A,
  NACK = 0x0B,
};

enum class TxCommands : uint8_t
{
    READ = 0x01,
    REBOOT = 0x05,
};

enum class ErrorCodes : uint8_t
{
    READ_ERROR,
    OK = 0,
    LENGTH_ERROR,
    PACKMEMMOVE_ERROR,
    WRITE_ERROR,
    NULLPOINTER,
};

class Oscup {
    public:
        Oscup(uint8_t id, uint32_t baudrate);
        uint8_t testWrite();
        uint8_t write(uint8_t command, uint8_t length, char* buffer);
        uint8_t read(packet_t* packet);

    private:
        uint8_t _id;
        uint32_t _baudrate;

        packet_t _packet_rx;
        packet_t _packet_tx;

        char _RXBuffer[MAX_PAYLOAD_LENGTH + 5];
        char _TXBuffer[MAX_PAYLOAD_LENGTH + 5];

        uart_config_t _uart_config;
        uart_port_t uart_port;

        int uart_rxd_pin;
        int uart_txd_pin;
        int uart_rts_pin;
        int uart_cts_pin;

        int intr_alloc_flags;

        uint8_t pack(uint8_t command, uint8_t length, char* buffer);
        void bufferize(packet_t *packet);
        void unpack(uint16_t len);
        uint16_t computeCRC(char* buff, uint8_t len);
};

#endif /*OSCUP_H*/
