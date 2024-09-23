/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-03     Rick       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "signal_led.h"

rt_thread_t led_thread = RT_NULL;
led_mem_opreation_t led_mem_opreation;

static led_t *led_obj_learn = RT_NULL;
static led_t *led_obj_sys_run = RT_NULL;
static led_t *led_obj_detected = RT_NULL;

static void off_red_on(void *param)
{
    ws2812b_red(0,1);
}

static void off_red_off(void *param)
{
    ws2812b_red(0,0);
}

static void on_green_on(void *param)
{
    ws2812b_green(0,1);
}

static void on_green_off(void *param)
{
    ws2812b_green(0,0);
}

void led_work_sys_run(void)
{
    led_stop(led_obj_detected);
    led_start(led_obj_sys_run);
}

void led_work_human_detected(void)
{
    led_stop(led_obj_sys_run);
    led_start(led_obj_detected);
}

void led_learn_succcess(void)
{
    led_start(led_obj_learn);
}

static void led_run(void *parameter)
{
    ws2812b_init();
    while(1)
    {
        rt_thread_mdelay(LED_TICK_TIME);
        led_ticks();
        RGB_SendArray();
    }
}

void led_init(void)
{
    led_mem_opreation.malloc_fn = (void* (*)(size_t))rt_malloc;
    led_mem_opreation.free_fn = rt_free;
    led_set_mem_operation(&led_mem_opreation);

    led_obj_sys_run = led_create(on_green_on, on_green_off, NULL);
    led_set_mode(led_obj_sys_run, LOOP_PERMANENT, "50,10000,");

    led_obj_detected = led_create(on_green_on, on_green_off, NULL);
    led_set_mode(led_obj_detected, LOOP_PERMANENT, "1,0,");

    led_obj_learn = led_create(off_red_on, off_red_off, NULL);
    led_set_mode(led_obj_learn, 5, "200,200,");

    led_thread = rt_thread_create("signal_led",led_run,RT_NULL,512,RT_THREAD_PRIORITY_MAX/2,100);
    rt_thread_startup(led_thread);
}
