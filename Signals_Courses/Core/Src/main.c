/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"

#include "atk_md0350/atk_md0350.h"
#include "math.h"

#include "arm_math.h"
#include "arm_const_structs.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint32_t freq = 100;  // 全局频率

uint16_t SinBuffer[200];
//#define PI 3.141592653589793f
void signalSin(){
    for(int i = 0; i < 200; i++){
        SinBuffer[i] = (uint16_t)((sin(i*2*PI/200) + 1) * (4096.0/2.0) * (1/3.3));
        //SinBuffer[i] = (sin(i*2*PI/200)*0.5+0.5) * (4095.0/3.3);
        if (SinBuffer[i] > 4095) SinBuffer[i] = 4095;
    }
}

//#define SAMPLING_RATE  400000.0f// sampling_rate
float SAMPLING_RATE = 400000.0f;
#define ADC_BUFFER_SIZE      4096
#define FFT_SIZE             ADC_BUFFER_SIZE
#define MAGNITUDE_THRESHOLD     0.02f
#define PEAK_RATIO_THRESHOLD    1.5f
#define WINDOW_SIZE             10

#define ADC_REF_VOLTAGE      3.3f
#define ADC_RESOLUTION       4095.0f

uint16_t adc_buffer[ADC_BUFFER_SIZE];
float voltage_buffer[ADC_BUFFER_SIZE];
float complex_input[ADC_BUFFER_SIZE * 2];
float fft_output[ADC_BUFFER_SIZE];
float peak_frequencies[ADC_BUFFER_SIZE / 2];
float peak_magnitudes[ADC_BUFFER_SIZE / 2];
uint32_t peak_count = 0;

#define AVG_WINDOW_SIZE 10
static float amplitude_history[AVG_WINDOW_SIZE] = {0};
static int history_index = 0;

uint8_t ADC_Cplt_Flag = 0;// 转换完成标志
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/**
  ******************************************************************************
    *name: Change_TIM3_Freq(float target_sampling_rate)
    *func：改变TIM3参数来改变实际采样率
   ******************************************************************************
  */
float Change_TIM3_Freq(float target_sampling_rate){
    // 遍历PSC和ARR组合，寻找最接近目标采样率的组合
    uint32_t best_psc = 0;
    uint32_t best_arr = 0;
    float min_diff = 1e9;
    float real_sampling_rate = 0.0f;

    for (uint32_t psc = 0; psc <= 65535; psc++) {
        uint32_t arr = (uint32_t)(84000000.0f / target_sampling_rate / (psc + 1)) - 1;
        if (arr > 65535) continue;

        float actual = 84000000.0f / ((psc + 1) * (arr + 1));
        float diff = fabsf(actual - target_sampling_rate);

        if (diff < min_diff) {
            min_diff = diff;
            best_psc = psc;
            best_arr = arr;
            real_sampling_rate = actual;
        }
        if (diff < 1.0f) break;  // 提前�?�?
    }
    TIM3->ARR = best_arr;
    TIM3->PSC = best_psc;
    real_sampling_rate = 84000000.0f / ((best_arr + 1) * (best_psc + 1));
    return real_sampling_rate;
}
/**
  ******************************************************************************
    *name: Process_FFT_and_Spectrum()
    *func：FFT + 频谱分析
   ******************************************************************************
  */
