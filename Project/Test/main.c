#include <stdio.h>
#include <stdlib.h>
#include "includes.h"
#include "time_user.h"
#include "usart_user.h"
#include "LED_user.h"
#include "slip.h"
#include "transp.h"

uint8_t dataBuffer[DATA_BUFFER_SIZE];

uint32_t delay_decrement_1mcs;
__IO uint32_t TimeOut = 0x00;

RCC_ClocksTypeDef RCC_Clocks;
extern volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
extern volatile uint16_t rx_wr_index, rx_rd_index, rx_counter;
extern volatile uint8_t tx_buffer[TX_BUFFER_SIZE];
extern volatile uint16_t tx_wr_index, tx_rd_index, tx_counter;
uint16_t channelOrderState=0;                                                   //Текущая очередь отправки отмеченных каналов
uint8_t isEmpty=1;                                                              //Флаг, что буфер пустой

/********************************************************
* MAIN
********************************************************/

void main(void) {
  
    clock_init(0);
    SystemInit();
    LEDInit();
    RTC_init();
    InitUSART2();      
    while (1){      
       LEDToggle();
       if(getMesFlag()){
         slip_packet_receive_handler();
         resetMesFlag();
       }
     }    
}

uint8_t bufferIsEmpy(){
     return isEmpty;
}
//Устанвить, что буфер пустой
void setBufferEmpty(){
    isEmpty=1;
}
//Установить, что буфер не пустой
void resetBufferEmpty(){
    isEmpty=0;
}


void generateBuffer(){
  uint8_t k=0;
  if(channelOrderState==0){                                                     //Если выдали все отмеченные каналы, начинаем сначала
    channelOrderState=getChannelsOrder();
    if(channelOrderState==0) return;                                            //Если никакие каналы не отмечены, не генерируем новый буфер
  } 
  
  for (int i=1;i<9;i=i*2){                                                      //пробегаем по битам (отмеченные каналы) до первой единички
    if(channelOrderState & i){                                                  //нашли единичку
      k=20*i;                                                                   //для тестовой генерации буфера (ниже)
      channelOrderState&=~i;                                                    //сбрасываем найденную единичку
      setCurrentChannel(i);                                                     //Устанавливаем переменную для заголовка в transp.h
      break;
    }
  }
  
  for(int i=0;i<DATA_BUFFER_SIZE;i++){                                          //Собираем тестовый буфер
      dataBuffer[i] = k++;
  }
  resetBufferEmpty();

}
void Delay_100mcs(uint32_t nTime) {
    delay_decrement_1mcs = nTime;
    while (delay_decrement_1mcs != 0) {
    }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line){
    while (1){  
    }
}
#endif
