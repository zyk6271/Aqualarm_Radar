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

#define DBG_TAG "radar"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static struct rt_semaphore rx_sem;
static rt_device_t serial;
static rt_thread_t radar_uart_thread = RT_NULL;
static rt_thread_t radar_service_thread = RT_NULL;
struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */

rt_timer_t radar_detect_timer = RT_NULL;

static uint8_t human_detect_flag = 0;

#define RADAR_UART_NAME                   "uart1"

static rt_err_t uart_rx_ind(rt_device_t dev, rt_size_t size)
{
    if (size > 0)
    {
        rt_sem_release(&rx_sem);
    }
    return RT_EOK;
}

static char uart_sample_get_char(void)
{
    char ch;

    while (rt_device_read(serial, 0, &ch, 1) == 0)
    {
        rt_sem_control(&rx_sem, RT_IPC_CMD_RESET, RT_NULL);
        rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
    }
    return ch;
}

void data_parsing(void)
{
    char ch;
    while (1)
    {
        ch = uart_sample_get_char();
        LOG_D("uart_sample_get_char %02X", ch);
    }
}

void radar_uart_init(void)
{
    char uart_name[RT_NAME_MAX];
    rt_strncpy(uart_name, RADAR_UART_NAME, RT_NAME_MAX);

    serial = rt_device_find(uart_name);
    if (!serial)
    {
        LOG_D("find %s failed!\n", uart_name);
    }

    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
    /* 串口参数配置 */
    config.baud_rate = BAUD_RATE_9600;        //修改波特率为 115200
    config.data_bits = DATA_BITS_8;           //数据位 8
    config.stop_bits = STOP_BITS_1;           //停止位 1
    config.parity    = PARITY_NONE;           //无奇偶校验位
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
    rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);
    rt_device_set_rx_indicate(serial, uart_rx_ind);

    radar_uart_thread = rt_thread_create("radar_uart", (void (*)(void *parameter))data_parsing, RT_NULL, 512, 7, 10);
    rt_thread_startup(radar_uart_thread);
}

void radar_uart_data_send(uint8_t *data,uint32_t len)
{
    rt_device_write(serial,0,data,len);
}

void radar_hw_init(void)
{
   rt_pin_mode(RADAR_OUT_PIN, PIN_MODE_INPUT);
   rt_pin_mode(PIR_OUT_PIN, PIN_MODE_INPUT);
}

void radar_config_init(void)
{
    rt_thread_mdelay(300);
    radar_config_distance_set(8);
    rt_thread_mdelay(50);
    radar_config_work_control(1);
}

uint8_t sensor_read_human_detected(void)
{
    return human_detect_flag;
}

uint8_t sensor_radar_level_read(void)
{
    return rt_pin_read(RADAR_OUT_PIN);
}

uint8_t sensor_pir_level_read(void)
{
    return rt_pin_read(PIR_OUT_PIN);
}

static void radar_service_thread_entry(void *parameter)
{
    static uint8_t radar_status,pir_status = 0;
    static uint32_t delay_time = 0;
    rt_thread_mdelay(2000);
    RF_HeartWithMain();
    while(1)
    {
        radar_status = sensor_radar_level_read();
        pir_status = sensor_pir_level_read();
        if(radar_status == 1 && pir_status == 1)
        {
            if(human_detect_flag == 0)
            {
                human_detect_flag = 1;
                led_work_human_detected();
                RF_Open_Valve();
            }
            delay_time = (5 * 60 * 1000) + (radar_delay_time_level_read() * 30 * 1000);
            rt_timer_stop(radar_detect_timer);
            rt_timer_control(radar_detect_timer, RT_TIMER_CTRL_SET_TIME, &delay_time);
            rt_timer_start(radar_detect_timer);
        }
        LOG_D("radar status:[%d],pir status:[%d],human_detect:[%d],total_time:[%d]",radar_status,pir_status,human_detect_flag,delay_time);
        rt_thread_mdelay(1000);
    }
}

void radar_detect_timer_callback(void *parameter)
{
    human_detect_flag = 0;
    led_work_sys_run();
    RF_Close_Valve();
}

void radar_init(void)
{
    radar_hw_init();
    radar_uart_init();
    led_work_sys_run();
    radar_detect_timer = rt_timer_create("radar_detect", radar_detect_timer_callback, RT_NULL, 5 * 1000, RT_TIMER_FLAG_SOFT_TIMER | RT_TIMER_FLAG_ONE_SHOT);
    radar_service_thread = rt_thread_create("radar_thread",radar_service_thread_entry,RT_NULL,1024,10,10);
    rt_thread_startup(radar_service_thread);
}