void Process_FFT_and_Spectrum() {

    // 1. 执行FFT-4096
    arm_cfft_f32(&arm_cfft_sR_f32_len4096, complex_input, 0, 1);
    //计算幅度
    arm_cmplx_mag_f32(complex_input, fft_output, FFT_SIZE);

    // 3. 频谱分析：寻找峰
    peak_count = 0;
    for (int i = 1; i < FFT_SIZE / 2 - WINDOW_SIZE; i += WINDOW_SIZE) {
        // 3.1 计算当前窗口的平�??
        float window_avg = 0;
        for (int j = 0; j < WINDOW_SIZE; j++) {
            window_avg += fft_output[i + j];
        }
        window_avg /= WINDOW_SIZE;

        // 3.2 在窗口内寻找
        float max_magnitude = 0;
        int max_index = i;
        for (int j = 0; j < WINDOW_SIZE; j++) {
            if (fft_output[i + j] > max_magnitude && fft_output[i + j] >= MAGNITUDE_THRESHOLD) {
                max_magnitude = fft_output[i + j];
                max_index = i + j;
            }
        }

        // 3.3查是否为有效
        if (max_magnitude > 0) {
            float neighbor_avg = 0;
            int neighbor_count = 0;
            for (int k = -5; k <= 5; k++) {
                if (k != 0 && (max_index + k) >= 0 && (max_index + k) < FFT_SIZE / 2) {
                    neighbor_avg += fft_output[max_index + k];
                    neighbor_count++;
                }
            }
            neighbor_avg /= neighbor_count;

            if (max_magnitude >= neighbor_avg * PEAK_RATIO_THRESHOLD) {
                peak_frequencies[peak_count] = (float)max_index * (SAMPLING_RATE / FFT_SIZE); // 频率=索引*频率分辨
                peak_magnitudes[peak_count] = max_magnitude;
                peak_count++;
            }
        }
    }

    // 4. 确定主频信号（幅度最高的�??
    float main_freq = 0;
    float main_magnitude = 0;
    for (int i = 0; i < peak_count; i++) {
        if (peak_magnitudes[i] > main_magnitude) {
            main_magnitude = peak_magnitudes[i];
            main_freq = peak_frequencies[i];
        }
    }
    //main_freq = ((int)(main_freq + 50)) / 100 * 100;

    // 5. 计算信号幅度-DC分量×2
    float dc_offset = fft_output[0] / FFT_SIZE;  // 直流分量归一
    float signal_amplitude = dc_offset * 2;      // 幅度=直流×2
//    float signal_amplitude = main_magnitude / (FFT_SIZE / 2);  // 归一
    // 更新滑动平均缓冲
    amplitude_history[history_index] = signal_amplitude;
    history_index = (history_index + 1) % AVG_WINDOW_SIZE;

    // 计算滑动平均
    float smoothed_amplitude = 0;
    for (int i = 0; i < AVG_WINDOW_SIZE; i++) {
        smoothed_amplitude += amplitude_history[i];
    }
    smoothed_amplitude /= AVG_WINDOW_SIZE;

    //预计频率(INTO-B)
//    target_sampling_rate = 4.0f * main_freq;
//    // 限制采样率范围在1kHz ~ 400kHz
//    if (target_sampling_rate > 400000.0f) {
//        target_sampling_rate = 400000.0f;
//    } else if (target_sampling_rate < 1000.0f) {
//        target_sampling_rate = 1000.0f;
//    }
//    SAMPLING_RATE = Change_TIM3_Freq(target_sampling_rate);
    // 设置为进入模式B

    // 输出结果（可通过串口或LCD显示
    //printf("Main Frequency: %.2f Hz, Amplitude: %.2f V\n", main_freq, signal_amplitude);
    char freq_str[20];
    snprintf(freq_str, sizeof(freq_str), "ADC_freq: %.2f", main_freq);
    atk_md0350_show_string(60, 240, 160, 60, freq_str, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);

    char amp_str[20];
    snprintf(amp_str, sizeof(amp_str), "ADC_amplitude: %.2f", smoothed_amplitude*1.0);
    atk_md0350_show_string(260, 240, 160, 60, amp_str, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);

}

/**
  ******************************************************************************
    *Slavename: Do_FFT_And_Find_Main_Freq_Magnitude()
    * Mastername: Process_FFT_and_Spectrum_NEW()
    *func：带通采样定�??+动�?�调整采样频�??
   ******************************************************************************
  */
//float main_freq = 0, amp = 0;
void Do_FFT_And_Find_Main_Freq_Magnitude(float* main_freq, float* smoothed_amplitude) {
    // 1. 执行FFT-4096
    arm_cfft_f32(&arm_cfft_sR_f32_len4096, complex_input, 0, 1);
    arm_cmplx_mag_f32(complex_input, fft_output, FFT_SIZE);
    // 2. 寻找频谱峰�??
    peak_count = 0;
    for (int i = 1; i < FFT_SIZE / 2 - WINDOW_SIZE; i += WINDOW_SIZE) {
        float window_avg = 0;
        for (int j = 0; j < WINDOW_SIZE; j++) {
            window_avg += fft_output[i + j];
        }
        window_avg /= WINDOW_SIZE;
        float max_magnitude = 0;
        int max_index = i;
        for (int j = 0; j < WINDOW_SIZE; j++) {
            if (fft_output[i + j] > max_magnitude && fft_output[i + j] >= MAGNITUDE_THRESHOLD) {
                max_magnitude = fft_output[i + j];
                max_index = i + j;
            }
        }
        if (max_magnitude > 0) {
            float neighbor_avg = 0;
            int neighbor_count = 0;
            for (int k = -5; k <= 5; k++) {
                if (k != 0 && (max_index + k) >= 0 && (max_index + k) < FFT_SIZE / 2) {
                    neighbor_avg += fft_output[max_index + k];
                    neighbor_count++;
                }
            }
            neighbor_avg /= neighbor_count;

            if (max_magnitude >= neighbor_avg * PEAK_RATIO_THRESHOLD) {
                peak_frequencies[peak_count] = (float)max_index * (SAMPLING_RATE / FFT_SIZE);
                peak_magnitudes[peak_count] = max_magnitude;
                peak_count++;
            }
        }
    }
    // 3. 提取主频
    float local_main_freq = 0;
    float main_magnitude = 0;
    for (int i = 0; i < peak_count; i++) {
        if (peak_magnitudes[i] > main_magnitude) {
            main_magnitude = peak_magnitudes[i];
            local_main_freq = peak_frequencies[i];
        }
    }
    // 4. 计算信号幅度（用 DC 分量 × 2 表示大振幅）
    float dc_offset = fft_output[0] / FFT_SIZE;
    float signal_amplitude = dc_offset * 2;
//    float max_magnitude = fft_output[max_index];  // 对应主频点的幅度
//    float signal_amplitude = max_magnitude / (FFT_SIZE / 2);  // 归一化幅值

    // 5. 滑动平均
    amplitude_history[history_index] = signal_amplitude;
    history_index = (history_index + 1) % AVG_WINDOW_SIZE;
    float avg_amplitude = 0;
    for (int i = 0; i < AVG_WINDOW_SIZE; i++) {
        avg_amplitude += amplitude_history[i];
    }
    avg_amplitude /= AVG_WINDOW_SIZE;
    // 6. 输出
    *main_freq = local_main_freq;
    *smoothed_amplitude = avg_amplitude;
}

