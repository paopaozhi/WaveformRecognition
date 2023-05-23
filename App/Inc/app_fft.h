#ifndef __APP_FFT_H
#define __APP_FFT_H

#include "main.h"

#define adc_length 1024  // ADC采集的点的个数

struct app_fft {
  /* ADC序列 */
  uint16_t* adc_buf;

  /*基本属性*/
  float voltage_max;    // 最大值
  float voltage_min;    // 最小值
  float voltage_vpp;    // 峰峰值
  float frequency;      // 频 率
  float frequency_max;  // 最大频率
  uint16_t npoints;     // 一个周期内的点的个数

  /* 周期电压序列 */
  float wave[adc_length];

  /* 下标储存 */
  uint32_t voltage_max_pos;
  uint32_t voltage_min_pos;
  uint32_t frequency_max_pos;

  /*频域参数*/
  float rms;  // 交流电有效值

  /* 方法 */
  // void (*get_voltage_max)(void);
};
typedef struct app_fft app_fft_t;

void App_FFT_Init(app_fft_t* fft, uint16_t* adc_buf);
void App_FFT_RefreshData(app_fft_t* fft);
float App_FFT_GetK(app_fft_t* fft);
float App_FFT_GetRms(app_fft_t* fft);

#endif
