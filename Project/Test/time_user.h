#include "stm32f4xx_tim.h"
#include "string.h"
#define DLY_100US  16800

void clock_init(uint32_t IntrPriority);
int RTC_init(void);
void update_Time(RTC_TimeTypeDef *time, unsigned char* RTC_time);
void DelayResolution100us(unsigned int Dly);
void SysTickStart(uint32_t Tick);
void SysTickStop(void);
