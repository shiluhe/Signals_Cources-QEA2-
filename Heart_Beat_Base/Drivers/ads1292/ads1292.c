/**
 * @file ads1292.c
 * @author BusyBox (busybox177634@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-12-26
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "usart.h"
#include "string.h"
#include "stdio.h"
#include "gpio.h"
#include "spi.h"
#include "stm32f4xx_hal.h"

#include "ads1292/ads1292.h"


#define CMD_WAKEUP 0x02
#define CMD_STANDBY 0x04
#define CMD_RESET 0x06
#define CMD_START 0x08
#define CMD_STOP 0x0a
#define CMD_OFFSETCAL 0x1a

#define CMD_RDATAC 0x10
#define CMD_SDATAC 0x11
#define CMD_RDATA 0x12

#define CMD_RREG 0b00100000
#define CMD_WREG 0b01000000

#define REG_ID 0x00
#define REG_CONFIG1 0x01
#define REG_CONFIG2 0x02
#define REG_LOFF 0x03
#define REG_CH1SET 0x04
#define REG_CH2SET 0x05
#define REG_RLD_SENS 0x06
#define REG_LOFF_SENS 0x07
#define REG_LOFF_STAT 0x08
#define REG_RESP1 0x09
#define REG_RESP2 0x0a
#define REG_GPIO 0x0b

int ads1292_init()
{
    HAL_GPIO_WritePin(ADS_START_GPIO_Port, ADS_START_Pin, GPIO_PIN_RESET);

    uint8_t id = 0;
    while (id != 0x73 && id != 0x53) {
        ads1292_send_cmd(CMD_SDATAC);
        HAL_Delay(10);

        id = ads1292_read_register(REG_ID);

        char msg[64];
        snprintf(msg, sizeof(msg), "id:%d\r\n", id);
        HAL_UART_Transmit(&huart1, msg, strlen(msg), 100);
        HAL_Delay(20);
    }

    ads1292_write_register(REG_CONFIG2, 0xa0); //使用内部参考电压
    ads1292_write_register(REG_CONFIG1, 0x02); //设置转换速率为500SPS
    ads1292_write_register(REG_CH1SET, 0x00); // PGA gain 6
    ads1292_write_register(REG_CH2SET, 0x00); // PGA gain 6
    ads1292_write_register(REG_RLD_SENS, 0x2c);
    ads1292_write_register(REG_RESP1, 0x02);
    ads1292_write_register(REG_RESP2, 0x03);

    ads1292_send_cmd(CMD_RDATAC);
    HAL_GPIO_WritePin(ADS_START_GPIO_Port, ADS_START_Pin, GPIO_PIN_SET);
    return 0;
}

int ads1292_use_test_signal()
{
    ads1292_write_register(REG_CONFIG2, 0xa3);
    ads1292_write_register(REG_CH2SET, 0b0101);
    return 0;
}

int ads1292_use_external_signal()
{
    ads1292_write_register(REG_CONFIG2, 0xa0);
    ads1292_write_register(REG_CH2SET, 0x00);
    return 0;
}

uint8_t ads1292_read_register(uint8_t address)
{
    HAL_GPIO_WritePin(ADS_CS_GPIO_Port, ADS_CS_Pin, GPIO_PIN_RESET);
    uint8_t buff[3] = {0};
    buff[0] = CMD_RREG | address;
    buff[1] = 0;
    HAL_SPI_TransmitReceive(&hspi1, buff, buff, 3, 100);
    HAL_GPIO_WritePin(ADS_CS_GPIO_Port, ADS_CS_Pin, GPIO_PIN_SET);
    return buff[2];
}

int ads1292_write_register(uint8_t address, uint8_t data)
{
    HAL_GPIO_WritePin(ADS_CS_GPIO_Port, ADS_CS_Pin, GPIO_PIN_RESET);
    uint8_t buff[3] = {0};
    buff[0] = CMD_WREG | address;
    buff[1] = 0;
    buff[2] = data;
    HAL_SPI_TransmitReceive(&hspi1, buff, buff, 3, 100);
    HAL_Delay(1);
    HAL_GPIO_WritePin(ADS_CS_GPIO_Port, ADS_CS_Pin, GPIO_PIN_SET);
    return 0;
}

int ads1292_send_cmd(uint8_t cmd)
{
    HAL_GPIO_WritePin(ADS_CS_GPIO_Port, ADS_CS_Pin, GPIO_PIN_RESET);
    uint8_t buff[1] = {0};
    buff[0] = cmd;
    HAL_SPI_TransmitReceive(&hspi1, buff, buff, 1, 100);
    HAL_Delay(1);
    HAL_GPIO_WritePin(ADS_CS_GPIO_Port, ADS_CS_Pin, GPIO_PIN_SET);
    return 0;
}

double ads1292_read_channel1()
{
    HAL_GPIO_WritePin(ADS_CS_GPIO_Port, ADS_CS_Pin, GPIO_PIN_RESET);
    uint8_t buff[10] = {0};
    buff[0] = CMD_RDATA;
    HAL_SPI_TransmitReceive(&hspi1, buff, buff, 10, 100);
    HAL_GPIO_WritePin(ADS_CS_GPIO_Port, ADS_CS_Pin, GPIO_PIN_SET);

    uint32_t value = (((uint32_t)buff[4] & 0xff) << 16) | (((uint32_t)buff[5] & 0xff) << 8) | (((uint32_t)buff[6] & 0xff) << 0);

    // 提取24位有符号数的符号位
    uint32_t sign = (value >> 23) & 1;
    if (sign) {
        // 如果符号位为1，说明是负数，进行符号扩展
        return (int32_t)(value | 0xFF000000);
    }

    /* 转换为mV */
    return (int32_t)value * 2.42 / 6.0 * 1000.0 / ((1 << 23) - 1) + 0.07; /* 0.07mV电压偏移 */
}

double ads1292_read_channel2()
{
    HAL_GPIO_WritePin(ADS_CS_GPIO_Port, ADS_CS_Pin, GPIO_PIN_RESET);
    uint8_t buff[10] = {0};
    buff[0] = CMD_RDATA;
    HAL_SPI_TransmitReceive(&hspi1, buff, buff, 10, 100);
    HAL_GPIO_WritePin(ADS_CS_GPIO_Port, ADS_CS_Pin, GPIO_PIN_SET);

    uint32_t value = (((uint32_t)buff[7] & 0xff) << 16) | (((uint32_t)buff[8] & 0xff) << 8) | (((uint32_t)buff[9] & 0xff) << 0);

    // 提取24位有符号数的符号位
    uint32_t sign = (value >> 23) & 1;
    if (sign) {
        // 如果符号位为1，说明是负数，进行符号扩展
        value = value | 0xFF000000;
    }

    /* 转换为mV */
    return (int32_t)value * 2.42 / 6.0 * 1000.0 / ((1 << 23) - 1) + 0.07; /* 0.07mV电压偏移 */
}