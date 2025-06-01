/**
 ****************************************************************************************************
 * @file        atk_md0350.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-06-21
 * @brief       ATK-MD0350模块驱动代码
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


#include "atk_md0350/atk_md0350_fsmc.h"
#include "atk_md0350/atk_md0350_font.h"

#include "atk_md0350/atk_md0350.h"


/* ATK-MD0350模块LCD驱动器ID */
#define ATK_MD0350_CHIP_ID1         0x5310
#define ATK_MD0350_CHIP_ID2         0x7796

/* ATK-MD0350模块LCD扫描方向 */
#define ATK_MD0350_SCAN_DIR_L2R_U2D (0x0000)
#define ATK_MD0350_SCAN_DIR_L2R_D2U (0x0080)
#define ATK_MD0350_SCAN_DIR_R2L_U2D (0x0040)
#define ATK_MD0350_SCAN_DIR_R2L_D2U (0x00C0)
#define ATK_MD0350_SCAN_DIR_U2D_L2R (0x0020)
#define ATK_MD0350_SCAN_DIR_U2D_R2L (0x0060)
#define ATK_MD0350_SCAN_DIR_D2U_L2R (0x00A0)
#define ATK_MD0350_SCAN_DIR_D2U_R2L (0x00E0)

/* ATK-MD0350模块状态数据结构体 */
static struct
{
    uint16_t chip_id;                   /* 驱动器ID */
    uint16_t width;                     /* LCD宽度 */
    uint16_t height;                    /* LCD高度 */
    atk_md0350_lcd_scan_dir_t scan_dir; /* LCD扫描方向 */
    atk_md0350_lcd_disp_dir_t disp_dir; /* LCD显示方向 */
} g_atk_md0350_sta = {0};

/**
 * @brief       ATK-MD0350模块硬件初始化
 * @param       无
 * @retval      无
 */
static void atk_md0350_hw_init(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};

    /* 使能时钟 */
    ATK_MD0350_BL_GPIO_CLK_ENABLE();

    /* 初始化BL引脚 */
    gpio_init_struct.Pin    = ATK_MD0350_BL_GPIO_PIN;
    gpio_init_struct.Mode   = GPIO_MODE_OUTPUT_PP;
    gpio_init_struct.Pull   = GPIO_PULLUP;
    gpio_init_struct.Speed  = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ATK_MD0350_BL_GPIO_PORT, &gpio_init_struct);

    ATK_MD0350_BL(0);
}

/**
 * @brief       获取ATK-MD0350模块驱动器ID
 * @param       无
 * @retval      无
 */
static inline uint16_t atk_md0350_get_chip_id(void)
{
    uint16_t chip_id;

    atk_md0350_fsmc_write_cmd(0xD4);
    chip_id = atk_md0350_fsmc_read_dat();
    chip_id = atk_md0350_fsmc_read_dat();
    chip_id = atk_md0350_fsmc_read_dat() << 8;
    chip_id |= (atk_md0350_fsmc_read_dat()) & 0x00FF;

    if (chip_id != ATK_MD0350_CHIP_ID1)
    {
        /* 尝试获取ID2 */
        atk_md0350_fsmc_write_cmd(0xD3);
        chip_id = atk_md0350_fsmc_read_dat();
        chip_id = atk_md0350_fsmc_read_dat();
        chip_id = atk_md0350_fsmc_read_dat() << 8;
        chip_id |= (atk_md0350_fsmc_read_dat()) & 0x00FF;
    }

    return chip_id;
}

/**
 * @brief       ATK-MD0350模块寄存器初始化
 * @param       无
 * @retval      无
 */
