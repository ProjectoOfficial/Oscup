/*
* Oscup: Open Source Custom Uart Protocol
* This Software was release under: GPL-3.0 License
* Copyright � 2021 Daniel Rossi & Riccardo Salami
* Version: ALPHA 1.0.0
*/
#ifndef OSCUP_H_
#define OSCUP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************** UART ************************/
#include "driver/uart.h"
#define UART_RXD_PIN 3
#define UART_TXD_PIN 1
#define UART_RTS_PIN 0
#define UART_CTS_PIN 0

#define MAX_PAYLOAD_LENGTH 255

/********************** TIMER ***********************/
#include "driver/timer.h"
#define TIMER_PRESCALER_240MHZ 23900
#define TIMER_PRESCALER_160MHZ 15900
#define TIMER_PRESCALER_80MHZ   7900
#define TIMER_PRESCALER_40MHZ   3900

#define MAX_ACK_WAIT 5
#define MAX_PACKET_RESEND 3
typedef struct {
    timer_group_t group;
    timer_idx_t index;
    uint64_t alarm_value;
    bool auto_reload;
} timer_info_t;


static xQueueHandle s_timer_queue;
/********************** PACKET **********************/
typedef struct {
  uint8_t id;
  uint8_t command;
  uint8_t length;
  char payload[MAX_PAYLOAD_LENGTH];
  uint16_t crc;
} packet_t;

/******************* COMMANDS ***********************/
enum class RxCommands : uint8_t
{
  ACK = 0xFE,
  NACK = 0xFF,
};

enum class TxCommands : uint8_t
{
    // SHARE: just for sharing data with another device
    SHARE = 0x01,

    // CONFIRM: this data are relevant for the other device
    CONFIRM = 0x02,
};


/******************* ERROR CODES ********************/
enum class ErrorCodes : uint8_t
{
    UARTREAD_ERROR,
    OK = 0,
    LENGTH_ERROR,
    PACKMEMMOVE_ERROR,
    WRITE_ERROR,
    NULLPOINTER,
};


/*****************************************************
************************ OSCUP ***********************
******************************************************/
class Oscup {
    public:
        Oscup(uint8_t id, uint32_t baudrate);
        void begin();
        uint8_t testWrite();
        uint8_t write(uint8_t command, uint8_t length, char* buffer);
        uint8_t read(packet_t* packet);
        uint64_t get_timer();
        uint64_t get_APB_clk();

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

        timer_config_t _timer_config;
        timer_info_t _timer_info;

        void tim_init(int prescaler);  
        static bool IRAM_ATTR timer_group_isr_callback(void *args);
        uint8_t pack(uint8_t command, uint8_t length, char* buffer);
        void bufferize(packet_t *packet);
        void unpack(uint16_t len);
        uint16_t computeCRC(char* buff, uint16_t len);
};

#ifdef __cplusplus
}
#endif

#endif /*OSCUP_H*/
