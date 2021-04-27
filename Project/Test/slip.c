#include "slip.h"
#include "transp.h"
#include "usart_user.h"

#define END             0xC0    /* indicates end of packet */
#define ESC             0xDB    /* indicates byte stuffing */
#define ESC_END         0xDC    /* ESC ESC_END means END data byte */
#define ESC_ESC         0xDD    /* ESC ESC_ESC means ESC data byte */

#define RX_SLIP_BUFFER_SIZE 100

enum {
    STATE_OK ,
    STATE_ESC
};

static uint16_t received = 0;
static uint8_t rx_slip_buffer[RX_SLIP_BUFFER_SIZE];
static uint8_t state = STATE_OK;


void slip_new_rx_byte(uint8_t byte) {
    switch(byte) {
    case END:
        if (received) {
            addSlipPacket(rx_slip_buffer,received);
            received = 0;
        }
        state = STATE_OK;
        break;
    case ESC:
        state = STATE_ESC;
        break;
    case ESC_END:
        if (state == STATE_ESC) {
            rx_slip_buffer[received++] = END;            
            state = STATE_OK;
            break;
        }
    case ESC_ESC:
        if (state == STATE_ESC) {
            rx_slip_buffer[received++] = ESC;            
            state = STATE_OK;
            break;
        }
    default:
        if (received < RX_SLIP_BUFFER_SIZE)
            rx_slip_buffer[received++] = byte;
        state = STATE_OK;
    }    
    return;
}

void slip_send_packet(uint8_t *buff, uint16_t len) {

    USART2_put_char(END);
    for (int i = 0; i < len; i++) {
        uint8_t c = *buff++;
        if (c == END) {
            USART2_put_char(ESC);
            c = ESC_END;
        }
        if (c == ESC) {
            USART2_put_char(ESC);
            c = ESC_ESC;
        }
        USART2_put_char(c);
    }
    USART2_put_char(END);  
}