static void atk_md0350_reg_init(void)
{
    if (g_atk_md0350_sta.chip_id == ATK_MD0350_CHIP_ID1)
    {
        atk_md0350_fsmc_write_cmd(0xED);
        atk_md0350_fsmc_write_dat(0x01);
        atk_md0350_fsmc_write_dat(0xFE);
        atk_md0350_fsmc_write_cmd(0xEE);
        atk_md0350_fsmc_write_dat(0xDE);
        atk_md0350_fsmc_write_dat(0x21);
        atk_md0350_fsmc_write_cmd(0xF1);
        atk_md0350_fsmc_write_dat(0x01);
        atk_md0350_fsmc_write_cmd(0xDF);
        atk_md0350_fsmc_write_dat(0x10);
        atk_md0350_fsmc_write_cmd(0xC4);
        atk_md0350_fsmc_write_dat(0x8F);
        atk_md0350_fsmc_write_cmd(0xC6);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xE2);
        atk_md0350_fsmc_write_dat(0xE2);
        atk_md0350_fsmc_write_dat(0xE2);
        atk_md0350_fsmc_write_cmd(0xBF);
        atk_md0350_fsmc_write_dat(0xAA);
        atk_md0350_fsmc_write_cmd(0xB0);
        atk_md0350_fsmc_write_dat(0x0D);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x0D);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x11);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x19);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x21);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x2D);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x3D);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x5D);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x5D);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xB1);
        atk_md0350_fsmc_write_dat(0x80);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x8B);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x96);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xB2);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x02);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x03);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xB3);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xB4);
        atk_md0350_fsmc_write_dat(0x8B);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x96);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xA1);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xB5);
        atk_md0350_fsmc_write_dat(0x02);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x03);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x04);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xB6);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xB7);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x3F);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x5E);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x64);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x8C);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xAC);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xDC);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x70);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x90);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xEB);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xDC);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xB8);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xBA);
        atk_md0350_fsmc_write_dat(0x24);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xC1);
        atk_md0350_fsmc_write_dat(0x20);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x54);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xFF);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xC2);
        atk_md0350_fsmc_write_dat(0x0A);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x04);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xC3);
        atk_md0350_fsmc_write_dat(0x3C);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x3A);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x39);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x37);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x3C);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x36);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x32);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x2F);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x2C);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x29);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x26);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x24);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x24);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x23);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x3C);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x36);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x32);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x2F);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x2C);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x29);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x26);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x24);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x24);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x23);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xC4);
        atk_md0350_fsmc_write_dat(0x62);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x05);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x84);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xF0);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x18);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xA4);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x18);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x50);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x0C);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x17);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x95);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xF3);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xE6);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xC5);
        atk_md0350_fsmc_write_dat(0x32);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x44);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x65);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x76);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x88);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xC6);
        atk_md0350_fsmc_write_dat(0x20);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x17);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x01);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xC7);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xC8);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xC9);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xE0);
        atk_md0350_fsmc_write_dat(0x16);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x1C);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x21);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x36);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x46);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x52);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x64);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x7A);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x8B);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x99);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xA8);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xB9);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xC4);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xCA);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xD2);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xD9);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xE0);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xF3);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xE1);
        atk_md0350_fsmc_write_dat(0x16);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x1C);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x22);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x36);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x45);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x52);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x64);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x7A);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x8B);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x99);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xA8);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xB9);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xC4);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xCA);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xD2);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xD8);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xE0);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xF3);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xE2);
        atk_md0350_fsmc_write_dat(0x05);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x0B);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x1B);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x34);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x44);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x4F);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x61);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x79);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x88);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x97);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xA6);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xB7);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xC2);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xC7);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xD1);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xD6);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xDD);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xF3);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xE3);
        atk_md0350_fsmc_write_dat(0x05);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xA);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x1C);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x33);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x44);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x50);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x62);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x78);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x88);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x97);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xA6);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xB7);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xC2);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xC7);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xD1);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xD5);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xDD);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xF3);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xE4);
        atk_md0350_fsmc_write_dat(0x01);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x01);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x02);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x2A);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x3C);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x4B);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x5D);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x74);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x84);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x93);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xA2);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xB3);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xBE);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xC4);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xCD);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xD3);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xDD);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xF3);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xE5);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x02);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x29);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x3C);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x4B);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x5D);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x74);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x84);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x93);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xA2);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xB3);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xBE);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xC4);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xCD);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xD3);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xDC);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xF3);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xE6);
        atk_md0350_fsmc_write_dat(0x11);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x34);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x56);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x76);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x77);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x66);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x88);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x99);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xBB);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x99);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x66);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x55);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x55);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x45);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x43);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x44);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xE7);
        atk_md0350_fsmc_write_dat(0x32);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x55);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x76);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x66);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x67);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x67);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x87);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x99);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xBB);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x99);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x77);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x44);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x56);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x23);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x33);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x45);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xE8);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x99);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x87);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x88);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x77);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x66);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x88);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xAA);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0xBB);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x99);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x66);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x55);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x55);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x44);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x44);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x55);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xE9);
        atk_md0350_fsmc_write_dat(0xAA);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0x00);
        atk_md0350_fsmc_write_dat(0xAA);
        atk_md0350_fsmc_write_cmd(0xCF);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xF0);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x50);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xF3);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0xF9);
        atk_md0350_fsmc_write_dat(0x06);
        atk_md0350_fsmc_write_dat(0x10);
        atk_md0350_fsmc_write_dat(0x29);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0x3A);
        atk_md0350_fsmc_write_dat(0x55);
        atk_md0350_fsmc_write_cmd(0x11);
        HAL_Delay(100);
        atk_md0350_fsmc_write_cmd(0x29);
        atk_md0350_fsmc_write_cmd(0x35);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_cmd(0x51);
        atk_md0350_fsmc_write_dat(0xFF);
        atk_md0350_fsmc_write_cmd(0x53);
        atk_md0350_fsmc_write_dat(0x2C);
        atk_md0350_fsmc_write_cmd(0x55);
        atk_md0350_fsmc_write_dat(0x82);
        atk_md0350_fsmc_write_cmd(0x2C);
    }
    else if (g_atk_md0350_sta.chip_id == ATK_MD0350_CHIP_ID2)
    {
        atk_md0350_fsmc_write_cmd(0x11);
        HAL_Delay(120);
        atk_md0350_fsmc_write_cmd(0x36);
        atk_md0350_fsmc_write_dat(0x48);
        atk_md0350_fsmc_write_cmd(0x3A);
        atk_md0350_fsmc_write_dat(0x55);
        atk_md0350_fsmc_write_cmd(0xF0);
        atk_md0350_fsmc_write_dat(0xC3);
        atk_md0350_fsmc_write_cmd(0xF0);
        atk_md0350_fsmc_write_dat(0x96);
        atk_md0350_fsmc_write_cmd(0xB4);
        atk_md0350_fsmc_write_dat(0x01);
        atk_md0350_fsmc_write_cmd(0xB6);
        atk_md0350_fsmc_write_dat(0x0A);
        atk_md0350_fsmc_write_dat(0xA2);
        atk_md0350_fsmc_write_cmd(0xB7);
        atk_md0350_fsmc_write_dat(0xC6);
        atk_md0350_fsmc_write_cmd(0xB9);
        atk_md0350_fsmc_write_dat(0x02);
        atk_md0350_fsmc_write_dat(0xE0);
        atk_md0350_fsmc_write_cmd(0xC0);
        atk_md0350_fsmc_write_dat(0x80);
        atk_md0350_fsmc_write_dat(0x16);
        atk_md0350_fsmc_write_cmd(0xC1);
        atk_md0350_fsmc_write_dat(0x19);
        atk_md0350_fsmc_write_cmd(0xC2);
        atk_md0350_fsmc_write_dat(0xA7);
        atk_md0350_fsmc_write_cmd(0xC5);
        atk_md0350_fsmc_write_dat(0x16);
        atk_md0350_fsmc_write_cmd(0xE8);
        atk_md0350_fsmc_write_dat(0x40);
        atk_md0350_fsmc_write_dat(0x8A);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x00);
        atk_md0350_fsmc_write_dat(0x29);
        atk_md0350_fsmc_write_dat(0x19);
        atk_md0350_fsmc_write_dat(0xA5);
        atk_md0350_fsmc_write_dat(0x33);
        atk_md0350_fsmc_write_cmd(0xE0);
        atk_md0350_fsmc_write_dat(0xF0);
        atk_md0350_fsmc_write_dat(0x07);
        atk_md0350_fsmc_write_dat(0x0D);
        atk_md0350_fsmc_write_dat(0x04);
        atk_md0350_fsmc_write_dat(0x05);
        atk_md0350_fsmc_write_dat(0x14);
        atk_md0350_fsmc_write_dat(0x36);
        atk_md0350_fsmc_write_dat(0x54);
        atk_md0350_fsmc_write_dat(0x4C);
        atk_md0350_fsmc_write_dat(0x38);
        atk_md0350_fsmc_write_dat(0x13);
        atk_md0350_fsmc_write_dat(0x14);
        atk_md0350_fsmc_write_dat(0x2E);
        atk_md0350_fsmc_write_dat(0x34);
        atk_md0350_fsmc_write_cmd(0xE1);
        atk_md0350_fsmc_write_dat(0xF0);
        atk_md0350_fsmc_write_dat(0x10);
        atk_md0350_fsmc_write_dat(0x14);
        atk_md0350_fsmc_write_dat(0x0E);
        atk_md0350_fsmc_write_dat(0x0C);
        atk_md0350_fsmc_write_dat(0x08);
        atk_md0350_fsmc_write_dat(0x35);
        atk_md0350_fsmc_write_dat(0x44);
        atk_md0350_fsmc_write_dat(0x4C);
        atk_md0350_fsmc_write_dat(0x26);
        atk_md0350_fsmc_write_dat(0x10);
        atk_md0350_fsmc_write_dat(0x12);
        atk_md0350_fsmc_write_dat(0x2C);
        atk_md0350_fsmc_write_dat(0x32);
        atk_md0350_fsmc_write_cmd(0xF0);
        atk_md0350_fsmc_write_dat(0x3C);
        atk_md0350_fsmc_write_cmd(0xF0);
        atk_md0350_fsmc_write_dat(0x69);
        HAL_Delay(120);
        atk_md0350_fsmc_write_cmd(0x21);
        atk_md0350_fsmc_write_cmd(0x29);
    }
}

