/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-06-20     Rick       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "radio.h"
#include "radio_app.h"
#include "radio_driver.h"
#include "radio_encoder.h"

#define DBG_TAG "RADIO_CSMA"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define CSMA_NOISE_THRESHOLD_IN_DBM (-75)
#define CSMA_WINDOW_TIME 5

static uint8_t cad_detected = 0;

void radio_cad_detected(uint8_t channelActivityDetected)
{
    cad_detected = channelActivityDetected;
    LOG_D("radio_cad_detected %d,tick %d",channelActivityDetected,rt_tick_get( ));
}

rt_err_t csma_check_start(uint32_t send_freq)
{
    rt_err_t ret = RT_EOK;

    cad_detected = 0;
    Radio.Standby();
    Radio.SetChannel( send_freq );
    SUBGRF_SetCadParams(LORA_CAD_02_SYMBOL, 22, 10, LORA_CAD_ONLY, 0);
    Radio.StartCad();
    rt_thread_mdelay(2 * CSMA_WINDOW_TIME);

    if(cad_detected)
    {
        LOG_E("CAD detected valid frame,exit csma");
        ret = RT_ERROR;
        return ret;
    }

    if(Radio.IsChannelFree(send_freq,200000,CSMA_NOISE_THRESHOLD_IN_DBM,CSMA_WINDOW_TIME) == 0)
    {
        LOG_E("RSSI detected too high,exit csma");
        ret = RT_ERROR;
        return ret;
    }

    return ret;
}
