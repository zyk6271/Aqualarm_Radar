/*
 * Copyright (c) 2006-2024, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-09-13     RT-Thread    first version
 */

#include <rtthread.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define MCU_VER     "1.0.3"

int main(void)
{
    LOG_I("System Version is %s\r\n",MCU_VER);
    led_init();
    button_init();
    radar_init();
    radio_init();
    adc_init();
    storage_init();
    while (1)
    {
        rt_thread_mdelay(1000);
    }

    return RT_EOK;
}
