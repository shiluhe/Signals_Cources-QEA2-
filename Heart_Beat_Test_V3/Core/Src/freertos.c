/* USER CODE BEGIN Header */
/**
 * @file freertos.c
 * @author BusyBox (busybox177634@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-12-26
 *
 * @copyright Copyright (c) 2024
 *
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "usart.h"
#include "math.h"
#include "string.h"
#include "stdio.h"
#include "arm_math.h"
#include "led/led.h"
#include "atk_md0350/atk_md0350.h"
#include "ads1292/ads1292.h"
#include "iir_filter/iir_filter.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for drawTask */
osThreadId_t drawTaskHandle;
const osThreadAttr_t drawTask_attributes = {
  .name = "drawTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for fftTask */
osThreadId_t fftTaskHandle;
const osThreadAttr_t fftTask_attributes = {
  .name = "fftTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for beepTask */
osThreadId_t beepTaskHandle;
const osThreadAttr_t beepTask_attributes = {
  .name = "beepTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for keyScanTask */
osThreadId_t keyScanTaskHandle;
const osThreadAttr_t keyScanTask_attributes = {
  .name = "keyScanTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for adsAvaiable */
osSemaphoreId_t adsAvaiableHandle;
const osSemaphoreAttr_t adsAvaiable_attributes = {
  .name = "adsAvaiable"
};
/* Definitions for drawValueAvailable */
osSemaphoreId_t drawValueAvailableHandle;
const osSemaphoreAttr_t drawValueAvailable_attributes = {
  .name = "drawValueAvailable"
};
/* Definitions for fftAvailable */
osSemaphoreId_t fftAvailableHandle;
const osSemaphoreAttr_t fftAvailable_attributes = {
  .name = "fftAvailable"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

volatile double draw_value = 0;
volatile int16_t* current_fft = 0;
volatile int enable_beep = 0;
volatile int using_test_signal = 0;
volatile int using_square = 0;

volatile int show_fft = 1;

static int16_t wave_buff0[4096];
static int16_t wave_buff1[4096];
static int16_t last_fft[4096];
static int16_t wave_fft[4096];
static int current_index = 0;
static int current_wave_append = 0;

static arm_rfft_instance_q15 fft_instance;

void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
    if (pin == GPIO_PIN_4) {
        osSemaphoreRelease(adsAvaiableHandle);
        //led_ds1_off();
        led_ds0_off();
        static int a = 0;
        a++;
        if (a % 500 == 0)
            led_ds1_toggle();

    }
}


void lcd_draw_heartbeat(double value)
{
//    value = - value; /* 由于LCD绘制方向，需要翻转一下图像 */

    char msg_title[64];
    snprintf(msg_title, sizeof(msg_title), "Your Heart_Beat");
    atk_md0350_show_string(180, 0, 250, 16, msg_title, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);


//    static const int x1 = 5, y1 = 10, x2 = 432, y2 = 150; /* 绘制范围 */
    static const int x1 = 5, y1 = 20, x2 = 390, y2 = 150; /* 绘制范围 */
    atk_md0350_draw_line(x1, y1, x1, y2, ATK_MD0350_BLACK);
    atk_md0350_draw_line(x1, y2, x2, y2, ATK_MD0350_BLACK);

    static int x = x1;
    static const double scale = 0.4;
    static int last_draw_x = x1, last_draw_y = (y1 + y2) / 2;
    static double min_value = 1.0, max_value = -1.0;
    static double c_min_value = 1.0, c_max_value = -1.0;

    if (x > (x2 / scale)) {
        x = x1;
        atk_md0350_fill(last_draw_x, y1, last_draw_x + 1, y2, ATK_MD0350_WHITE);
        min_value = c_min_value, max_value = c_max_value;
        c_min_value = 1.0; c_max_value = -1.0;


        /* 绘制最高，最低信号值 */
        char Peak_voltage[64];
        snprintf(Peak_voltage, sizeof(Peak_voltage), "Peak-V:");
        atk_md0350_show_string(400, 20, 80, 16, Peak_voltage, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);
        char Valley_voltage[64];
        snprintf(Valley_voltage, sizeof(Valley_voltage), "Valley-V:");
        atk_md0350_show_string(400, 60, 80, 16, Valley_voltage, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);

        char msg_max_value[64];
        char msg_min_value[64];
        snprintf(msg_max_value, sizeof(msg_max_value), " %.2lfmV", -max_value);
        snprintf(msg_min_value, sizeof(msg_min_value), "%.2lfmV", -min_value);
        atk_md0350_fill(400, 40, 400 + 50, 40 + 12, ATK_MD0350_WHITE);
        atk_md0350_show_string(400, 40, 50, 12, msg_min_value, ATK_MD0350_LCD_FONT_12, ATK_MD0350_BLACK);
        atk_md0350_fill(400, 80, 400 + 50, 80 + 12, ATK_MD0350_WHITE);
        atk_md0350_show_string(400, 80, 50, 12, msg_max_value, ATK_MD0350_LCD_FONT_12, ATK_MD0350_BLACK);
    }

    /* 寻找幅值范围 */
    if (value > c_max_value) c_max_value = value;
    if (value < c_min_value) c_min_value = value;

    int draw_x = scale * x++;
    int draw_y = y1 + 3 + (value - min_value) * (y2 - y1) / (max_value - min_value) * 0.95;

    /* 限制绘制幅度 */
    if (draw_x > (x2 / scale)) draw_x = x2;
    if (draw_x < x1) draw_x = x1;
    if (draw_y > y2) draw_y = y2;
    if (draw_y < y1) draw_y = y1;

    /* 绘制之前清除上一次的图形 */
    atk_md0350_fill(last_draw_x + 1, y1, draw_x + 1, y2, ATK_MD0350_WHITE);
    if (draw_x > last_draw_x || (draw_x == last_draw_x && draw_x != x1)) atk_md0350_draw_line(last_draw_x, last_draw_y, draw_x, draw_y, ATK_MD0350_RED);

    last_draw_x = draw_x;
    last_draw_y = draw_y;


    /* 计算心率 */
    static const int window_size = 20;
    static int window_index = 0;
    static double window[32] = {0.0};
    static int delay_cnt = 0;
    static int time_index = 0;
    static int last_time_index = 0;
    time_index++;

    window[window_index++] = value;
    if (window_index >= window_size) window_index = 0;

    double window_min_value = 1.0, window_max_value = -0.1;
    for (int i = 0; i < window_size; ++i) {
        if (window[i] < window_min_value) window_min_value = window[i];
        if (window[i] > window_max_value) window_max_value = window[i];
    }

    if (window_max_value - window_min_value > (max_value - min_value) * 0.5 && delay_cnt == 0) { /* 0.5为心跳阈值 */
        delay_cnt = 100;

        int heart_rate = 1000.0 / ((time_index - last_time_index) * 2) * 60;

        last_time_index = time_index;
        enable_beep = 1;
        char msg_heart_rate[64];
        atk_md0350_fill(10, 300, 10 + 150, 300 + 16, ATK_MD0350_WHITE);
        if(heart_rate < 210){
            snprintf(msg_heart_rate, sizeof(msg_heart_rate), "heart_rate:%d", heart_rate);
            atk_md0350_show_string(10, 300, 150, 16, msg_heart_rate, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);
        }
       else{
            snprintf(msg_heart_rate, sizeof(msg_heart_rate), "CAN NOT TEST");
            atk_md0350_show_string(10, 300, 150, 16, msg_heart_rate, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);
       }
    }
    delay_cnt -= 1;
    if (delay_cnt < 0) delay_cnt = 0;

    if (max_value - min_value < 0.05) { /* 没有心跳了 */
        enable_beep = 1;
    }
}

static void draw_fft(int16_t* wave, int start, int end)
{
    char msg_title[64];
    snprintf(msg_title, sizeof(msg_title), "About Spectrum");
    atk_md0350_show_string(180, 150, 250, 16, msg_title, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);

//    static const int x1 = 5, y1 = 180, x2 = 432, y2 = 310; /* 绘制范围 */
    static const int x1 = 5, y1 = 170, x2 = 400, y2 = 280; /* 绘制范围 */
    atk_md0350_fill(x1, y1, x2, y2, ATK_MD0350_WHITE);
//    atk_md0350_draw_line(x1, y1, x2, y1, ATK_MD0350_BLACK);
//    atk_md0350_draw_line(x2, y1, x2, y2, ATK_MD0350_BLACK);
    atk_md0350_draw_line(x1, y1, x1, y2, ATK_MD0350_BLACK);
    atk_md0350_draw_line(x1, y2, x2, y2, ATK_MD0350_BLACK);

    int16_t min_value = 0x7fff, max_value = 0;
    /* 寻找幅值范围 */
    for (int index = start; index < end; ++index) {
        if (wave[index] > max_value) max_value = wave[index];
        if (wave[index] < min_value) min_value = wave[index];
    }

    int last_draw_x = -1, last_draw_y = 0;
    for (int index = start; index < end; ++index) {

        int draw_x = x1 + (index - start) * (x2 - x1) / (end - start);
        int draw_y = y2 - (wave[index] - min_value) * (y2 - y1) / (max_value - min_value) * 0.95;

        if (draw_x > x2) draw_x = x2;
        if (draw_x < x1) draw_x = x1;
        if (draw_y > y2) draw_y = y2;
        if (draw_y < y1) draw_y = y1;

        if (last_draw_x == -1) {
            last_draw_x = draw_x;
            last_draw_y = draw_y;
            continue ;
        }
        atk_md0350_draw_line(last_draw_x, last_draw_y, draw_x, draw_y, ATK_MD0350_BLUE);
        last_draw_x = draw_x;
        last_draw_y = draw_y;
    }
//    char msg_fft[64];
//    snprintf(msg_fft, sizeof(msg_fft), "FFT %d", show_fft);
//    atk_md0350_fill(435, 298, 432 + 50, 298 + 12, ATK_MD0350_WHITE);
//    atk_md0350_show_string(435, 298, 80, 40, msg_fft, ATK_MD0350_LCD_FONT_12, ATK_MD0350_BLACK);
}

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void DrawTask(void *argument);
void FFTTask(void *argument);
void BeepTask(void *argument);
void KeyScanTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of adsAvaiable */
  adsAvaiableHandle = osSemaphoreNew(1, 1, &adsAvaiable_attributes);

  /* creation of drawValueAvailable */
  drawValueAvailableHandle = osSemaphoreNew(1, 1, &drawValueAvailable_attributes);

  /* creation of fftAvailable */
  fftAvailableHandle = osSemaphoreNew(1, 1, &fftAvailable_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of drawTask */
  drawTaskHandle = osThreadNew(DrawTask, NULL, &drawTask_attributes);

  /* creation of fftTask */
  fftTaskHandle = osThreadNew(FFTTask, NULL, &fftTask_attributes);

  /* creation of beepTask */
  beepTaskHandle = osThreadNew(BeepTask, NULL, &beepTask_attributes);

  /* creation of keyScanTask */
  keyScanTaskHandle = osThreadNew(KeyScanTask, NULL, &keyScanTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */

    struct IIR_Handle notch_50hz;
    struct IIR_Handle notch_100hz;
    struct IIR_Handle low_pass;
    struct IIR_Handle high_pass;

    iir_filter_init(&notch_50hz, 1.0, -1.52226142, 0.88161859, 0.9408093, -1.52226142, 0.9408093);
    iir_filter_init(&notch_100hz, 1.0, -0.54871515, 0.77567951, 0.88783976, -0.54871515, 0.88783976);
    iir_filter_init(&low_pass, 1.0, -1.64745998, 0.70089678, 0.0133592, 0.0267184, 0.0133592);
    iir_filter_init(&high_pass, 1.0, -1.91119707, 0.91497583, 0.95654323, -1.91308645, 0.95654323);

    while (1) {
        if (osSemaphoreAcquire(adsAvaiableHandle, pdMS_TO_TICKS(portMAX_DELAY)) == osOK) {
//          double value = ads1292_read_channel2();
            double value;

            if(!using_square && using_test_signal){

                value = ads1292_read_channel1();//原始信号导入MATLAB可以进行滤波
                //用测量仪的时候加value*系数，测人心跳时不加
                //value = value * 0.0001;

            }
            else if(using_square && !using_test_signal){

                value = ads1292_read_channel2();//可以显示内部方波
            }
            else{
                value = ads1292_read_channel1();//波形好看
                value = iir_filter_process(&notch_50hz, value);
                value = iir_filter_process(&notch_100hz, value);
                value = iir_filter_process(&low_pass, value);
                value = iir_filter_process(&high_pass, value);
                //用测量仪的时候加value*系数，测人心跳时不加
                //value = value * 0.0001;

//                if (fabs(value) < 200) value *= 0.3;
//
//                value = value * 7;
            }

            char msg[64];
            snprintf(msg, sizeof(msg), "%.4lf\r\n", - value);
            HAL_UART_Transmit(&huart1, msg, strlen(msg), 100);

            draw_value = value;
            osSemaphoreRelease(drawValueAvailableHandle);

            if (current_wave_append) {
                wave_buff1[current_index++] = value * 1000;
                if (current_index >= 4096) {
                    current_index = 0;
                    current_wave_append = 0;
                    current_fft = wave_buff1;

                    osSemaphoreRelease(fftAvailableHandle);
                }
            } else {
                wave_buff0[current_index++] = value * 1000;
                if (current_index >= 4096) {
                    current_index = 0;
                    current_wave_append = 1;
                    current_fft = wave_buff0;

                    osSemaphoreRelease(fftAvailableHandle);
                }
            }
        }
    }

  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_DrawTask */
/**
* @brief Function implementing the drawTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_DrawTask */
void DrawTask(void *argument)
{
  /* USER CODE BEGIN DrawTask */
  /* Infinite loop */
  for(;;)
  {
      if (osSemaphoreAcquire(drawValueAvailableHandle, pdMS_TO_TICKS(portMAX_DELAY)) == osOK) {
          portENTER_CRITICAL();
          //draw_value = ads1292_read_channel1();
          lcd_draw_heartbeat(draw_value);
          portEXIT_CRITICAL();
      }
  }
  /* USER CODE END DrawTask */
}

/* USER CODE BEGIN Header_FFTTask */
/**
* @brief Function implementing the fftTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_FFTTask */
void FFTTask(void *argument)
{
  /* USER CODE BEGIN FFTTask */
  /* Infinite loop */
  for(;;)
  {
      if (osSemaphoreAcquire(fftAvailableHandle, pdMS_TO_TICKS(portMAX_DELAY)) == osOK) {
          if (show_fft) {
              uint16_t fftSize = 4096;      //定义rfft的长度
              uint8_t ifftFlag = 0;         //表示fft变换为正变换,1则为逆变换
              arm_rfft_init_q15(&fft_instance, fftSize, ifftFlag, 1);
              arm_rfft_q15(&fft_instance, current_fft, wave_fft);
              for (int i = 0; i < 4096; ++i) wave_fft[i] = fabs(wave_fft[i]);

              portENTER_CRITICAL();
              draw_fft(wave_fft, 0, 1024);
              portEXIT_CRITICAL();
          }
      }
    osDelay(1);
  }
  /* USER CODE END FFTTask */
}

/* USER CODE BEGIN Header_BeepTask */
/**
* @brief Function implementing the beepTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_BeepTask */
void BeepTask(void *argument)
{
  /* USER CODE BEGIN BeepTask */
  /* Infinite loop */
  for(;;)
  {
//      if (enable_beep) {
//          enable_beep = 0;
//          led_ds0_on();
//          HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
//          osDelay(100);
//          led_ds0_off();
//          HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
//      }
    osDelay(1);
  }
  /* USER CODE END BeepTask */
}

/* USER CODE BEGIN Header_KeyScanTask */
/**
* @brief Function implementing the keyScanTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_KeyScanTask */
void KeyScanTask(void *argument)
{
  /* USER CODE BEGIN KeyScanTask */

  int key0 = 0, key0_press = 0;
  int key1 = 1, key1_press = 0;
  int key2 = 0, key2_press = 0;

  /* Infinite loop */
  for(;;)
  {
      if (HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == GPIO_PIN_RESET) {
          osDelay(50);
          if (HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin) == GPIO_PIN_RESET) {
              if (!key0_press) {
                  key0 = !key0;

                  if (key0) {
                      ads1292_use_test_signal();
                      //方波
                      using_square = 1;
                  } else {
                      ads1292_use_external_signal();
                      //滤波后信号
                      using_square = 0;
                  }
              }
              key0_press = 1;
          }
      } else {
          key0_press = 0;
      }

      if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET) {
          osDelay(50);
          if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET) {
              if (!key1_press) {
                  key1 = !key1;

                  if (key1) {
                      show_fft = 1;
                      portENTER_CRITICAL();
                      draw_fft(0, 0, 0);
                      portEXIT_CRITICAL();
                  } else {
                      show_fft = 0;
                      portENTER_CRITICAL();
                      draw_fft(0, 0, 0);
                      portEXIT_CRITICAL();
                  }
              }

              key1_press = 1;
          }
      } else {
          key1_press = 0;
      }

      if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET) {
          osDelay(50);
          if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET) {
              if (!key2_press) {
                  key2 = !key2;

                  if (key2) {
                      using_test_signal = 1;

                  } else {
                      using_test_signal = 0;

                  }

              }

              key2_press = 1;
          }
      } else {
          key2_press = 0;
      }


    osDelay(1);
  }
  /* USER CODE END KeyScanTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

