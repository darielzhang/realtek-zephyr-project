/*
 * Copyright (c) 2024 Realtek Semiconductor 
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "zephyr/kernel.h"
#include "zephyr/device.h"
#include "zephyr/logging/log.h"
#include "usb_handle.h"
#include "board.h"
#include "usb_hid_interface_keyboard.h"
#include "trace.h"

LOG_MODULE_DECLARE(usb, LOG_LEVEL_INF);

/**
  * @brief keyboard_app_usb_send_keyboard_data
  * @param[in] T_KEYBOARD_DATA
  * @retval bool
  */
bool app_usb_send_keyboard_data(T_KEYBOARD_DATA keyboard_data)
{
    bool ret = false;
    const struct device *hid_keyboard_dev;
 
    hid_keyboard_dev = device_get_binding("RTK_HID_KEYBOARD_0");

    uint8_t sendbuf[KEYBOARD_DATA_SIZE+1] = {0};
    sendbuf[0] = KEYBOARD_REPORT_ID;
    memcpy(&sendbuf[1],keyboard_data.keyboard_common_data,KEYBOARD_DATA_SIZE);
    for(uint8_t i=0;i<KEYBOARD_DATA_SIZE+1;i++)
    {
        DBG_DIRECT("app usb send data:%x",((uint8_t *)&keyboard_data)[i]);
    }
    ret = hid_int_ep_write(hid_keyboard_dev, &sendbuf[0], sizeof(sendbuf), NULL)==0?true:false;
    // Pad_Config(P0_0, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE,
    //                PAD_OUT_LOW);
    

    return ret;
}
/**
  * @brief keyboard_app_usb_send_consumer_data
  * @param[in] T_KEYBOARD_DATA
  * @retval bool
  */
bool app_usb_send_consumer_data(T_KEYBOARD_DATA keyboard_data)
{
    bool ret = false;
    const struct device *hid_keyboard_dev;
 
    hid_keyboard_dev = device_get_binding("RTK_HID_KEYBOARD_0");

    uint8_t sendbuf[CONSUMER_DATA_SIZE+1]= {0};
    sendbuf[0] = CONSUMER_REPORT_ID;
    memcpy(&sendbuf[1],keyboard_data.keyboard_consumer_data,CONSUMER_DATA_SIZE);
    ret = hid_int_ep_write(hid_keyboard_dev, &sendbuf[0], sizeof(sendbuf), NULL)==0?true:false;

    return ret;
}
/**
  * @brief app_usb_send_vendor_data
  * @param[in] T_KEYBOARD_DATA
  * @retval bool
  */
bool app_usb_send_vendor_data(T_KEYBOARD_DATA keyboard_data)
{
    bool ret = false;
    const struct device *hid_keyboard_dev;
 
    hid_keyboard_dev = device_get_binding("RTK_HID_KEYBOARD_0");

    uint8_t sendbuf[VENDOR_DATA_SIZE+1] = {0};
    sendbuf[0] = VENDOR_REPORT_ID;
    memcpy(&sendbuf[1],keyboard_data.keyboard_vendor_data,VENDOR_DATA_SIZE);
    ret = hid_int_ep_write(hid_keyboard_dev, &sendbuf[0], sizeof(sendbuf), NULL)==0?true:false;

    return ret;
}