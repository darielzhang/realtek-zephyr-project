/*
 * Copyright (c) 2024 dariel Zhang
 * Copyright (c) 2024 Realtek Semiconductor 
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _LED_DRIVER_H_
#define _LED_DRIVER_H_

#include "zephyr/kernel.h"
#include "zephyr/device.h"
#include "zephyr/drivers/gpio.h"
#include "led_gpio_ctrl_driver.h"
#include "led_hw_timer_pwm_driver.h"

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)
#define LED4_NODE DT_ALIAS(led4)
#define LED5_NODE DT_ALIAS(led5)
#define LED6_NODE DT_ALIAS(led6)
#define LED7_NODE DT_ALIAS(led7)


#define GPIO_SPEC(node_id) GPIO_DT_SPEC_GET_OR(node_id, gpios, {0})

#endif