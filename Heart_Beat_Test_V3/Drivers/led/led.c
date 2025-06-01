/**
 * @file led.c
 * @author BusyBox (busybox177634@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-12-25
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "gpio.h"
#include "stm32f4xx_hal.h"

#include "led/led.h"

void led_ds0_on()
{
    HAL_GPIO_WritePin(LED_DS0_GPIO_Port, LED_DS0_Pin, GPIO_PIN_RESET);
}

void led_ds0_off()
{
    HAL_GPIO_WritePin(LED_DS0_GPIO_Port, LED_DS0_Pin, GPIO_PIN_SET);
}

void led_ds0_toggle()
{
    HAL_GPIO_TogglePin(LED_DS0_GPIO_Port, LED_DS0_Pin);
}

void led_ds1_on()
{
    HAL_GPIO_WritePin(LED_DS1_GPIO_Port, LED_DS1_Pin, GPIO_PIN_RESET);
}

void led_ds1_off()
{
    HAL_GPIO_WritePin(LED_DS1_GPIO_Port, LED_DS1_Pin, GPIO_PIN_SET);
}

void led_ds1_toggle()
{
    HAL_GPIO_TogglePin(LED_DS1_GPIO_Port, LED_DS1_Pin);
}