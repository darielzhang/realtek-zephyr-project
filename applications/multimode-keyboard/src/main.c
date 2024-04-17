/*
 * Copyright (c) 2018, 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_hid.h>
#include "board.h"
#include "app_task.h"
#include "key_handle.h"
#include "usb_hid_interface_keyboard.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
	/* led init */
	led_pad_config();
	led_module_init();
	/* app global data init */
	app_global_data_init();
	/* usb init */
	usb_hid_keyboard_driver_init();
	/* keyscan init */
	key_handle_init_data();
	key_handle_init_config();
	key_handle_init_timer();

	/* app task init */
	os_zephyr_patch_init();
	app_task_init();
	
	k_yield();

	return 0;
}
/** @} */ /* End of group PERIPH_DEMO_MAIN */