void Process_FFT_and_Spectrum_NEW() {
    //SAMPLING_RATE = 400000.0f;
    SAMPLING_RATE = Change_TIM3_Freq(400000.0f);

    float target_sampling_rate = 0;
    float main_freq , amp;
    Do_FFT_And_Find_Main_Freq_Magnitude((float *) &main_freq, &amp);

    //INTO-B
    target_sampling_rate = 4 * main_freq;

//    // 限制采样率范围在1kHz ~ 400kHz
    if (target_sampling_rate > 400000) {
        target_sampling_rate = 400000;
    } else if (target_sampling_rate < 1000) {
        target_sampling_rate = 1000;
    }
    SAMPLING_RATE = Change_TIM3_Freq(target_sampling_rate);

//    Do_FFT_And_Find_Main_Freq_Magnitude((float *) &main_freq, &amp);
    // 设置为进入模式B

    // 输出结果（可通过串口或LCD显示
    //printf("Main Frequency: %.2f Hz, Amplitude: %.2f V\n", main_freq, signal_amplitude);
    char freq_str[20];
    snprintf(freq_str, sizeof(freq_str), "ADC_freq: %.2f", main_freq);
    atk_md0350_show_string(60, 240, 160, 60, freq_str, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);

    char amp_str[20];
    snprintf(amp_str, sizeof(amp_str), "ADC_amplitude: %.2f", amp*1.0);
    atk_md0350_show_string(260, 240, 160, 60, amp_str, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);

}
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */


/**
  ******************************************************************************
    *name:Draw_On_TFT()
    *func：将波形显示在TFT屏幕�??
   ******************************************************************************
  */
#define X_START 0
#define X_END   480
#define Y_START 0
#define Y_END   200

void Draw_On_TFT(float* buffer) {
    int i;
    int screen_width = X_END - X_START;
    int screen_height = Y_END - Y_START;

    if (ADC_BUFFER_SIZE < 2) return;

    float x_step = (float)screen_width / (ADC_BUFFER_SIZE - 1);

    // 清除波形显示区域
    atk_md0350_fill(0, 0, 480, 215, ATK_MD0350_WHITE);

    // 画波形
    for (i = 0; i < ADC_BUFFER_SIZE; i++) {
        float voltage = buffer[i];  // 使用传入的数据
        uint16_t y = Y_END - (uint16_t)((voltage / 3.3f) * screen_height);
        uint16_t x = X_START + (uint16_t)(i * x_step);

        atk_md0350_draw_point(x, y, ATK_MD0350_BLACK);
    }
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FSMC_Init();
  MX_DAC_Init();
  MX_TIM7_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
    int analysis_step = 0;  // 0=初步分析, 1=正式采样
    float estimated_freq = 1000.0f;  // 默认值防止第一次出现采样率过低
    HAL_Delay(100);
    atk_md0350_init();

    signalSin();
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t *)SinBuffer, 200, DAC_ALIGN_12B_R);

    HAL_ADC_Start_DMA(&hadc1,(uint32_t *)adc_buffer,ADC_BUFFER_SIZE);

    //LED_Test
    //    HAL_GPIO_WritePin(GPIOF,GPIO_PIN_9,GPIO_PIN_RESET);
    //    HAL_GPIO_WritePin(GPIOF,GPIO_PIN_10,GPIO_PIN_RESET);

    atk_md0350_show_string(20, 280, 240, 80, "Made by Shirley He", ATK_MD0350_LCD_FONT_24, ATK_MD0350_BLACK);
    atk_md0350_show_string(310, 280, 160, 60, "KEY2:DAC_freq_up", ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);
    atk_md0350_show_string(310, 300, 160, 60, "KEY0:DAC_freq_down", ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);
    atk_md0350_draw_line(0, 220, 480, 220, ATK_MD0350_BLACK);