/**
 * @brief       设置ATK-MD0350模块列地址
 * @param       无
 * @retval      无
 */
static void atk_md0350_set_column_address(uint16_t sc, uint16_t ec)
{
    atk_md0350_fsmc_write_cmd(0x2A);
    atk_md0350_fsmc_write_dat((uint8_t)(sc >> 8) & 0xFF);
    atk_md0350_fsmc_write_dat((uint8_t)sc & 0xFF);
    atk_md0350_fsmc_write_dat((uint8_t)(ec >> 8) & 0xFF);
    atk_md0350_fsmc_write_dat((uint8_t)ec & 0xFF);
}

/**
 * @brief       设置ATK-MD0350模块页地址
 * @param       无
 * @retval      无
 */
static void atk_md0350_set_page_address(uint16_t sp, uint16_t ep)
{
    atk_md0350_fsmc_write_cmd(0x2B);
    atk_md0350_fsmc_write_dat((uint8_t)(sp >> 8) & 0xFF);
    atk_md0350_fsmc_write_dat((uint8_t)sp & 0xFF);
    atk_md0350_fsmc_write_dat((uint8_t)(ep >> 8) & 0xFF);
    atk_md0350_fsmc_write_dat((uint8_t)ep & 0xFF);
}

/**
 * @brief       开始写ATK-MD0350模块显存
 * @param       无
 * @retval      无
 */
static void atk_md0350_start_write_memory(void)
{
    atk_md0350_fsmc_write_cmd(0x2C);
}

/**
 * @brief       开始读ATK-MD0350模块显存
 * @param       无
 * @retval      无
 */
static void atk_md0350_start_read_memory(void)
{
    atk_md0350_fsmc_write_cmd(0x2E);
}

/**
 * @brief       平方函数，x^y
 * @param       x: 底数
 *              y: 指数
 * @retval      x^y
 */
static uint32_t atk_md0350_pow(uint8_t x, uint8_t y)
{
    uint8_t loop;
    uint32_t res = 1;

    for (loop=0; loop<y; loop++)
    {
        res *= x;
    }

    return res;
}

