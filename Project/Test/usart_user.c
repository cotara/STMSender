#include "usart_user.h"

 uint8_t    rx_buffer[RX_BUFFER_SIZE];
 uint16_t   rx_wr_index,
                    rx_rd_index,
                    rx_counter;
 uint8_t   tx_buffer[TX_BUFFER_SIZE];
 uint16_t  tx_wr_index,
                   tx_rd_index,
                   tx_counter;


int InitUSART2() { 
    
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); //  USART4
  
  GPIO_InitTypeDef      GPIO_InitStructureUSART;
  GPIO_InitStructureUSART.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructureUSART.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructureUSART.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructureUSART.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructureUSART.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructureUSART);
  
  GPIO_InitStructureUSART.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructureUSART.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructureUSART.GPIO_Mode = GPIO_Mode_AF;
  GPIO_Init(GPIOD, &GPIO_InitStructureUSART);
  
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2); //PC10 to TX USART2
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2); //PC11 to RX USART2
  
  USART_InitTypeDef USART_InitStructureUSART;  
  USART_InitStructureUSART.USART_BaudRate = 460800;
  USART_InitStructureUSART.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructureUSART.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStructureUSART.USART_Parity = USART_Parity_No;
  USART_InitStructureUSART.USART_StopBits = USART_StopBits_1;
  USART_InitStructureUSART.USART_WordLength = USART_WordLength_8b;
  USART_Init(USART2, &USART_InitStructureUSART);
  
  NVIC_SetPriority (USART2_IRQn, 1);
  NVIC_EnableIRQ (USART2_IRQn);

  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
  USART_Cmd(USART2, ENABLE);    

  return 0; 
   
}

void USART2_put_char(uint8_t c) {
  while (tx_counter == TX_BUFFER_SIZE);                                         //если буфер переполнен, ждем
  USART_ITConfig(USART2, USART_IT_TC, DISABLE);                                 //запрещаем прерывание, чтобы оно не мешало менять переменную
  if (tx_counter || (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET))     //если в буфере уже что-то есть или если в данный момент что-то уже передается
  {
    tx_buffer[tx_wr_index++] = c;                                               //то кладем данные в буфер
    if (tx_wr_index == TX_BUFFER_SIZE) tx_wr_index=0;                           //идем по кругу
    ++tx_counter;                                                               //увеличиваем счетчик количества данных в буфере
    
  }
  else                                                                          //если UART свободен
    USART_SendData(USART2, c);                                                  //передаем данные без прерывания
  
  USART_ITConfig(USART2, USART_IT_TC, ENABLE);                                  //разрешаем прерывание
}

void USART2_put_string_2(unsigned char *string, uint32_t l) {
  while (l != 0){
    USART2_put_char(*string++);
    l--;
  }
}
