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
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "atk_md0350/atk_md0350.h"
#include "math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint32_t freq = 100;  // 全局频率变量

uint16_t SinBuffer[200];
#define PI 3.141592653589693
void signalSin(){
    for(int i = 0; i < 200; i++){
        SinBuffer[i] = (uint16_t)((sin(i*2*PI/200) + 1) * (4096.0/2.0) * (1.0/3.3));
        //SinBuffer[i] = (sin(i*2*PI/200)*0.5+0.5) * (4095.0/3.3);
        if (SinBuffer[i] > 4095) SinBuffer[i] = 4095;
    }
}

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

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
  /* USER CODE BEGIN 2 */

    HAL_Delay(100);
    atk_md0350_init();

    signalSin();
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t *)SinBuffer, 200, DAC_ALIGN_12B_R);

//LED_Test
//    HAL_GPIO_WritePin(GPIOF,GPIO_PIN_9,GPIO_PIN_SET);
//    HAL_GPIO_WritePin(GPIOF,GPIO_PIN_10,GPIO_PIN_SET);

    atk_md0350_show_string(30, 300, 160, 80, "Made by Shirley He", ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);
    atk_md0350_show_string(310, 280, 160, 80, "KEY2:freq_up_DAC", ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);
    atk_md0350_show_string(310, 300, 160, 80, "KEY0:freq_down_DAC", ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);
//    atk_md0350_show_string(360, 300, 120, 120, "freq:100.00", ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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
#define MIN_FREQ 100     // �???低频�???100Hz
#define MAX_FREQ 10000   // �???高频�???10kHz
#define FREQ_STEP 100    // 频率步进100Hz

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

    TIM7->ARR = (uint32_t)(422000.0f / freq + 0.0) - 1 ;

    // 打印 ARR 的�??
//    char arr_str[20];
//    snprintf(arr_str, sizeof(arr_str), "ARR: %lu", TIM7->ARR);
//    atk_md0350_show_string(360, 300, 120, 120, arr_str, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);
    // 可�?�：打印频率（freq�?
//    char freq_str[20];
//    snprintf(freq_str, sizeof(freq_str), "freq: %.2f", freq);
//    atk_md0350_show_string(360, 320, 120, 120, freq_str, ATK_MD0350_LCD_FONT_16, ATK_MD0350_BLACK);
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
