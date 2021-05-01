#include "time_user.h"
#include "stm32f4xx_pwr.h"
volatile unsigned int Ticks,TicksUIP=0 ;
extern uint32_t delay_decrement_1mcs;
void clock_init(uint32_t IntrPriority)
{

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
RCC_ClocksTypeDef RCC_ClocksStatus;

Ticks = 0;
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 |  RCC_APB1Periph_TIM5,ENABLE);
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_TIM2 |  RCC_APB1Periph_TIM5,DISABLE);
//  /***********************************************************************
//                                TIMER2              
//  ***********************************************************************/
   RCC_GetClocksFreq(&RCC_ClocksStatus);
  TIM_InternalClockConfig(TIM2);
  TIM_TimeBaseStructure.TIM_Prescaler = 42-1;                                //Частота шины 84МГц, но 84000 поставить нельзя потому, что там 2 байта
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 2*10000;                                    //1мс
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
   
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
  TIM_ClearFlag(TIM2,TIM_IT_Update);
  
  NVIC_SetPriority (TIM2_IRQn, 1);
  NVIC_EnableIRQ (TIM2_IRQn);

  //TIM_Cmd(TIM2,ENABLE);
  
 /***********************************************************************
                                TIMER5              
  ***********************************************************************/
 
  RCC_GetClocksFreq(&RCC_ClocksStatus);
  TIM_InternalClockConfig(TIM5);
  
  TIM_TimeBaseStructure.TIM_Prescaler = 84-1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 100;                                       //100 мкс
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM5,&TIM_TimeBaseStructure);
  
 
  TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
  TIM_ClearFlag(TIM5,TIM_IT_Update);
   
  NVIC_SetPriority (TIM5_IRQn, 1);
  NVIC_EnableIRQ (TIM5_IRQn);
  
  TIM_Cmd(TIM5,ENABLE);

  // TIM 6
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

  TIM_TimeBaseInitTypeDef base_timer;
  TIM_TimeBaseStructInit(&base_timer);
  base_timer.TIM_Prescaler = 42000 - 1;                                         //1мс
  base_timer.TIM_Period = 2*1000;                                              //5 секунд!
  TIM_TimeBaseInit(TIM6, &base_timer);

  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM6, ENABLE);

  NVIC_EnableIRQ(TIM6_DAC_IRQn);

/*
   // TIM 7
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

  TIM_TimeBaseStructInit(&base_timer);
  base_timer.TIM_Prescaler = 24000 - 1;
  base_timer.TIM_Period = 1000;
  TIM_TimeBaseInit(TIM7, &base_timer);

  TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM7, ENABLE);

  NVIC_EnableIRQ(TIM7_IRQn);
*/
}


int RTC_init(void)
{
  RTC_TimeTypeDef time = {
  .RTC_Hours = 0,
  .RTC_Minutes = 0,
  .RTC_Seconds = 0,
  .RTC_H12 = 0
};
  /*SysTick to times per second*/
  SysTickStart(1);

  /* Enable LSE */
 
  RCC_APB1PeriphClockCmd(  RCC_APB1Periph_PWR, ENABLE);
  PWR_BackupAccessCmd(ENABLE);
  
  RCC_LSEConfig(RCC_LSE_ON);
  /* Wait till LSE is ready */
  int Tick = 0;
  
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
  {
    /*Check Ticks*/
    if( SysTick->CTRL & (1<<16))
    {
      if(6 < ++Tick)
      {
        return 1;
      }
    }
  }
  /* Select LSE as RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

  /* Enable RTC Clock */
  RCC_RTCCLKCmd(ENABLE);

  /* Wait until last write operation on RTC registers has finished */
  RTC_WaitForSynchro();
  /*Set time*/
  RTC_SetTime(RTC_Format_BIN,&time);
 
  Tick = 0;
  return 0;
}

void update_Time(RTC_TimeTypeDef *time, unsigned char* RTC_time)
{ 
  unsigned char h1,h2,m1,m2,s1,s2;
  
      if( SysTick->CTRL & (1<<16))
      {
        RTC_GetTime(RTC_Format_BIN,time);
        h1=48+time->RTC_Hours/10;
        h2=48+time->RTC_Hours%10;
        m1=48+time->RTC_Minutes/10;
        m2=48+time->RTC_Minutes%10;
        s1=48+time->RTC_Seconds/10;
        s2=48+time->RTC_Seconds%10;
        
        memcpy(RTC_time,&h1,1);
        memcpy(RTC_time+1,&h2,1);
        memcpy(RTC_time+2,":",1);
        memcpy(RTC_time+3,&m1,1);
        memcpy(RTC_time+4,&m2,1);
        memcpy(RTC_time+5,":",1);
        memcpy(RTC_time+6,&s1,1);
        memcpy(RTC_time+7,&s2,1);
              
      }
}

void DelayResolution100us(unsigned int Dly)
{
  for(; Dly; Dly--)
  {
    for(volatile uint32_t j = DLY_100US; j; j--)
    {
    }
  }
}
void SysTickStart(uint32_t Tick)
{
RCC_ClocksTypeDef Clocks;
volatile uint32_t dummy;

  RCC_GetClocksFreq(&Clocks);

  dummy = SysTick->CTRL;  
  SysTick->LOAD = (Clocks.HCLK_Frequency/8)/Tick;
  
  SysTick->CTRL = 1;
}

void SysTickStop(void)
{
    SysTick->CTRL = 0;
}

