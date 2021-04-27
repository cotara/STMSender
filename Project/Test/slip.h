#ifndef __SLIP_H
#define __SLIP_H

#include <stdint.h>

void slip_new_rx_byte(uint8_t byte);
void slip_send_packet(uint8_t *buff, uint16_t len);
void slip_set_send_callback(void (*callback)(uint8_t byte));
void slip_set_data_ready_callback(void (*callback)(uint8_t *buff, uint16_t len));

#endif // __SLIP_H