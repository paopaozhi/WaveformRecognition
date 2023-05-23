#include "bsp_adc.h"

#include <stdio.h>

#include "adc.h"
#include "app_fft.h"
#include "app_fft1.h"

uint16_t b_adc_buf[1024];

double Get_Temp(uint16_t temp_adc_value) {
  double temp_mv = temp_adc_value * (3.3 / 4096.0);

  return (1.43 - temp_mv) / 0.0043 + 25;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
  if (hadc == &hadc1) {
    App_FFT_RefreshData(&ftt_test);
    float rms = App_FFT_GetRms(&ftt_test);
    float k = App_FFT_GetK(&ftt_test);
    printf("RMS = %.2lf \r\n", rms);
    printf("K = %.2lf \r\n", k);

    // APP_Fft_K();

    if (rms > 0.64 && rms < 0.8) {
      printf("正弦波   \r\n");
    } else if (rms > 0.8) {
      printf("方波   \r\n");
    } else if (rms < 0.63) {
      printf("三角波  \r\n");
    } else {
      printf("Error: 无法识别！  \r\n");
    }
  }
}