/**
 * @brief       ATK-MD0350模块初始化
 * @param       无
 * @retval      ATK_MD0350_EOK  : ATK_MD0350模块初始化成功
 *              ATK_MD0350_ERROR: ATK_MD0350模块初始化失败
 */
uint8_t atk_md0350_init(void)
{
    uint16_t chip_id;

    atk_md0350_hw_init();               /* ATK-MD0350模块硬件初始化 */
    atk_md0350_fsmc_init();             /* ATK-MD0350模块FSMC接口初始化 */
    chip_id = atk_md0350_get_chip_id(); /* 获取ATK-MD0350模块驱动器ID */
    if ((chip_id != ATK_MD0350_CHIP_ID1) && (chip_id != ATK_MD0350_CHIP_ID2))
    {
        return ATK_MD0350_ERROR;
    }
    else
    {
        g_atk_md0350_sta.chip_id = chip_id;
        g_atk_md0350_sta.width = ATK_MD0350_LCD_WIDTH;
        g_atk_md0350_sta.height = ATK_MD0350_LCD_HEIGHT;
    }
    atk_md0350_reg_init();
    atk_md0350_set_disp_dir(ATK_MD0350_LCD_DISP_DIR_90);
    atk_md0350_clear(ATK_MD0350_WHITE);
    atk_md0350_display_on();
    atk_md0350_backlight_on();
#if (ATK_MD0350_USING_TOUCH != 0)
    atk_md0350_touch_init();
#endif

    return ATK_MD0350_EOK;
}

/**
 * @brief       获取ATK-MD0350模块LCD宽度
 * @param       无
 * @retval      ATK-MD0350模块LCD宽度
 */
uint16_t atk_md0350_get_lcd_width(void)
{
    return g_atk_md0350_sta.width;
}

/**
 * @brief       获取ATK-MD0350模块LCD高度
 * @param       无
 * @retval      ATK-MD0350模块LCD高度
 */
uint16_t atk_md0350_get_lcd_height(void)
{
    return g_atk_md0350_sta.height;
}

/**
 * @brief       开启ATK-MD0350模块LCD背光
 * @param       无
 * @retval      无
 */
void atk_md0350_backlight_on(void)
{
    ATK_MD0350_BL(1);
}

/**
 * @brief       关闭ATK-MD0350模块LCD背光
 * @param       无
 * @retval      无
 */
void atk_md0350_backlight_off(void)
{
    ATK_MD0350_BL(0);
}

/**
 * @brief       开启ATK-MD0350模块LCD显示
 * @param       无
 * @retval      无
 */
void atk_md0350_display_on(void)
{
    atk_md0350_fsmc_write_cmd(0x29);
}

/**
 * @brief       关闭ATK-MD0350模块LCD显示
 * @param       无
 * @retval      无
 */
void atk_md0350_display_off(void)
{
    atk_md0350_fsmc_write_cmd(0x28);
}

/**
 * @brief       设置ATK-MD0350模块LCD扫描方向
 * @param       scan_dir: ATK_MD0350_LCD_SCAN_DIR_L2R_U2D: 从左到右，从上到下
 *                        ATK_MD0350_LCD_SCAN_DIR_L2R_D2U: 从左到右，从下到上
 *                        ATK_MD0350_LCD_SCAN_DIR_R2L_U2D: 从右到左，从上到下
 *                        ATK_MD0350_LCD_SCAN_DIR_R2L_D2U: 从右到左，从下到上
 *                        ATK_MD0350_LCD_SCAN_DIR_U2D_L2R: 从上到下，从左到右
 *                        ATK_MD0350_LCD_SCAN_DIR_U2D_R2L: 从上到下，从右到左
 *                        ATK_MD0350_LCD_SCAN_DIR_D2U_L2R: 从下到上，从左到右
 *                        ATK_MD0350_LCD_SCAN_DIR_D2U_R2L: 从下到上，从右到左
 * @retval      ATK_MD0350_EOK   : 设置ATK-MD0350模块LCD扫描方向成功
 *              ATK_MD0350_ERROR : 设置ATK-MD0350模块LCD扫描方向失败
*               ATK_MD0350_EINVAL: 传入参数错误
 */
