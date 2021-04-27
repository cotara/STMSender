#include "includes.h"
#include <stdio.h>
//#include "main.h"
#define RX_BUFFER_SIZE 10000 
#define TX_BUFFER_SIZE 11000 


void USART2_put_string_2(unsigned char *string, uint32_t l);
int InitUSART2();
void USART2_put_char(uint8_t c);
