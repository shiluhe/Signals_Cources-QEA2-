/**
 ****************************************************************************************************
 * @file        atk_md0350_fsmc.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-06-21
 * @brief       ATK-MD0350模块FSMC接口驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 探索者 F407开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#ifndef __ATK_MD0350_FSMC_H
#define __ATK_MD0350_FSMC_H

#include "gpio.h"
#include "stdint.h"

/* ATK-MD0350模块FSMC接口定义 */
#define ATK_MD0350_FSMC_BANK                    FSMC_NORSRAM_BANK4                          /* ATK-MD0350模块所接FSMC的Bank */
#define ATK_MD0350_FSMC_BANK_ADDR               (0x6C000000)
#define ATK_MD0350_FSMC_REG_SEL                 (6)
#define ATK_MD0350_FSMC_READ_AST                0x0F                                        /* 读时序的地址建立时间，单位：HCLK */
#define ATK_MD0350_FSMC_READ_DST                0x3C                                        /* 读时序的数据建立时间，单位：HCLK */
#define ATK_MD0350_FSMC_WRITE_AST               0x02                                        /* 写时序的地址建立时间，单位：HCLK */
#define ATK_MD0350_FSMC_WRITE_DST               0x02                                        /* 写时序的数据建立时间，单位：HCLK */
#define ATK_MD0350_FSMC_CLK_ENABLE()            do{ __HAL_RCC_FSMC_CLK_ENABLE(); }while(0)  /* ATK-MD0350模块所接FSMC时钟使能 */

/* ATK-MD0350模块FSMC接口读写命令、数据地址 */
#define ATK_MD0350_FSMC_CMD_ADDR                (ATK_MD0350_FSMC_BANK_ADDR | (((1U << ATK_MD0350_FSMC_REG_SEL) - 1) << 1))
#define ATK_MD0350_FSMC_DAT_ADDR                (ATK_MD0350_FSMC_BANK_ADDR | ((1U << ATK_MD0350_FSMC_REG_SEL) << 1))

/* ATK-MD0350模块FSMC接口读写命令、数据 */
#define ATK_MD0350_FSMC_CMD_REG                 (*(volatile uint16_t *)ATK_MD0350_FSMC_CMD_ADDR)
#define ATK_MD0350_FSMC_DAT_REG                 (*(volatile uint16_t *)ATK_MD0350_FSMC_DAT_ADDR)