uint8_t atk_md0350_set_scan_dir(atk_md0350_lcd_scan_dir_t scan_dir)
{
    uint16_t reg36;

    switch (g_atk_md0350_sta.disp_dir)
    {
        case ATK_MD0350_LCD_DISP_DIR_0:
        {
            reg36 = (uint16_t)scan_dir;
            break;
        }
        case ATK_MD0350_LCD_DISP_DIR_90:
        {
            switch (scan_dir)
            {
                case ATK_MD0350_LCD_SCAN_DIR_L2R_U2D:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_U2D_R2L;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_L2R_D2U:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_U2D_L2R;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_R2L_U2D:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_D2U_R2L;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_R2L_D2U:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_D2U_L2R;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_U2D_L2R:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_R2L_U2D;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_U2D_R2L:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_R2L_D2U;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_D2U_L2R:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_L2R_U2D;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_D2U_R2L:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_L2R_D2U;
                    break;
                }
                default:
                {
                    return ATK_MD0350_EINVAL;
                }
            }
            break;
        }
        case ATK_MD0350_LCD_DISP_DIR_180:
        {
            switch (scan_dir)
            {
                case ATK_MD0350_LCD_SCAN_DIR_L2R_U2D:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_R2L_D2U;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_L2R_D2U:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_R2L_U2D;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_R2L_U2D:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_L2R_D2U;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_R2L_D2U:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_L2R_U2D;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_U2D_L2R:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_D2U_R2L;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_U2D_R2L:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_D2U_L2R;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_D2U_L2R:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_U2D_R2L;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_D2U_R2L:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_U2D_L2R;
                    break;
                }
                default:
                {
                    return ATK_MD0350_EINVAL;
                }
            }
            break;
        }
        case ATK_MD0350_LCD_DISP_DIR_270:
        {
            switch (scan_dir)
            {
                case ATK_MD0350_LCD_SCAN_DIR_L2R_U2D:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_D2U_L2R;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_L2R_D2U:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_D2U_R2L;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_R2L_U2D:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_U2D_L2R;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_R2L_D2U:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_U2D_R2L;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_U2D_L2R:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_L2R_D2U;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_U2D_R2L:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_L2R_U2D;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_D2U_L2R:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_R2L_D2U;
                    break;
                }
                case ATK_MD0350_LCD_SCAN_DIR_D2U_R2L:
                {
                    reg36 = ATK_MD0350_LCD_SCAN_DIR_R2L_U2D;
                    break;
                }
                default:
                {
                    return ATK_MD0350_EINVAL;
                }
            }
            break;
        }
        default:
        {
            return ATK_MD0350_ERROR;
        }
    }

    g_atk_md0350_sta.scan_dir = (atk_md0350_lcd_scan_dir_t)reg36;

    if (g_atk_md0350_sta.chip_id == ATK_MD0350_CHIP_ID2)   /* ST7796要设置BGR位 */
    {
        reg36 |= 0x08;
    }

    atk_md0350_fsmc_write_reg(0x36, reg36);
    atk_md0350_set_column_address(0, g_atk_md0350_sta.width - 1);
    atk_md0350_set_page_address(0, g_atk_md0350_sta.height - 1);

    return ATK_MD0350_EOK;
}

/**
 * @brief       设置ATK-MD0350模块LCD显示方向
 * @param       disp_dir: ATK_MD0350_LCD_DISP_DIR_0  : LCD顺时针旋转0°显示内容
 *                        ATK_MD0350_LCD_DISP_DIR_90 : LCD顺时针旋转90°显示内容
 *                        ATK_MD0350_LCD_DISP_DIR_180: LCD顺时针旋转180°显示内容
 *                        ATK_MD0350_LCD_DISP_DIR_270: LCD顺时针旋转270°显示内容
 * @retval      ATK_MD0350_EOK   : 设置ATK-MD0350模块LCD显示方向成功
 *              ATK_MD0350_EINVAL: 传入参数错误
 */
uint8_t atk_md0350_set_disp_dir(atk_md0350_lcd_disp_dir_t disp_dir)
{
    switch (disp_dir)
    {
        case ATK_MD0350_LCD_DISP_DIR_0:
        {
            g_atk_md0350_sta.width = ATK_MD0350_LCD_WIDTH;
            g_atk_md0350_sta.height = ATK_MD0350_LCD_HEIGHT;
            break;
        }
        case ATK_MD0350_LCD_DISP_DIR_90:
        {
            g_atk_md0350_sta.width = ATK_MD0350_LCD_HEIGHT;
            g_atk_md0350_sta.height = ATK_MD0350_LCD_WIDTH;
            break;
        }
        case ATK_MD0350_LCD_DISP_DIR_180:
        {
            g_atk_md0350_sta.width = ATK_MD0350_LCD_WIDTH;
            g_atk_md0350_sta.height = ATK_MD0350_LCD_HEIGHT;
            break;
        }
        case ATK_MD0350_LCD_DISP_DIR_270:
        {
            g_atk_md0350_sta.width = ATK_MD0350_LCD_HEIGHT;
            g_atk_md0350_sta.height = ATK_MD0350_LCD_WIDTH;
            break;
        }
        default:
        {
            return ATK_MD0350_EINVAL;
        }
    }

    g_atk_md0350_sta.disp_dir = disp_dir;
    atk_md0350_set_scan_dir(ATK_MD0350_LCD_SCAN_DIR_L2R_U2D);

    return ATK_MD0350_EOK;
}

/**
 * @brief       获取ATK-MD0350模块LCD扫描方向
 * @param       无
 * @retval      ATK-MD0350模块LCD扫描方向
 */
atk_md0350_lcd_scan_dir_t atk_md0350_get_scan_dir(void)
{
    return g_atk_md0350_sta.scan_dir;
}

/**
 * @brief       获取ATK-MD0350模块LCD显示方向
 * @param       无
 * @retval      ATK-MD0350模块LCD显示方向
 */
atk_md0350_lcd_disp_dir_t atk_md0350_get_disp_dir(void)
{
    return g_atk_md0350_sta.disp_dir;
}

/**
 * @brief       ATK-MD0350模块LCD区域填充
 * @param       xs   : 区域起始X坐标
 *              ys   : 区域起始Y坐标
 *              xe   : 区域终止X坐标
 *              ye   : 区域终止Y坐标
 *              color: 区域填充颜色
 * @retval      无
 */
void atk_md0350_fill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color)
{
    uint16_t x_index;
    uint16_t y_index;

    atk_md0350_set_column_address(xs, xe);
    atk_md0350_set_page_address(ys, ye);
    atk_md0350_start_write_memory();
    for (y_index=ys; y_index<=ye; y_index++)
    {
        for (x_index=xs; x_index<= xe; x_index++)
        {
            atk_md0350_fsmc_write_dat(color);
        }
    }
}