//    atk_md0350_show_string(360, 300, 120, 120, "freq:100.00", ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if(ADC_Cplt_Flag == 1){
        ADC_Cplt_Flag = 0;

        // 1.ADC原始值转电压
        for (int i = 0; i < ADC_BUFFER_SIZE; i++) {
            voltage_buffer[i] = (adc_buffer[i] / 4095.0f) * 3.3f;
        }

        // 2.复数数组
        for (int i = 0; i < ADC_BUFFER_SIZE; i++) {
            complex_input[2 * i] = voltage_buffer[i];
            complex_input[2 * i + 1] = 0.0f;
        }

        float main_freq, amp;
        if (analysis_step == 0) {
            // 第一步：用默认400k分析频率
            SAMPLING_RATE = Change_TIM3_Freq(400000.0f);
            Do_FFT_And_Find_Main_Freq_Magnitude(&main_freq, &amp);

            estimated_freq = main_freq;  // 记录分析得到的频率
            float target_sampling_rate = 4 * estimated_freq;
            if (target_sampling_rate > 400000) target_sampling_rate = 400000;
            if (target_sampling_rate < 1000) target_sampling_rate = 1000;

            SAMPLING_RATE = Change_TIM3_Freq(target_sampling_rate);

            float voltage_buffer_display[ADC_BUFFER_SIZE];
            memcpy(voltage_buffer_display, voltage_buffer, sizeof(voltage_buffer));
            Draw_On_TFT(voltage_buffer_display);

            analysis_step = 1;  // 下一帧再采样一次并分析

        }
        else {
            // 第二步：采样率已设好，分析正式数据
            Do_FFT_And_Find_Main_Freq_Magnitude(&main_freq, &amp);
            //atk_md0350_fill(0, 0, 480, 215, ATK_MD0350_WHITE);
            char freq_str[20];
            snprintf(freq_str, sizeof(freq_str), "ADC_freq: %.3f", main_freq);
            atk_md0350_show_string(60, 240, 160, 60, freq_str, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);

            char amp_str[20];
            snprintf(amp_str, sizeof(amp_str), "ADC_amplitude: %.2f", amp);
            atk_md0350_show_string(260, 240, 160, 60, amp_str, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);

            //Draw_On_TFT();  // 显示曲线
            analysis_step = 0;  // 下一轮重新估计频率
        }

        HAL_TIM_Base_Start(&htim3);
        HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, ADC_BUFFER_SIZE);

    }

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
#define MIN_FREQ 100     // 低频100Hz
#define MAX_FREQ 10000   // 高频10kHz
#define FREQ_STEP 100    // 步进100Hz

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

    static uint32_t last_tick = 0;    // 上次响应的时间戳
    const uint32_t debounce_time = 200; // 防抖时间

    uint32_t current_tick = HAL_GetTick(); // 获取当前系统时间

    // 判断是否超过防抖时间
    if (current_tick - last_tick < debounce_time) {
        return;
    }
    last_tick = current_tick;

    if (GPIO_Pin == GPIO_PIN_2) {
        //atk_md0350_clear(ATK_MD0350_WHITE);

        if (freq + FREQ_STEP <= MAX_FREQ) {
            freq += FREQ_STEP;
        } else {
            freq = MAX_FREQ;
        }
    }
    else if (GPIO_Pin == GPIO_PIN_4) {
        //atk_md0350_clear(ATK_MD0350_WHITE);

        if (freq - FREQ_STEP >= MIN_FREQ) {
            freq -= FREQ_STEP;
        } else {
            freq = MIN_FREQ;
        }
    }

    TIM7->ARR = (uint32_t)(422000.0f / freq + 0.0) - 1 ;//tim7_tim_not_correct
    // 打印 ARR
//    char arr_str[20];
//    snprintf(arr_str, sizeof(arr_str), "ARR: %lu", TIM7->ARR);
//    atk_md0350_show_string(360, 300, 120, 120, arr_str, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);
    // freq
//    char freq_str[20];
//    snprintf(freq_str, sizeof(freq_str), "freq: %.2f", freq);
//    atk_md0350_show_string(360, 320, 120, 120, freq_str, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
    //HAL_GPIO_WritePin(GPIOF,GPIO_PIN_9,GPIO_PIN_RESET);
    atk_md0350_fill(0, 240, 480, 275, ATK_MD0350_WHITE);
    if(hadc->Instance == ADC1){
        HAL_TIM_Base_Stop(&htim3);//stop_tim3_convtick
        HAL_ADC_Stop_DMA(&hadc1);  //stop_DMA

        ADC_Cplt_Flag = 1;

    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
