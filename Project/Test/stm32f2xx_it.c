/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_it.h"

#include "usart_user.h"
#include "LED_user.h"
#include "slip.h"

#define LED_ETH 0x01
#define LED_RX 0x02
#define LED_TX 0x04
#define LED_TRIGG 0x08

////////////////////////////////////////////////////////////////////////
//extern __IO uint8_t ubRxIndex;/////////////////////
//extern __IO uint8_t ubTxIndex;/////////////////////
//extern __IO uint8_t ubUsartTransactionType;////////
//extern __IO uint8_t ubUsartMode;///////////////////
__IO uint8_t ubCounter = 0x00; /////////////////////
extern __IO uint32_t TimeOut; //////////////////////
///////////////////////////////////////////////////

extern uint32_t delay_decrement_1mcs, strob_decrement;
extern  uint8_t rx_buffer[RX_BUFFER_SIZE];
extern  uint16_t rx_wr_index, rx_rd_index, rx_counter;
extern  uint8_t tx_buffer[TX_BUFFER_SIZE];
extern  uint16_t tx_wr_index, tx_rd_index, tx_counter;


uint32_t time_mcs100=0;

void HardFault_Handler(void) {
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1) {
    }
}

void SysTick_Handler(void) {
    /* Decrement the timeout value */
    if (TimeOut != 0x0) {
        TimeOut--;
    }
    
    if (ubCounter < 10) {
        ubCounter++;
    } else {
        ubCounter = 0x00;
        //STM_EVAL_LEDToggle(LED1);
    }
}

void USART2_IRQHandler(void) {  
    GPIO_SetBits(GPIOE, GPIO_Pin_0);
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
        USART_ClearITPendingBit(USART2, USART_IT_RXNE); //очистка признака прерывания
        if ((USART2->SR & (USART_FLAG_FE | USART_FLAG_PE)) == 0) { //нет ошибок
           
            //rx_buffer[rx_wr_index++] = (uint8_t) (USART_ReceiveData(USART2) & 0xFF); //считываем данные в буфер, инкрементируя хвост буфера
            
            uint8_t byte = USART_ReceiveData(USART2) & 0xFF;
            slip_new_rx_byte(byte);
            TIM2->CNT = 0;
            TIM_Cmd(TIM2, ENABLE); //Запустили таймер
            /*if(rx_buffer[rx_wr_index-1]=='\n'){          
              addCommand(rx_buffer,rx_wr_index);
              rx_wr_index = 0;
              TIM_Cmd(TIM2, DISABLE);
            }*/
            
        }                                 //Сообщение из USART пришло без ошибок
        else
           USART_ReceiveData(USART2) ;
    }
    
    
    if (USART_GetITStatus(USART2, USART_IT_ORE_RX) == SET) { //прерывание по переполнению буфера
        USART_ReceiveData(USART2); //в идеале пишем здесь обработчик переполнения буфера, но мы просто сбрасываем этот флаг прерывания чтением из регистра данных.
    }
    
    
    if (USART_GetITStatus(USART2, USART_IT_TC) != RESET) { //прерывание по передаче
        USART_ClearITPendingBit(USART2, USART_IT_TC); //очищаем признак прерывания
        if (tx_counter) {                               //если есть что передать
            --tx_counter;           // уменьшаем количество не переданных данных
            USART_SendData(USART2, tx_buffer[tx_rd_index++]); //передаем данные инкрементируя хвост буфера
            if (tx_rd_index == TX_BUFFER_SIZE)
                tx_rd_index = 0;                         //идем по кругу
        }
        else
            USART_ITConfig(USART2, USART_IT_TC, DISABLE); //если нечего передать, запрещаем прерывание по передаче
    }

}
//Таймер для сброса непринятой посылки
void TIM2_IRQHandler(void) {
    TIM_ClearFlag(TIM2, TIM_IT_Update);
    rx_wr_index = 0;          //1 мс не приходило нового байта. Значит был затык
    TIM_Cmd(TIM2, DISABLE);         //Перестали считать. Ждем следующей посылки.
    TIM2->CNT = 0;
}
//Таймер для задержек
void TIM5_IRQHandler(void) {
  // Clear TIM5 counter
   TIM_ClearFlag(TIM5, TIM_IT_Update);  
   
   time_mcs100++;
    if (delay_decrement_1mcs != 0) {
        delay_decrement_1mcs--;
    }
    
}
// каждые 2 секунды генерируется новый буфер
void TIM6_DAC_IRQHandler() {
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) {
        generateBuffer();
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
        TIM_Cmd(TIM6, DISABLE);
    }
}
// таймер для генераци строба
void TIM7_IRQHandler(void) {
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) {        
               
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    }
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