/**
 * @brief       ATK-MD0350模块LCD清屏
 * @param       color: 清屏颜色
 * @retval      无
 */
void atk_md0350_clear(uint16_t color)
{
    atk_md0350_fill(0, 0, ATK_MD0350_LCD_HEIGHT - 1, ATK_MD0350_LCD_WIDTH - 1, color);
}

/**
 * @brief       ATK-MD0350模块LCD画点
 * @param       x    : 待画点的X坐标
 *              y    : 待画点的Y坐标
 *              color: 待画点的颜色
 * @retval      无
 */
void atk_md0350_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
    atk_md0350_set_column_address(x, x);
    atk_md0350_set_page_address(y, y);
    atk_md0350_start_write_memory();
    atk_md0350_fsmc_write_dat(color);
}

/**
 * @brief       ATK-MD0350模块LCD读点
 * @param       x    : 待读点的X坐标
 *              y    : 待读点的Y坐标
 * @retval      待读点的颜色
 */
uint16_t atk_md0350_read_point(uint16_t x, uint16_t y)
{
    uint16_t color;
    uint8_t color_r;
    uint8_t color_g;
    uint8_t color_b;

    if ((x >= g_atk_md0350_sta.width) || (y >= g_atk_md0350_sta.height))
    {
        return ATK_MD0350_EINVAL;
    }

    atk_md0350_set_column_address(x, x);
    atk_md0350_set_page_address(y, y);
    atk_md0350_start_read_memory();

    color = atk_md0350_fsmc_read_dat(); /* Dummy */
    color = atk_md0350_fsmc_read_dat(); /* [15:11]: R, [7:2]:G */
    color_r = (uint8_t)(color >> 11) & 0x1F;
    color_g = (uint8_t)(color >> 2) & 0x3F;
    color = atk_md0350_fsmc_read_dat(); /* [15:11]: B */
    color_b = (uint8_t)(color >> 11) & 0x1F;

    return (uint16_t)(color_r << 11) | (color_g << 5) | color_b;
}

/**
 * @brief       ATK-MD0350模块LCD画线段
 * @param       x1   : 待画线段端点1的X坐标
 *              y1   : 待画线段端点1的Y坐标
 *              x2   : 待画线段端点2的X坐标
 *              y2   : 待画线段端点2的Y坐标
 *              color: 待画线段的颜色
 * @retval      无
 */
void atk_md0350_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t x_delta;
    uint16_t y_delta;
    int16_t x_sign;
    int16_t y_sign;
    int16_t error;
    int16_t error2;

    x_delta = (x1 < x2) ? (x2 - x1) : (x1 - x2);
    y_delta = (y1 < y2) ? (y2 - y1) : (y1 - y2);
    x_sign = (x1 < x2) ? 1 : -1;
    y_sign = (y1 < y2) ? 1 : -1;
    error = x_delta - y_delta;

    atk_md0350_draw_point(x2, y2, color);

    while ((x1 != x2) || (y1 != y2))
    {
        atk_md0350_draw_point(x1, y1, color);

        error2 = error << 1;
        if (error2 > -y_delta)
        {
            error -= y_delta;
            x1 += x_sign;
        }

        if (error2 < x_delta)
        {
            error += x_delta;
            y1 += y_sign;
        }
    }
}

/**
 * @brief       ATK-MD0350模块LCD画矩形框
 * @param       x1   : 待画矩形框端点1的X坐标
 *              y1   : 待画矩形框端点1的Y坐标
 *              x2   : 待画矩形框端点2的X坐标
 *              y2   : 待画矩形框端点2的Y坐标
 *              color: 待画矩形框的颜色
 * @retval      无
 */
void atk_md0350_draw_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    atk_md0350_draw_line(x1, y1, x2, y1, color);
    atk_md0350_draw_line(x1, y2, x2, y2, color);
    atk_md0350_draw_line(x1, y1, x1, y2, color);
    atk_md0350_draw_line(x2, y1, x2, y2, color);
}

/**
 * @brief       ATK-MD0350模块LCD画圆形框
 * @param       x    : 待画圆形框原点的X坐标
 *              y    : 待画圆形框原点的Y坐标
 *              r    : 待画圆形框的半径
 *              color: 待画圆形框的颜色
 * @retval      无
 */
void atk_md0350_draw_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
    int32_t x_t;
    int32_t y_t;
    int32_t error;
    int32_t error2;

    x_t = -r;
    y_t = 0;
    error = 2 - 2 * r;

    do {
        atk_md0350_draw_point(x - x_t, y + y_t, color);
        atk_md0350_draw_point(x + x_t, y + y_t, color);
        atk_md0350_draw_point(x + x_t, y - y_t, color);
        atk_md0350_draw_point(x - x_t, y - y_t, color);

        error2 = error;
        if (error2 <= y_t)
        {
            y_t++;
            error = error + (y_t * 2 + 1);
            if ((-x_t == y_t) && (error2 <= x_t))
            {
                error2 = 0;
            }
        }

        if (error2 > x_t)
        {
            x_t++;
            error = error + (x_t * 2 + 1);
        }
    } while (x_t <= 0);
}

