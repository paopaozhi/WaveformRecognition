#include "bsp_usart.h"

#include <stdio.h>

#ifdef USE_OutputRedirection

__ASM(".global __use_no_semihosting");

FILE __stdout;

void _sys_exit(int x) { x = x; }

int fputc(int ch, FILE *f) {
  while ((USART1->SR & 0X40) == 0)
    ;
  USART1->DR = (uint8_t)ch;
  return ch;
}

#endif
