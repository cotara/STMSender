#ifndef __TRANSP_H
#define __TRANSP_H

#include <stdint.h>
/**********************************************************************/
/*                      Packet format:                                */
/*   +-----------+----------+--------+---------+------+----------+    */
/*   | packet id |  length  |  cmd   |  value  | data |  crc16   |    */
/*   | (2 byte)  | (2 byte) | 1 byte | 1 byte  | (-)  | (2 byte) |    */
/*   +-----------+----------+--------+---------+------+----------+    */
/*                                                                    */
/**********************************************************************/
#define OK              0x01
#define FAIL            0x00
#define DATA_READY      0x04
#define NO_DATA_READY   0x05


#define ASK_MCU         0x41
#define STATUS_REQUEST  0x50

#define REQUEST_POINTS  0x70
#define WRITE_POINTS    0x74


#define OUT_BUFFER_SIZE 11000
#define IN_BUF_SIZE 100
#define SLIP_HDR_LEN 4
#define TRANS_HDR_LEN 2

struct slip_hdr{
    uint16_t id;
    uint16_t len;
};
struct trasp_hdr{
    uint8_t cmd;
    uint8_t value;
};

void slip_packet_receive_handler();
void transp_send_answer();
int check_crc16(uint8_t *buff, uint16_t len);
uint16_t _crc16(uint8_t *buff, uint16_t len);
void send_std_answer(uint8_t cmd, uint8_t value);
void transp_packet_receive_handler();
void setMesFlag();
void resetMesFlag();
uint8_t getMesFlag();
void addSlipPacket(uint8_t *com, uint8_t len);


#endif