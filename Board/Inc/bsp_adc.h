#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "bsp.h"
#include "main.h"


extern uint16_t b_adc_buf[1024];

double Get_Temp(uint16_t temp_adc_value);

#endif  // __BSP_ADC_H

