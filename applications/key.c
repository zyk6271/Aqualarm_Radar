/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-12-27     Rick       the first version
 */
#include <rtthread.h>
#include <agile_button.h>
#include "pin_config.h"

#define DBG_TAG "key"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

uint8_t key_long_count = 0;
uint8_t key_long_click = 0;

extern enum Device_Status DeviceStatus;

agile_btn_t *key_btn = RT_NULL;

void key_long_hold_handle(void)
{
    if(key_long_count < 4)
    {
        key_long_count ++;
        LOG_D("key_long_count %d\r\n",key_long_count);
    }
    else
    {
        if(key_long_click == 0)
        {
            key_long_click = 1;
            RF_Learn_Request();
        }
    }
}

void key_click_handle(void)
{
    LOG_D("key_click_handle\r\n");
    RF_HeartWithMain();
}

void key_long_free_handle(void)
{
    key_long_count = 0;
    key_long_click = 0;
}

void button_init(void)
{
    key_btn = agile_btn_create(KEY_PIN, PIN_LOW, PIN_MODE_INPUT);
    agile_btn_set_hold_cycle_time(key_btn,1000);

    agile_btn_set_event_cb(key_btn, BTN_CLICK_EVENT, key_click_handle);
    agile_btn_set_event_cb(key_btn, BTN_HOLD_EVENT, key_long_hold_handle);
    agile_btn_set_event_cb(key_btn, BTN_HOLD_FREE_EVENT, key_long_free_handle);

    agile_btn_start(key_btn);
}
