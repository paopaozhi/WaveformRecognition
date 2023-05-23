#include <stdio.h>

#include "app_fft1.h"
#include "arm_const_structs.h"
#include "arm_math.h"

#define length 1024  // ADC采集的点的个数
// static uint16_t i, j;  // 循环变量，length 改变及时修改变量范围
static uint32_t Amax_pos, min_pos;  // 存储最大值最小值下标
static uint16_t flag[length];       // 是否被搜索过
static uint8_t n;                   // 第几大、小值
static uint16_t Npoints;            // 一个周期内的点的个数
static float fft_inputbuf[length * 2];
static float fft_outputbuf[length];
/*
AdcConvEnd用来检测测ADC是否采集完毕
0：没有采集完
1：采集完毕，在stm32f1xx_it里的DMA完成中断进行修改
 */
static __IO uint8_t AdcConvEnd = 0;        //__IO 防止MDK优化
static float wave[length], wave1[length];  // 存放adc转换后的波形的值
static float RMS;
static float fre, Amax;
static float k;  // 频率
static uint16_t d;
static float temp;

static double sqr(double x) { return x * x; }

int APP_Fft_K(void) {
  for (int i = 0; i < length; i++) {
    wave[i] = ftt_test.adc_buf[i] * 3.3 / 4095;  // 转换电压
  }

  for (int i = 0; i < length; i++) {
    fft_inputbuf[i * 2] = wave[i];
    fft_inputbuf[i * 2 + 1] = 0;
  }

  arm_cfft_f32(&arm_cfft_sR_f32_len1024, fft_inputbuf, 0, 1);
  arm_cmplx_mag_f32(fft_inputbuf, fft_outputbuf, length);
  fft_outputbuf[0] /= 1024;

  for (int i = 1; i < length; i++) {
    fft_outputbuf[i] /= 512;
  }

  n = 1;
  memset(flag, 0, sizeof(flag));
  for (int i = 1; i <= n; i++) {
    Amax = 0.0;
    for (int j = 1; j < length / 2; j++) {
      if (flag[j] == 1) break;
      if (fft_outputbuf[j] > Amax) {
        Amax = fft_outputbuf[j];
        Amax_pos = j;
      }
    }
    flag[Amax_pos] = 1;
  }

  fre = 500000 / 1024 * Amax_pos;
  printf("frep : %.2f \r\n", fre);
  /*********聚集能量，减小栅栏效应*******/

  if (fre > 1100) {
    d = Amax_pos - 2;  // 计算要聚集能量的范围
                       // ,这里判断波形只需要基波和二次谐波，所以我们只聚两个点
    temp = sqr(fft_outputbuf[Amax_pos]);  // 暂时存储 平方之和
    for (int i = 1; i <= d; i++) {
      temp = temp + sqr(fft_outputbuf[Amax_pos + i]) +
             sqr(fft_outputbuf[Amax_pos - i]);
      fft_outputbuf[Amax_pos + i] = 0;
      fft_outputbuf[Amax_pos - i] = 0;
    }
    fft_outputbuf[Amax_pos] = sqrt(temp);
  }

  d = Amax_pos - 2;
  temp = sqr(fft_outputbuf[Amax_pos * 3]);
  for (int i = 1; i <= d; i++) {
    temp = temp + sqr(fft_outputbuf[Amax_pos * 3 + i]) +
           sqr(fft_outputbuf[Amax_pos * 3 - i]);
    fft_outputbuf[Amax_pos * 3 + i] = 0;
    fft_outputbuf[Amax_pos * 3 - i] = 0;
  }
  fft_outputbuf[Amax_pos * 3] = sqrt(temp);

  /*********判断波形*********/
  // for (i = 0; i < length / 2; i++) {
  //   printf("$%f;", fft_outputbuf[i]);
  // }
  // for (i = 0; i < length ; i++)
  //    printf("%f\r\n", wave[i]);

  k = fft_outputbuf[Amax_pos] / fft_outputbuf[3 * Amax_pos];

  printf("k值=%f\r\n", k);
  printf("基波幅度=%f\r\n", fft_outputbuf[Amax_pos]);
  printf("二次谐波幅度=%f\r\n", fft_outputbuf[3 * Amax_pos]);
  printf("基波所在下标=%d\r\n", Amax_pos);

  return k;
}