/**
 * @brief       ATK-MD0350模块LCD显示1个字符
 * @param       x    : 待显示字符的X坐标
 *              y    : 待显示字符的Y坐标
 *              ch   : 待显示字符
 *              font : 待显示字符的字体
 *              color: 待显示字符的颜色
 * @retval      无
 */
void atk_md0350_show_char(uint16_t x, uint16_t y, char ch, atk_md0350_lcd_font_t font, uint16_t color)
{
    const uint8_t *ch_code;
    uint8_t ch_width;
    uint8_t ch_height;
    uint8_t ch_size;
    uint8_t ch_offset;
    uint8_t byte_index;
    uint8_t byte_code;
    uint8_t bit_index;
    uint8_t width_index = 0;
    uint8_t height_index = 0;

    ch_offset = ch - ' ';

    switch (font)
    {
#if (ATK_MD0350_FONT_12 != 0)
        case ATK_MD0350_LCD_FONT_12:
        {
            ch_code = atk_md0350_font_1206[ch_offset];
            ch_width = ATK_MD0350_FONT_12_CHAR_WIDTH;
            ch_height = ATK_MD0350_FONT_12_CHAR_HEIGHT;
            ch_size = ATK_MD0350_FONT_12_CHAR_SIZE;
            break;
        }
#endif
#if (ATK_MD0350_FONT_16 != 0)
        case ATK_MD0350_LCD_FONT_16:
        {
            ch_code = atk_md0350_font_1608[ch_offset];
            ch_width = ATK_MD0350_FONT_16_CHAR_WIDTH;
            ch_height = ATK_MD0350_FONT_16_CHAR_HEIGHT;
            ch_size = ATK_MD0350_FONT_16_CHAR_SIZE;
            break;
        }
#endif
#if (ATK_MD0350_FONT_24 != 0)
        case ATK_MD0350_LCD_FONT_24:
        {
            ch_code = atk_md0350_font_2412[ch_offset];
            ch_width = ATK_MD0350_FONT_24_CHAR_WIDTH;
            ch_height = ATK_MD0350_FONT_24_CHAR_HEIGHT;
            ch_size = ATK_MD0350_FONT_24_CHAR_SIZE;
            break;
        }
#endif
#if (ATK_MD0350_FONT_32 != 0)
        case ATK_MD0350_LCD_FONT_32:
        {
            ch_code = atk_md0350_font_3216[ch_offset];
            ch_width = ATK_MD0350_FONT_32_CHAR_WIDTH;
            ch_height = ATK_MD0350_FONT_32_CHAR_HEIGHT;
            ch_size = ATK_MD0350_FONT_32_CHAR_SIZE;
            break;
        }
#endif
        default:
        {
            return;
        }
    }

    if ((x + ch_width > ATK_MD0350_LCD_HEIGHT) || (y + ch_height > ATK_MD0350_LCD_WIDTH))
    {
        return;
    }

    for (byte_index=0; byte_index<ch_size; byte_index++)
    {
        byte_code = ch_code[byte_index];
        for (bit_index=0; bit_index<8; bit_index++)
        {
            if ((byte_code & 0x80) != 0)
            {
                atk_md0350_draw_point(x + width_index, y + height_index, color);
            }
            height_index++;
            if (height_index == ch_height)
            {
                height_index = 0;
                width_index++;
                break;
            }
            byte_code <<= 1;
        }
    }
}

/**
 * @brief       ATK-MD0350模块LCD显示字符串
 * @note        会自动换行和换页
 * @param       x     : 待显示字符串的X坐标
 *              y     : 待显示字符串的Y坐标
 *              width : 待显示字符串的显示高度
 *              height: 待显示字符串的显示宽度
 *              str   : 待显示字符串
 *              font  : 待显示字符串的字体
 *              color : 待显示字符串的颜色
 * @retval      无
 */
void atk_md0350_show_string(uint16_t x, uint16_t y, uint16_t width, uint16_t height, char *str, atk_md0350_lcd_font_t font, uint16_t color)
{
    uint8_t ch_width;
    uint8_t ch_height;
    uint16_t x_raw;
    uint16_t y_raw;
    uint16_t x_limit;
    uint16_t y_limit;

    switch (font)
    {
#if (ATK_MD0350_FONT_12 != 0)
        case ATK_MD0350_LCD_FONT_12:
        {
            ch_width = ATK_MD0350_FONT_12_CHAR_WIDTH;
            ch_height = ATK_MD0350_FONT_12_CHAR_HEIGHT;
            break;
        }
#endif
#if (ATK_MD0350_FONT_16 != 0)
        case ATK_MD0350_LCD_FONT_16:
        {
            ch_width = ATK_MD0350_FONT_16_CHAR_WIDTH;
            ch_height = ATK_MD0350_FONT_16_CHAR_HEIGHT;
            break;
        }
#endif
#if (ATK_MD0350_FONT_24 != 0)
        case ATK_MD0350_LCD_FONT_24:
        {
            ch_width = ATK_MD0350_FONT_24_CHAR_WIDTH;
            ch_height = ATK_MD0350_FONT_24_CHAR_HEIGHT;
            break;
        }
#endif
#if (ATK_MD0350_FONT_32 != 0)
        case ATK_MD0350_LCD_FONT_32:
        {
            ch_width = ATK_MD0350_FONT_32_CHAR_WIDTH;
            ch_height = ATK_MD0350_FONT_32_CHAR_HEIGHT;
            break;
        }
#endif
        default:
        {
            return;
        }
    }

    x_raw = x;
    y_raw = y;
//    x_limit = ((x + width + 1) > ATK_MD0350_LCD_WIDTH) ? ATK_MD0350_LCD_WIDTH : (x + width + 1);
//    y_limit = ((y + height + 1) > ATK_MD0350_LCD_HEIGHT) ? ATK_MD0350_LCD_HEIGHT : (y + height + 1);

    x_limit = ((x + width + 1) > ATK_MD0350_LCD_HEIGHT) ? ATK_MD0350_LCD_HEIGHT : (x + width + 1);
    y_limit = ((y + height + 1) > ATK_MD0350_LCD_WIDTH) ? ATK_MD0350_LCD_WIDTH : (y + height + 1);

    while ((*str >= ' ') && (*str <= '~'))
    {
        if (x + ch_width >= x_limit)
        {
            x = x_raw;
            y += ch_height;
        }

        if (y + ch_height >= y_limit)
        {
            y = x_raw;
            x = y_raw;
        }

        atk_md0350_show_char(x, y, *str, font, color);

        x += ch_width;
        str++;
    }
}

