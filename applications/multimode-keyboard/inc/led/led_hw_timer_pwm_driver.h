/*
 * Copyright (c) 2024 dariel Zhang
 * Copyright (c) 2024 Realtek Semiconductor 
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _LED_HW_TIMER_PWM_DRIVER_H_
#define _LED_HW_TIMER_PWM_DRIVER_H_

#include "zephyr/kernel.h"
#include "zephyr/device.h"
#include "zephyr/drivers/gpio.h"
#include "rtl_tim.h"
#include "rtl_enh_tim.h"
#include "board.h"
/*============================================================================*
 *                              Macros
 *============================================================================*/
#define PWM_LED_DEBUG        0
#define LED_PWM_PERIOD       79360 /*504Hz*/

#define LED_GRAYSCALE        LED_PWM_PERIOD / 256  /* step 1/310 */

#define LED_CNT_TIM                 TIM6
#define LED_CNT_TIME_MS             2 /* 2ms */

#if DT_NODE_HAS_STATUS(DT_ALIAS(pwm1), okay)
#define PWM_DEV_NODE1 DT_ALIAS(pwm1)
#endif
#if DT_NODE_HAS_STATUS(DT_ALIAS(pwm2), okay)
#define PWM_DEV_NODE2 DT_ALIAS(pwm2)
#endif
#if DT_NODE_HAS_STATUS(DT_ALIAS(pwm3), okay)
#define PWM_DEV_NODE3 DT_ALIAS(pwm3)
#endif
#if DT_NODE_HAS_STATUS(DT_ALIAS(check_timer), okay)
#define CHECK_TIM_NODE DT_ALIAS(check_timer)
#endif
/*============================================================================*
*                              Types
*============================================================================*/
typedef   uint32_t      PWM_LED_TYPE;

typedef enum
{
    PWM_LED_TYPE_IDLE,
    PWM_LED_TYPE_BLINK_RESET,
    PWM_LED_TYPE_BLINK_TEST,
    PWM_LED_TYPE_ON,
    PWM_LED_TYPE_MAX
} PWM_LED_TYPE_ENUM;

/*led loop time(ms) for each event*/
#define   PWM_LED_LOOP_NUM_IDLE             0
#define   PWM_LED_LOOP_NUM_500_TIMES        500

typedef uint8_t *(*LED_EVENT_HANDLE_FUNC)(void);

typedef struct
{
    uint8_t pin_num;
    uint8_t pin_func;
    TIM_TypeDef *timer_type;
    ENHTIM_TypeDef *entimer_type;
} T_LED_PWM_PARAM_DEF;

typedef struct
{
    uint8_t   led_type_index;
    uint32_t  led_loop_num;
    LED_EVENT_HANDLE_FUNC led_event_handle_func;
} T_PWM_LED_EVENT_STG;

/*struct support for led blink count*/
typedef struct
{
    uint8_t led_event_loop_cnt;
    float cur_tick_cnt_offset;
} T_PWM_LED_CNT_STG;

/*support for each led data*/
typedef struct
{
    uint8_t led_index;
    uint8_t led_current_type;
    T_PWM_LED_CNT_STG led_cnt_arr[PWM_LED_TYPE_MAX];
    uint32_t led_map;
} T_PWM_LED_DATA_STG;

/*============================================================================*
*                        Export Global Variables
*============================================================================*/
extern bool led_hw_tim_pwm_is_working;

/*============================================================================*
 *                         Functions
 *============================================================================*/
void led_hw_tim_pwm_ctl_pad_config(void);
void led_hw_tim_pwm_ctl_init(void);
void led_hw_tim_pwm_output(uint8_t index, uint8_t grayscale);
void led_hw_tim_pwm_turn_on(uint8_t led_index, const uint8_t *p_rgb_grayscale);
void led_hw_tim_pwm_turn_off(uint8_t led_index);
void led_hw_tim_pwm_blink_start(uint16_t led_index, PWM_LED_TYPE type, uint32_t cnt);
void led_hw_tim_pwm_blink_exit(uint16_t led_index, PWM_LED_TYPE type);

#define PWM_LED_BLINK(led_index, type, cnt) led_hw_tim_pwm_blink_start(led_index, type, cnt)
#define PWM_LED_BLINK_EXIT(led_index, type) led_hw_tim_pwm_blink_exit(led_index, type)

#endif