/*
 * Copyright (c) 2024 Realtek Semiconductor 
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _USB_HANDLE_H_
#define _USB_HANDLE_H_
#include "app_task.h"

/**
 * usb_handle.h
 *
 * \brief   usb send keyboard data
 *
 * \param[in]  pointer to data
 * \param[in]  data length
 *
 * \ingroup USB_HID
 */
bool app_usb_send_keyboard_data(T_KEYBOARD_DATA keyboard_data);

/**
 * usb_handle.h
 *
 * \brief   usb send consumer data
 *
 * \param[in]  pointer to data
 * \param[in]  data length
 *
 * \ingroup USB_HID
 */
bool app_usb_send_consumer_data(T_KEYBOARD_DATA keyboard_data);

/**
 * usb_handle.h
 *
 * \brief   usb send vendor data
 *
 * \param[in]  pointer to data
 * \param[in]  data length
 *
 * \ingroup USB_HID
 */
bool app_usb_send_vendor_data(T_KEYBOARD_DATA keyboard_data);

#endif