/**
 * @brief       ATK-MD0350模块LCD显示数字，可控制显示高位0
 * @param       x    : 待显示数字的X坐标
 *              y    : 待显示数字的Y坐标
 *              num  : 待显示数字
 *              len  : 待显示数字的位数
 *              mode : ATK_MD0350_NUM_SHOW_NOZERO: 数字高位0不显示
 *                     ATK_MD0350_NUM_SHOW_ZERO  : 数字高位0显示
 *              font : 待显示数字的字体
 *              color: 待显示数字的颜色
 * @retval      无
 */
void atk_md0350_show_xnum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, atk_md0350_num_mode_t mode, atk_md0350_lcd_font_t font, uint16_t color)
{
    uint8_t ch_width;
    uint8_t len_index;
    uint8_t num_index;
    uint8_t first_nozero = 0;
    char pad;

    switch (font)
    {
#if (ATK_MD0350_FONT_12 != 0)
        case ATK_MD0350_LCD_FONT_12:
        {
            ch_width = ATK_MD0350_FONT_12_CHAR_WIDTH;
            break;
        }
#endif
#if (ATK_MD0350_FONT_16 != 0)
        case ATK_MD0350_LCD_FONT_16:
        {
            ch_width = ATK_MD0350_FONT_16_CHAR_WIDTH;
            break;
        }
#endif
#if (ATK_MD0350_FONT_24 != 0)
        case ATK_MD0350_LCD_FONT_24:
        {
            ch_width = ATK_MD0350_FONT_24_CHAR_WIDTH;
            break;
        }
#endif
#if (ATK_MD0350_FONT_32 != 0)
        case ATK_MD0350_LCD_FONT_32:
        {
            ch_width = ATK_MD0350_FONT_32_CHAR_WIDTH;
            break;
        }
#endif
        default:
        {
            return;
        }
    }

    switch (mode)
    {
        case ATK_MD0350_NUM_SHOW_NOZERO:
        {
            pad = ' ';
            break;
        }
        case ATK_MD0350_NUM_SHOW_ZERO:
        {
            pad = '0';
            break;
        }
        default:
        {
            return;
        }
    }

    for (len_index=0; len_index<len; len_index++)
    {
        num_index = (num / atk_md0350_pow(10, len - len_index - 1)) % 10;
        if ((first_nozero == 0) && (len_index < (len - 1)))
        {
            if (num_index == 0)
            {
                atk_md0350_show_char(x + ch_width * len_index, y, pad, font, color);
                continue;
            }
            else
            {
                first_nozero = 1;
            }
        }

        atk_md0350_show_char(x + ch_width * len_index, y, num_index + '0', font, color);
    }
}

/**
 * @brief       ATK-MD0350模块LCD显示数字，不显示高位0
 * @param       x    : 待显示数字的X坐标
 *              y    : 待显示数字的Y坐标
 *              num  : 待显示数字
 *              len  : 待显示数字的位数
 *              font : 待显示数字的字体
 *              color: 待显示数字的颜色
 * @retval      无
 */
void atk_md0350_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, atk_md0350_lcd_font_t font, uint16_t color)
{
    atk_md0350_show_xnum(x, y, num, len, ATK_MD0350_NUM_SHOW_NOZERO, font, color);
}

/**
 * @brief       ATK-MD0350模块LCD图片
 * @note        图片取模方式: 水平扫描、RGB565、高位在前
 * @param       x     : 待显示图片的X坐标
 *              y     : 待显示图片的Y坐标
 *              width : 待显示图片的宽度
 *              height: 待显示图片的高度
 *              pic   : 待显示图片数组首地址
 * @retval      无
 */
void atk_md0350_show_pic(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t *pic)
{
    uint16_t x_index;
    uint16_t y_index;

    if ((x + width > ATK_MD0350_LCD_WIDTH) || (y + height > ATK_MD0350_LCD_HEIGHT))
    {
        return;
    }

    atk_md0350_set_column_address(x, x + width - 1);
    atk_md0350_set_page_address(y, y + height - 1);
    atk_md0350_start_write_memory();
    for (y_index=y; y_index<=(y + height); y_index++)
    {
        for (x_index=x; x_index<=(x + width); x_index++)
        {
            atk_md0350_fsmc_write_dat(*pic);
            pic++;
        }
    }
}