#ifndef __TRANSP_H
#define __TRANSP_H

#include <stdint.h>
/**********************************************************************/
/*                      Packet format:                                */
/*   +-----------+----------+--------+----------+------+----------+    */
/*   | packet id |  length  |  cmd   |  value   | data |  crc16   |    */
/*   | (2 byte)  | (2 byte) | 1 byte | 2 byte   | (-)  | (2 byte) |    */
/*   +-----------+----------+--------+----------+------+----------+    */
/*                                                                    */
/**********************************************************************/

#define TEST_MODE
//VALUE
#define OK              0x0101
#define FAIL            0x0000
#define DATA_READY      0xF1F1
#define NO_DATA_READY   0xFFFF


//CMD
#define ASK_MCU         0x41
#define STATUS_REQUEST  0x50
#define REQUEST_POINTS  0x70
#define WRITE_POINTS    0x74
#define CH_ORDER        0x81
#define CH1             0x01
#define CH2             0x02
#define CH3             0x04
#define CH4             0x08


#define OUT_BUFFER_SIZE 11000
#define IN_BUF_SIZE 100
#define SLIP_HDR_LEN 4
#define TRANS_HDR_LEN 3

struct slip_hdr{
    uint16_t id;
    uint16_t len;
};
struct trasp_hdr{
    uint8_t cmd;
    uint8_t valueMSB;
    uint8_t valueLSB;
};
//Разбиение на два отдельных байта valueMSB и valueLSB вмето short вызвано тем, что если использовать uint16_t value
//то разметка заголовка на указатель (#define OUT_BUF_TRASP ((struct trasp_hdr *)&outBuf[SLIP_HDR_LEN])) произойдет неверно
//и первый байт cmd разметится не на один байт буфера, а на два. Как-то это назвается (выравнивание по ширине вроде)

void slip_packet_receive_handler();
void transp_send_answer();
int check_crc16(uint8_t *buff, uint16_t len);
uint16_t _crc16(uint8_t *buff, uint16_t len);
void send_std_answer(uint8_t cmd, uint16_t value);
void transp_packet_receive_handler();
void setMesFlag();
void resetMesFlag();
uint8_t getMesFlag();
void addSlipPacket(uint8_t *com, uint8_t len);
uint8_t getChannelsOrder();
void setCurrentChannel(uint16_t ch);
#endif