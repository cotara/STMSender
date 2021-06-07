#include <string.h>
#include "transp.h"
#include "slip.h"
#include "main.h"
#include "time_user.h"

//Разметка буферов на структуры заголовков
#define OUT_BUF_SLIP ((struct slip_hdr *)&outBuf[0])
#define OUT_BUF_TRASP ((struct trasp_hdr *)&outBuf[SLIP_HDR_LEN])
#define IN_BUF_SLIP ((struct slip_hdr *)&inBuf[0])
#define IN_BUF_TRASP ((struct trasp_hdr *)&inBuf[SLIP_HDR_LEN])

uint8_t outBuf[OUT_BUFFER_SIZE];                                                //Буфер, сожержащий упакованный пакет, готовый к отправке
uint8_t inBuf[IN_BUF_SIZE];                                                     //Буфер входящих команд
extern uint8_t dataBuffer[DATA_BUFFER_SIZE];                                    //Буфер пользователя
uint16_t TxTail=0;
uint16_t channelsOrder=0,currenChanel=0;
uint8_t slipRecieved=0,inLen;
uint16_t outLen;
int check_crc16(uint8_t *data_p, uint16_t len);
uint16_t _crc16(uint8_t *data_p, uint16_t len);

static int last_packet_id = -1;

//Обработка пакета на уровне SLIP
void slip_packet_receive_handler() {
    if (check_crc16(inBuf, inLen)) {        
        uint16_t id;
        id=IN_BUF_SLIP->id;                                                        
        if ((int)id != last_packet_id) {                                        //ID изменился, значит пакет дошел, дуплим новый
            last_packet_id = id;
            transp_packet_receive_handler();                                    //Вызываем парсер команды и передаем ему данные, кроме id, len, crc
        } 
        else                                                                 //Если ID не совпадает, значит пакет не дошел - делаем переотправку
            slip_send_packet(outBuf, outLen);      
    }
}

//Обработка пакета на транспортном уровне
void transp_packet_receive_handler() {
    uint8_t cmd=IN_BUF_TRASP->cmd;
    uint16_t  value=IN_BUF_TRASP->valueMSB*256+IN_BUF_TRASP->valueLSB;
    outLen=0;
    switch(cmd) {
      case ASK_MCU: {
          send_std_answer(ASK_MCU, OK);
          break;
      }
      case STATUS_REQUEST: {
        if(bufferIsEmpy(1)|| bufferIsEmpy(2) || bufferIsEmpy(3) || bufferIsEmpy(4))
          send_std_answer(STATUS_REQUEST, NO_DATA_READY);
        else{
          send_std_answer(STATUS_REQUEST, DATA_BUFFER_SIZE);                    //Отправляем количество доступных точек
          TxTail=0;                                                             //Начинаем буфер сначала

          }
        break;
      }
      case CH_ORDER: {
        channelsOrder=value;
        send_std_answer(CH_ORDER, OK);
        #ifdef TEST_MODE
                  TIM_Cmd(TIM6, ENABLE);                                        //Включаем таймер заполнения буфера
        #endif
      }
      case REQUEST_POINTS: {
          if (bufferIsEmpy())
              send_std_answer(REQUEST_POINTS, NO_DATA_READY);
          else {           
              OUT_BUF_TRASP->cmd = REQUEST_POINTS;                              //отправляем точки
              OUT_BUF_TRASP->valueMSB = (currenChanel & 0xFF00) >> 8;           //текущий отправляемый канал             
              OUT_BUF_TRASP->valueLSB =  currenChanel & 0xFF;;

              if (DATA_BUFFER_SIZE-TxTail >= value){                            //Если точек в буфере осталось больше, чем заправшивается, то отправляем сколько запросили
                memcpy(OUT_BUF_TRASP+1, dataBuffer+TxTail, value);              //почему-то при +1 указатель смещается на весь заголовок, как надо
                outLen+=TRANS_HDR_LEN+value;
                TxTail+=value;                                                  //Передвинули указатель
                if(TxTail==DATA_BUFFER_SIZE){                                   //Отправляем последнее сообщение
                  TxTail=0;                                                     //Закончили отправку последней точки в буфере
                  setBufferEmpty(1);
#ifdef TEST_MODE
                  TIM_Cmd(TIM6, ENABLE);                                        //Включаем таймер заполнения буфера
#endif
                }
              }
              else {                                                            //На всяки случай, если запросят больше, чем есть
                memcpy(OUT_BUF_TRASP+1, dataBuffer+TxTail, DATA_BUFFER_SIZE-TxTail);
                outLen+=TRANS_HDR_LEN+value;
                TxTail=0;                                                       //Закончили отправку последней точки в буфере
                setBufferEmpty(1);
              }
              transp_send_answer();
              
          }
          break;
      } 
      case LAZER1_SET:{
        send_std_answer(LAZER1_SET, OK);
        //value
      }
      case LAZER2_SET:{
        send_std_answer(LAZER1_SET, OK);
        //value
      }
      case LAZERS_SAVE:{
        send_std_answer(LAZERS_SAVE, OK);
        //
      }      
    }
}      

//Отправка пакета на уровне SLIP
void transp_send_answer() {
    //Дописали заголовок SLIP
    OUT_BUF_SLIP->id=last_packet_id;                                            
    OUT_BUF_SLIP->len=outLen;
    outLen+= SLIP_HDR_LEN;                                                
    //Дописали в конец CRC
    uint16_t crc = _crc16(outBuf, outLen);    
    memcpy(outBuf+outLen, &crc, 2);
    outLen+=2;
    //Передали следующему уровню на отправку
    slip_send_packet(outBuf, outLen);

}

int check_crc16(uint8_t *buff, uint16_t len) {
    if (len < 3)
        return 0;
    uint16_t crc;
    crc = _crc16(buff, len - 2);
    return !memcmp(&crc, buff + len - 2, 2);
}

uint16_t _crc16(uint8_t *buff, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < len; i++) {
        crc = crc ^ buff[i];
        for (int j = 0; j < 8; ++j)
            if (crc & 0x01)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc = crc >> 1;
    }
    return (crc);
}

void send_std_answer(uint8_t cmd, uint16_t value) {
    OUT_BUF_TRASP->cmd = cmd;                                                
    OUT_BUF_TRASP->valueMSB = (value & 0xFF00) >> 8;
    OUT_BUF_TRASP->valueLSB = value & 0xFF;
    outLen+=3;
    transp_send_answer();
}

void setMesFlag(){
  slipRecieved=1;
}
void resetMesFlag(){
  slipRecieved=0;
}
uint8_t getMesFlag(){
  return slipRecieved;
}
uint8_t getChannelsOrder(){
  return channelsOrder;
}
void setCurrentChannel(uint16_t ch){
  currenChanel=ch;
}
//Сохраяет пришедшую команду в буфер для дальнейшей обработке в main
void addSlipPacket(uint8_t *com, uint8_t len){
  memcpy(inBuf,com, len);                                                       //Сохраняем принятый пакет SLIP в буфер       
  setMesFlag();                                                                 //Увеличиваем счетчик команд
  inLen=len;
}
