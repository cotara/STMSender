#include <stdio.h>
#include <stdlib.h>
#include "includes.h"
#include "time_user.h"
#include "usart_user.h"
#include "LED_user.h"
#include "slip.h"
#include "transp.h"

uint8_t ch1_f[DATA_BUFFER_SIZE], ch1_nf[DATA_BUFFER_SIZE], ch2_f[DATA_BUFFER_SIZE], ch2_nf[DATA_BUFFER_SIZE];

uint32_t delay_decrement_1mcs;
__IO uint32_t TimeOut = 0x00;

RCC_ClocksTypeDef RCC_Clocks;
extern volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
extern volatile uint16_t rx_wr_index, rx_rd_index, rx_counter;
extern volatile uint8_t tx_buffer[TX_BUFFER_SIZE];
extern volatile uint16_t tx_wr_index, tx_rd_index, tx_counter;

uint8_t isEmpty=1;

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

void generateRandomBuffer(){
  uint8_t k=0;
  for(int i=0;i<DATA_BUFFER_SIZE;i++){
      ch1_f[i] = k++;
      ch1_nf[i] = k+4;
      ch2_f[i] = k+5;
      ch2_nf[i] = k+5;
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
