/*
* Oscup: Open Source Custom Uart Protocol
* This Software was release under: GPL-3.0 License
* Copyright � 2022 Daniel Rossi 
* Version: 1.2.4
*/

#ifndef OSCUP_H_
#define OSCUP_H_

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

#define UART_BUFFER_LENGTH 255
#define FIX_PACKET_LENGTH 45
#define MAX_PAYLOAD_LENGTH (FIX_PACKET_LENGTH - 5) 

/********************** TIMER ***********************/
#include "driver/timer.h"

// 10kHz prescalers - APB should be 80MHz
#define TIMER_PRESCALER_240MHZ 23999
#define TIMER_PRESCALER_160MHZ 15999
#define TIMER_PRESCALER_80MHZ   7999
#define TIMER_PRESCALER_40MHZ   3999

#define MAX_ACK_WAIT 220 //22ms
#define RETRY_INTERVAL 50 //5ms
#define READ_MAX_ACK_WAIT 350 //35ms
#define READ_RETRY_INTERVAL 10 //1ms
#define MAX_ATTEMPTS 10

typedef struct {
    timer_group_t group;
    timer_idx_t index;
    uint64_t alarm_value;
    bool auto_reload;
} timer_info_t;


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
    OK = 0,
    LENGTH_ERROR = 0x01,
    PACKMEMMOVE_ERROR = 0x02,
    WRITE_ERROR = 0x03,
    NULLPOINTER = 0x04,
    ACK_TIMEOUT = 0x05,
    CRC_ERROR = 0x06,
    NACK = 0x07,
    PACK_ERROR = 0x08,
    NO_DATA = 0x09,
};


/*****************************************************
************************ OSCUP ***********************
******************************************************/
class Oscup {
    public:
        Oscup(uint8_t id, uart_port_t port = (uart_port_t)uart_port_t::UART_NUM_0, int RXPin = (int)UART_RXD_PIN, int TXPin = (int)UART_TXD_PIN);
        void begin(const uint32_t baudrate);
        uint8_t write(uint8_t command, uint8_t length, char* buffer);
        uint8_t read(packet_t* packet);
        uint64_t get_timer() const;
        uint64_t get_APB_clk() const;

    private:
        uint8_t id_;
        uint32_t baudrate_;

        packet_t packet_rx_;
        packet_t packet_tx_;

        char RXBuffer_[FIX_PACKET_LENGTH];
        char TXBuffer_[FIX_PACKET_LENGTH];

        uart_config_t uart_config_;
        uart_port_t uart_port_;        

        int uart_rxd_pin_;
        int uart_txd_pin_;
        int uart_rts_pin_;
        int uart_cts_pin_;

        int intr_alloc_flags_;

        timer_config_t timer_config_;
        timer_info_t timer_info_;

        void tim_init(const int prescaler);  
        uint8_t pack(uint8_t command, uint8_t length, char* buffer);
        void bufferize(packet_t *packet);
        void unpack();
        void resetRX();
        void resetTX();
        void sleep(const uint64_t ms);
        uint16_t computeCRC(char* buff, uint16_t len);
};

#ifdef __cplusplus
}
#endif

#endif /*OSCUP_H*/
