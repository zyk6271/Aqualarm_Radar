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
#include "board.h"

#define DBG_TAG "adc"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

ADC_HandleTypeDef adc_handle;
rt_thread_t adc_thread = RT_NULL;

uint8_t radar_delay_time_level = 0;
uint8_t radar_detect_range_level = 0;
uint32_t adc_value_temp[2] = {0};

void adc_hw_init(void)
{
    /* DMA controller clock enable */
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA1_Channel1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
    /* DMA1_Channel2_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
    HAL_NVIC_DisableIRQ(DMA1_Channel2_IRQn);
    /* USER CODE BEGIN ADC_Init 0 */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /* USER CODE END ADC_Init 0 */

    ADC_ChannelConfTypeDef sConfig = {0};

    /* USER CODE BEGIN ADC_Init 1 */

    /* USER CODE END ADC_Init 1 */

    /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
    */
    adc_handle.Instance = ADC;
    adc_handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    adc_handle.Init.Resolution = ADC_RESOLUTION_12B;
    adc_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    adc_handle.Init.ScanConvMode = ADC_SCAN_ENABLE;
    adc_handle.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    adc_handle.Init.LowPowerAutoWait = DISABLE;
    adc_handle.Init.LowPowerAutoPowerOff = DISABLE;
    adc_handle.Init.ContinuousConvMode = ENABLE;
    adc_handle.Init.NbrOfConversion = 2;
    adc_handle.Init.DiscontinuousConvMode = DISABLE;
    adc_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc_handle.Init.DMAContinuousRequests = ENABLE;
    adc_handle.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    adc_handle.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_79CYCLES_5;
    adc_handle.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_79CYCLES_5;
    adc_handle.Init.OversamplingMode = DISABLE;
    adc_handle.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
    if (HAL_ADC_Init(&adc_handle) != HAL_OK)
    {
      Error_Handler();
    }

    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_2;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
    if (HAL_ADC_ConfigChannel(&adc_handle, &sConfig) != HAL_OK)
    {
      Error_Handler();
    }

    /** Configure Regular Channel
    */
    sConfig.Channel = ADC_CHANNEL_3;
    sConfig.Rank = ADC_REGULAR_RANK_2;
    if (HAL_ADC_ConfigChannel(&adc_handle, &sConfig) != HAL_OK)
    {
      Error_Handler();
    }
}

uint8_t radar_delay_time_level_read(void)
{
    return radar_delay_time_level;
}

uint8_t radar_detect_range_level_read(void)
{
    return radar_detect_range_level;
}

static void adc_thread_entry(void *parameter)
{
    static uint8_t radar_delay_time_level_temp = 0;
    static uint8_t radar_detect_range_level_temp = 0;
    rt_thread_mdelay(1000);
    while(1)
    {
        radar_delay_time_level_temp = (uint8_t)(adc_value_temp[0] / 274);
        if(radar_delay_time_level_temp != radar_delay_time_level)
        {
            radar_delay_time_level = radar_delay_time_level_temp;
            RF_HeartWithMain();
        }
        radar_detect_range_level_temp = (uint8_t)(adc_value_temp[1] / 273);
        if(radar_detect_range_level_temp != radar_detect_range_level)
        {
            radar_detect_range_level = radar_detect_range_level_temp;
            radar_config_distance_set(radar_detect_range_level);
            RF_HeartWithMain();
        }
        LOG_D("radar_delay_time_level:%d radar_detect_range_level:%d",radar_delay_time_level,radar_detect_range_level);
        rt_thread_mdelay(1000);
    }
}

void adc_init(void)
{
    adc_hw_init();
    HAL_ADC_Start_DMA(&adc_handle, &adc_value_temp, 2);
    adc_thread = rt_thread_create("adc_thread",adc_thread_entry,RT_NULL,1024,10,10);
    rt_thread_startup(adc_thread);
}
