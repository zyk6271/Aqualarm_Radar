/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-13     Rick       the first version
 */
#include "rtthread.h"
#include "rtdevice.h"
#include "pin_config.h"
#include "board.h"

void radar_config_distance_set(uint8_t level)
{
    uint8_t dat[]={0x55,0x5a,0x01,0x00,0x00,0x00,0xfe};
    dat[4] = level;
    dat[5] = dat[2] ^ dat[3] ^ dat[4];
    radar_uart_data_send(dat,7);
}

void radar_config_work_control(uint8_t level)
{
    uint8_t dat[]={0x55,0x5a,0x09,0x00,0x00,0x00,0xfe};
    dat[4] = level;
    dat[5] = dat[2] ^ dat[3] ^ dat[4];
    radar_uart_data_send(dat,7);
}
