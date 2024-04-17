/*
 * Copyright (c) 2024 Realtek Semiconductor 
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*============================================================================*
 *                              Includes
 *============================================================================*/
#include "zephyr/kernel.h"
#include "zephyr/device.h"
#include "zephyr/drivers/gpio.h"
#include "zephyr/logging/log.h"
#include <rtl_pinmux.h>

#include "board.h"
#include "led_driver.h"
LOG_MODULE_REGISTER(led, LOG_LEVEL_INF);

#if SUPPORT_LED_INDICATION_FEATURE

/*============================================================================*
 *                              Local Variables
 *============================================================================*/

/*============================================================================*
 *                              Functions Declaration
 *============================================================================*/
void led_pad_config(void);
void led_driver_enable_boost_power(void);
void led_driver_disable_boost_power(void);
void led_module_init(void);
bool led_driver_dlps_check(void);

/*============================================================================*
 *                              Public Funcitons
 *============================================================================*/
/**
 * @brief  Led module pad config
 * @param  None
 * @return None
 */
void led_pad_config(void)
{
#if LED_GPIO_CTL_NUM
    LOG_INF("led gpio pad config");
    led_gpio_ctl_pad_config();
#endif
#if LED_HW_TIM_PWM_CTL_NUM
    LOG_INF("led pwm pad config");
    led_hw_tim_pwm_ctl_pad_config();
#endif
}

/**
 * @brief  Enable LED boost power
 * @param  None
 * @return None
 */
void led_driver_enable_boost_power(void)
{
#ifdef LED_EN_PIN
    bool is_led_working = false;
#if LED_GPIO_CTL_NUM
    is_led_working |= led_gpio_ctl_is_working;
#endif
#if LED_HW_TIM_PWM_CTL_NUM
    is_led_working |= led_hw_tim_pwm_is_working;
#endif
    if (is_led_working == true)
    {
#if LED_DEBUG
        //APP_PRINT_INFO0("[Led] led_driver_enable_boost_power");
#endif
        Pad_Config(LED_EN_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    }
#endif
}

/**
 * @brief  Disable LED boost power
 * @param  None
 * @return None
 */
void led_driver_disable_boost_power(void)
{
#ifdef LED_EN_PIN
    bool is_led_working = false;
#if LED_GPIO_CTL_NUM
    is_led_working |= led_gpio_ctl_is_working;
#endif
#if LED_HW_TIM_PWM_CTL_NUM
    is_led_working |= led_hw_tim_pwm_is_working;
#endif
    if (is_led_working == false)
    {
#if LED_DEBUG
        //APP_PRINT_INFO0("[Led] led_driver_disable_boost_power");
#endif
        Pad_Config(LED_EN_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    }
#endif
}

/**
 * @brief  Initialize led module
 * @param  None
 * @return None
 */
void led_module_init(void)
{
    LOG_INF("led module timer init");
#if LED_GPIO_CTL_NUM
    led_gpio_ctl_module_init();
#endif
#if LED_HW_TIM_PWM_CTL_NUM
    led_hw_tim_pwm_ctl_init();
#endif
}

/**
 * @brief  Check if led module allow enter dlps
 * @param  None
 * @return Result - true: allow enter dlps, false: can not enter dlps
 */
bool led_driver_dlps_check(void)
{
    bool dlps_check_flag = true;
#if LED_GPIO_CTL_NUM
    dlps_check_flag &= led_gpio_ctl_driver_dlps_check();
#endif
#if LED_HW_TIM_PWM_CTL_NUM
    dlps_check_flag &= led_hw_tim_pwm_ctl_dlps_check();
#endif
    return dlps_check_flag;
}
#endif
