#include "app_fft.h"

#include <stdio.h>

#include "arm_const_structs.h"
#include "arm_math.h"

static float fft_inputbuf[adc_length * 2];
static float fft_outputbuf[adc_length];

float wave1[adc_length];  // 存放adc转换npoints后的波形的值

void Get_Voltage_Max(app_fft_t* fft) {
  arm_max_f32(fft->wave, 1024, &fft->voltage_max, &fft->voltage_max_pos);
}

void Get_Voltage_Min(app_fft_t* fft) {
  arm_min_f32(fft->wave, 1024, &fft->voltage_min, &fft->voltage_min_pos);
}

void Get_Voltage_v_vpp(app_fft_t* fft) {
  fft->voltage_vpp = fft->voltage_max - fft->voltage_min;
}

/**
 * @brief 转换电压
 *
 */
static void SwitchVoltage(app_fft_t* fft) {
  for (int i = 0; i < adc_length; i++) {
    fft->wave[i] = fft->adc_buf[i] * 3.3 / 4095;  // 转换电压
  }
}

void App_FFT_Init(app_fft_t* fft, uint16_t* adc_buf) {
  fft->adc_buf = adc_buf;
  memset(fft->wave, 0, sizeof(fft->wave));
}

void App_FFT_RefreshData(app_fft_t* fft) {
  SwitchVoltage(fft);

  Get_Voltage_Max(fft);
  Get_Voltage_Min(fft);
  Get_Voltage_v_vpp(fft);
}

static void Get_Amplitude(uint16_t* adc_buf) {
  /* 设置虚部为0 */
  for (int i = 0; i < adc_length; i++) {
    fft_inputbuf[i * 2] = adc_buf[i];
    fft_inputbuf[i * 2 + 1] = 0;
  }

  /* 1.快速傅里叶变换 */
  arm_cfft_f32(&arm_cfft_sR_f32_len1024, fft_inputbuf, 0, 1);

  /* 2.求出复数的绝对值 (获取复数振幅) */
  arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, adc_length);

  /* 3.归一化 */

  for (int i = 1; i < adc_length; i++) {
    fft_outputbuf[i] /= (adc_length / 2);
  }
  fft_outputbuf[0] = 0;
}

static void DislodgeCycle(app_fft_t* fft) {
  Get_Amplitude(fft->adc_buf);

  /* 求取基波频率 基波的幅度是频谱中最大的 基波频率=原波形频率
   * 频率 = (峰值索引* 采样率) / FFT长度 */
  arm_max_f32(fft_outputbuf, adc_length, &fft->frequency_max,
              &fft->frequency_max_pos);
  fft->frequency = (float)400000 / 1024 * fft->frequency_max_pos;

  /* 求取周期点数 */
  fft->npoints = 400000 / fft->frequency;
}

float App_FFT_GetK(app_fft_t* fft) {
  Get_Amplitude(fft->adc_buf);

  arm_max_f32(fft_outputbuf, adc_length, &fft->frequency_max,
              &fft->frequency_max_pos);
  // fft->frequency = (float)400000 / 1024 * fft->frequency_max_pos;

  return fft_outputbuf[fft->frequency_max_pos] / fft_outputbuf[3 * fft->frequency_max_pos];
}

float App_FFT_GetRms(app_fft_t* fft) {
  DislodgeCycle(fft);

  for (int i = 0; i < fft->npoints; i++) {
    wave1[i] = fft->wave[i];  // 取出一个周期的点存在wave1[i]中
  }

  /* 将波形平移到与时间轴对称的位置 */
  for (int i = 0; i < fft->npoints; i++) {
    wave1[i] = wave1[i] - fft->voltage_min - fft->voltage_vpp / 2;
  }

  // DSP库封装好的求rms值函数 不再手动实现 原型： arm_rms_f32 (const float32_t
  // *pSrc, uint32_t blockSize, float32_t *pResult)
  arm_rms_f32(wave1, fft->npoints, &fft->rms);
  // 平移后波形的幅度就是所求的v_vpp的一半
  fft->rms = fft->rms / (fft->voltage_vpp / 2.0);
  return fft->rms;
}
