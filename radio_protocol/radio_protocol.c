/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2024-04-10     Rick       the first version
 */
#include "board.h"
#include "radio_protocol.h"

#define DBG_TAG "RADIO_PROTOCOL"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

uint8_t allow_add_device = 0;

void radio_protocol_parse(int rssi,int snr,uint8_t* data,size_t len)
{
    rx_format rx_frame;
    if(len < 14){
        // Log error: Size is too short
        return;
    }

    if (data[0] != FRAME_START) {
      // Log error: Frame start mismatch
      return;
    }

    if (data[1] >> 4 != NETID_TEST_ENV) {
      // Log error: Network ID mismatch
      return;
    }

    if (data[1] & 0x0F < NETWORK_VERSION) {
      // Log error: Network version too low
      return;
    }

    rx_frame.src_crc = data[len - 1];
    rx_frame.calc_crc = get_check_crc(data,len - 1);
    if(rx_frame.src_crc != rx_frame.calc_crc)
    {
        LOG_E("CRC is fail,src is 0x%02X,calc is 0x%02X\r\n",rx_frame.src_crc,rx_frame.calc_crc);
        return;
    }

    rx_frame.rssi = rssi;
    rx_frame.snr = snr;
    rx_frame.rx_data = &data[11];
    rx_frame.rx_len = len - 11;
    rx_frame.msg_type = data[2] & 0x07;
    rx_frame.device_type = (data[2] & 0x78) >> 3;
    rx_frame.dest_addr = (data[3] << 24) | (data[4] << 16) | (data[5] << 8) | data[6];
    rx_frame.source_addr = (data[7] << 24) | (data[8] << 16) | (data[9] << 8) | data[10];

    switch(rx_frame.device_type)
    {
    case DEVICE_TYPE_MAINUNIT:
    case DEVICE_TYPE_ALLINONE:
        radio_frame_mainunit_parse(&rx_frame);
        break;
    default:
        break;
    }
}