/* 引脚定义 */
#define ATK_MD0350_FSMC_RS_GPIO_PORT            GPIOF
#define ATK_MD0350_FSMC_RS_GPIO_PIN             GPIO_PIN_12
#define ATK_MD0350_FSMC_RS_GPIO_AF              GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_RS_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOF_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_CS_GPIO_PORT            GPIOG
#define ATK_MD0350_FSMC_CS_GPIO_PIN             GPIO_PIN_12
#define ATK_MD0350_FSMC_CS_GPIO_AF              GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_CS_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOG_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_RD_GPIO_PORT            GPIOD
#define ATK_MD0350_FSMC_RD_GPIO_PIN             GPIO_PIN_4
#define ATK_MD0350_FSMC_RD_GPIO_AF              GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_RD_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_WR_GPIO_PORT            GPIOD
#define ATK_MD0350_FSMC_WR_GPIO_PIN             GPIO_PIN_5
#define ATK_MD0350_FSMC_WR_GPIO_AF              GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_WR_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D0_GPIO_PORT            GPIOD
#define ATK_MD0350_FSMC_D0_GPIO_PIN             GPIO_PIN_14
#define ATK_MD0350_FSMC_D0_GPIO_AF              GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D0_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D1_GPIO_PORT            GPIOD
#define ATK_MD0350_FSMC_D1_GPIO_PIN             GPIO_PIN_15
#define ATK_MD0350_FSMC_D1_GPIO_AF              GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D1_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D2_GPIO_PORT            GPIOD
#define ATK_MD0350_FSMC_D2_GPIO_PIN             GPIO_PIN_0
#define ATK_MD0350_FSMC_D2_GPIO_AF              GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D2_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D3_GPIO_PORT            GPIOD
#define ATK_MD0350_FSMC_D3_GPIO_PIN             GPIO_PIN_1
#define ATK_MD0350_FSMC_D3_GPIO_AF              GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D3_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D4_GPIO_PORT            GPIOE
#define ATK_MD0350_FSMC_D4_GPIO_PIN             GPIO_PIN_7
#define ATK_MD0350_FSMC_D4_GPIO_AF              GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D4_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D5_GPIO_PORT            GPIOE
#define ATK_MD0350_FSMC_D5_GPIO_PIN             GPIO_PIN_8
#define ATK_MD0350_FSMC_D5_GPIO_AF              GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D5_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D6_GPIO_PORT            GPIOE
#define ATK_MD0350_FSMC_D6_GPIO_PIN             GPIO_PIN_9
#define ATK_MD0350_FSMC_D6_GPIO_AF              GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D6_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D7_GPIO_PORT            GPIOE
#define ATK_MD0350_FSMC_D7_GPIO_PIN             GPIO_PIN_10
#define ATK_MD0350_FSMC_D7_GPIO_AF              GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D7_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D8_GPIO_PORT            GPIOE
#define ATK_MD0350_FSMC_D8_GPIO_PIN             GPIO_PIN_11
#define ATK_MD0350_FSMC_D8_GPIO_AF              GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D8_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D9_GPIO_PORT            GPIOE
#define ATK_MD0350_FSMC_D9_GPIO_PIN             GPIO_PIN_12
#define ATK_MD0350_FSMC_D9_GPIO_AF              GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D9_GPIO_CLK_ENABLE()    do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D10_GPIO_PORT           GPIOE
#define ATK_MD0350_FSMC_D10_GPIO_PIN            GPIO_PIN_13
#define ATK_MD0350_FSMC_D10_GPIO_AF             GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D10_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D11_GPIO_PORT           GPIOE
#define ATK_MD0350_FSMC_D11_GPIO_PIN            GPIO_PIN_14
#define ATK_MD0350_FSMC_D11_GPIO_AF             GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D11_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D12_GPIO_PORT           GPIOE
#define ATK_MD0350_FSMC_D12_GPIO_PIN            GPIO_PIN_15
#define ATK_MD0350_FSMC_D12_GPIO_AF             GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D12_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D13_GPIO_PORT           GPIOD
#define ATK_MD0350_FSMC_D13_GPIO_PIN            GPIO_PIN_8
#define ATK_MD0350_FSMC_D13_GPIO_AF             GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D13_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D14_GPIO_PORT           GPIOD
#define ATK_MD0350_FSMC_D14_GPIO_PIN            GPIO_PIN_9
#define ATK_MD0350_FSMC_D14_GPIO_AF             GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D14_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)
#define ATK_MD0350_FSMC_D15_GPIO_PORT           GPIOD
#define ATK_MD0350_FSMC_D15_GPIO_PIN            GPIO_PIN_10
#define ATK_MD0350_FSMC_D15_GPIO_AF             GPIO_AF12_FSMC
#define ATK_MD0350_FSMC_D15_GPIO_CLK_ENABLE()   do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)

/* 操作函数 */
void atk_md0350_fsmc_init(void);                                                            /* ATK-MD0350模块FSMC接口初始化 */
static inline void atk_md0350_fsmc_write_cmd(volatile uint16_t cmd)                         /* ATK-MD0350模块通过FSMC接口写命令 */
{
    ATK_MD0350_FSMC_CMD_REG = cmd;
}
static inline void atk_md0350_fsmc_write_dat(volatile uint16_t dat)                         /* ATK-MD0350模块通过FSMC接口写数据 */
{
    ATK_MD0350_FSMC_DAT_REG = dat;
}
static inline void atk_md0350_fsmc_write_reg(volatile uint16_t reg, volatile uint16_t dat)  /* ATK-MD0350模块通过FSMC接口写寄存器 */
{
    ATK_MD0350_FSMC_CMD_REG = reg;
    ATK_MD0350_FSMC_DAT_REG = dat;
}
static inline uint16_t atk_md0350_fsmc_read_dat(void)                              /* ATK-MD0350模块通过FSMC接口读数据 */
{
    uint16_t dat;

    dat = ATK_MD0350_FSMC_DAT_REG;

    return dat;
}

#endif