/*
 * Copyright (c) 2024 Realtek Semiconductor 
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "zephyr/kernel.h"
#include "zephyr/device.h"
#include "zephyr/drivers/gpio.h"
#include "zephyr/logging/log.h"
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/counter.h>

#include "led_gpio_ctrl_driver.h"
#include "board.h"
#include "led_driver.h"
#include "rtl_tim.h"
#include "rtl_enh_tim.h"
#include "rtl_pinmux.h"
#include "trace.h"
LOG_MODULE_DECLARE(led, LOG_LEVEL_INF);

/*============================================================================*
 *                              Functions Declaration
 *============================================================================*/
static uint8_t *led_reset_event_handler(void);
static uint8_t *led_test_event_handler(void);
static void led_hw_cnt_timer_init(uint32_t time_ms);
static void led_hw_cnt_timer_stop(void);
static uint8_t led_hw_tim_update_pwm_states(uint8_t index, FunctionalState state);
static void led_hw_tim_pwm_start(uint8_t index, uint32_t pwm_high_cnt,
                                 uint32_t pwm_low_cnt);
static void led_hw_tim_pwm_stop(uint8_t index);
static void led_hw_tim_tick_handle(void);
static uint32_t led_hw_tim_pwm_get_next_event(uint32_t bitmap);
void led_cnt_hw_timer_handler(const struct device *dev, void *user_data);
void led_hw_tim_pwm_ctl_pad_config(void);
void led_hw_tim_pwm_ctl_init(void);
bool led_hw_tim_pwm_ctl_dlps_check(void);
void led_hw_tim_pwm_output(uint8_t index, uint8_t grayscale);
void led_hw_tim_pwm_turn_on(uint8_t led_index, const uint8_t *p_rgb_grayscale);
void led_hw_tim_pwm_turn_off(uint8_t led_index);
void led_hw_tim_pwm_blink_start(uint16_t led_index, PWM_LED_TYPE type, uint32_t cnt);
void led_hw_tim_pwm_blink_exit(uint16_t led_index, PWM_LED_TYPE type);

/*============================================================================*
*                              Local Variables
*============================================================================*/
static bool led_hw_cnt_timer_is_working = false;
static uint8_t led_pwm_init_states = 0;
static uint16_t led_tick_cnt = 0;
static uint8_t led_last_rgb_grayscale[3] = {0};
static uint8_t led_rgb_grayscale_red[3] = {255, 0, 0};
static uint8_t led_rgb_grayscale_green[3] = {0, 255, 0};
static uint8_t led_rgb_grayscale_blue[3] = {0, 0, 255};
static uint8_t led_rgb_grayscale_white[3] = {255, 255, 255};
static T_PWM_LED_DATA_STG pwm_led_arr = {0};

static const T_LED_PWM_PARAM_DEF led_pwm_list[3] =
{
    {LED0_R_PIN, 0, NULL, NULL},
    {LED0_G_PIN, 0, NULL, NULL},
    {LED0_B_PIN, 0, NULL, NULL},
};
static T_PWM_LED_EVENT_STG pwm_led_event_arr[PWM_LED_TYPE_MAX] =
{
    {PWM_LED_TYPE_IDLE,        PWM_LED_LOOP_NUM_IDLE,      NULL},
    {PWM_LED_TYPE_BLINK_RESET, PWM_LED_LOOP_NUM_500_TIMES, led_reset_event_handler},
    {PWM_LED_TYPE_BLINK_TEST,  PWM_LED_LOOP_NUM_500_TIMES, led_test_event_handler},
    {PWM_LED_TYPE_ON,          PWM_LED_LOOP_NUM_500_TIMES, NULL},
};
const struct device *pwm_dev1 = DEVICE_DT_GET(PWM_DEV_NODE1);
const struct device *pwm_dev2 = DEVICE_DT_GET(PWM_DEV_NODE2);
const struct device *pwm_dev3 = DEVICE_DT_GET(PWM_DEV_NODE3);

const struct device *check_tim_dev = DEVICE_DT_GET(CHECK_TIM_NODE);
/*============================================================================*
 *                              Global Variables
 *============================================================================*/
bool led_hw_tim_pwm_is_working = false;

/*============================================================================*
 *                              Private Funcitons
 *============================================================================*/
/**
 * @brief  Handler of led reset event
 * @param  None
 * @return Pointer to rgb grayscale buffer
 */
static uint8_t *led_reset_event_handler(void)
{
    return led_rgb_grayscale_white;
}
/**
 * @brief  Handler of led test event
 * @param  None
 * @return Pointer to rgb grayscale buffer
 */
static uint8_t *led_test_event_handler(void)
{
    static uint8_t led_test_rgb_grayscale[3] = {0};
    static uint8_t test_speed = 5;
    uint16_t streamer_cnt = (led_tick_cnt / test_speed) % 765;
    if (streamer_cnt <= 255)
    {
        led_test_rgb_grayscale[0] = 255 - streamer_cnt;
        led_test_rgb_grayscale[1] = streamer_cnt;
        led_test_rgb_grayscale[2] = 0;
    }
    else if (streamer_cnt <= 510)
    {
        led_test_rgb_grayscale[0] = 0;
        led_test_rgb_grayscale[1] = 510 - streamer_cnt;
        led_test_rgb_grayscale[2] = streamer_cnt - 255;
    }
    else
    {
        led_test_rgb_grayscale[0] = streamer_cnt - 510;
        led_test_rgb_grayscale[1] = 0;
        led_test_rgb_grayscale[2] = 765 - streamer_cnt;
    }
    return led_test_rgb_grayscale;
}
/**
 * @brief  Led count control timer handler, will be called periodically to check if pwm led status need change
 * @param  None
 * @return None
 */
void led_cnt_hw_timer_handler(const struct device *dev, void *user_data)
{
    counter_stop(check_tim_dev);
    DBG_DIRECT("led_cnt_hw_timer_handler");
    // TIM_ClearINT(LED_CNT_TIM);
    // TIM_INTConfig(LED_CNT_TIM, DISABLE);

    uint32_t type = 0;

    /* get highest prio event type */
    type = led_hw_tim_pwm_get_next_event(pwm_led_arr.led_map);
    if (type != pwm_led_arr.led_current_type)
    {
        pwm_led_arr.led_current_type = type;
    }

    if (pwm_led_arr.led_map != 0)
    {
        uint8_t *p_led_rgb_grayscale = pwm_led_event_arr[type].led_event_handle_func();
        led_hw_tim_pwm_turn_on(LED_HW_TIM_PWM_CTL_INDEX, p_led_rgb_grayscale);
    }

    /* led tick msg handle, update left events state */
    led_hw_tim_tick_handle();

    /**update led blink tick*/
    led_tick_cnt ++;

    if (pwm_led_arr.led_map != 0)
    {
        //TIM_INTConfig(LED_CNT_TIM, ENABLE);
    }
    return;
}
/**
 * @brief  Initialize led count control timer
 * @param  time_ms - period (uint ms).
 * @return None
 */
static void led_hw_cnt_timer_init(uint32_t time_ms)
{
    int ret = 0;
    counter_start(check_tim_dev);
    struct counter_top_cfg top_cfg = {
		.callback = led_cnt_hw_timer_handler,
		.user_data = NULL,
		.flags = 0,
	};

    top_cfg.ticks = counter_us_to_ticks(check_tim_dev,time_ms*1000);

    led_hw_cnt_timer_is_working = false;

    ret = counter_set_top_value(check_tim_dev,&top_cfg);
}
/**
 * @brief  Stop led count control timer
 * @param  None
 * @return None
 */
static void led_hw_cnt_timer_stop(void)
{
    if (led_hw_cnt_timer_is_working == true)
    {
        counter_stop(check_tim_dev);
        // TIM_Cmd(LED_CNT_TIM, DISABLE);
        // TIM_ClearINT(LED_CNT_TIM);
        led_hw_cnt_timer_is_working = false;
    }
}
/**
 * @brief  Upate led hw timer pwm states
 * @param  index - index of led_pwm_list
 * @param  state - ENABLE or DISABLE
 * @return Led pwm init states - one bit will be set to 1 if corresponding led hw pwm is started
 */
static uint8_t led_hw_tim_update_pwm_states(uint8_t index, FunctionalState state)
{
    if (state == DISABLE)
    {
        led_pwm_init_states &= ~(1 << index);
    }
    else
    {
        led_pwm_init_states |= (1 << index);
    }

    return led_pwm_init_states;
}

/**
 * @brief  Start led hw timer pwm
 * @param  index - index of led_pwm_list
 * @param  pwm_high_cnt - pwm high count
 * @param  pwm_low_cnt - pwm low count
 * @return None
 */
static void led_hw_tim_pwm_start(uint8_t index, uint32_t pwm_high_cnt, uint32_t pwm_low_cnt)
{
#if PWM_LED_DEBUG
    //APP_PRINT_INFO3("[led_module_pwm_start] index = %d, pwm_high_cnt = 0x%X, pwm_low_cnt = 0x%X",
    //               index, pwm_high_cnt, pwm_low_cnt);
#endif
    if (pwm_low_cnt == 0)
    {
        Pad_Config(led_pwm_list[index].pin_num, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
                   PAD_OUT_HIGH);
        led_hw_tim_update_pwm_states(index, DISABLE);
    }
    else if (pwm_high_cnt == 0)
    {
        Pad_Config(led_pwm_list[index].pin_num, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
                   PAD_OUT_LOW);
        led_hw_tim_update_pwm_states(index, DISABLE);
    }
    else
    {
        if(index == 0)
            pwm_set_cycles(pwm_dev1, 0, pwm_high_cnt+pwm_low_cnt, pwm_low_cnt,0);
        else if(index == 1)
            pwm_set_cycles(pwm_dev2, 0, pwm_high_cnt+pwm_low_cnt, pwm_low_cnt,0);
        else if(index == 2)
            pwm_set_cycles(pwm_dev3, 0, pwm_high_cnt+pwm_low_cnt, pwm_low_cnt,0);
    }
    led_hw_tim_update_pwm_states(index, ENABLE);
}
/**
 * @brief  Stop led hw timer pwm
 * @param  index - index of led_pwm_list
 * @return None
 */
static void led_hw_tim_pwm_stop(uint8_t index)
{
#if PWM_LED_DEBUG
    //APP_PRINT_INFO1("[led_module_pwm_stop] index %d", index);
#endif
    if(index == 0)
        pwm_set_cycles(pwm_dev1,0, 0, 0,0);
    else if(index == 1)
        pwm_set_cycles(pwm_dev2,0, 0, 0,0);   
    else if(index == 2)
        pwm_set_cycles(pwm_dev3,0, 0, 0,0);       
#if (LED_ON_LEVEL_TRIG == LED_ON_LEVEL_LOW)
    Pad_Config(led_pwm_list[index].pin_num, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
#else
    Pad_Config(led_pwm_list[index].pin_num, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_LOW);
#endif
    led_hw_tim_update_pwm_states(index, DISABLE);
    led_last_rgb_grayscale[index] = 0;
}
/**
 * @brief  Handle tick message for each led event.
 * @param  None
 * @return None
 */
static void led_hw_tim_tick_handle(void)
{
    if (0 == pwm_led_arr.led_map)
    {
        return;
    }

    for (uint8_t event_index = 0; event_index < PWM_LED_TYPE_MAX; event_index++)
    {
        /* update led state, when new tick come */
        uint32_t mask = (1 << event_index);

        if (0 == (mask & pwm_led_arr.led_map))
        {
            continue;
        }

        uint16_t led_cur_tick_cnt = led_tick_cnt +
                                    pwm_led_arr.led_cnt_arr[event_index].cur_tick_cnt_offset;
#if PWM_LED_DEBUG
        //APP_PRINT_INFO4("[Led] led_tick_cnt = %d, led_cur_tick_cnt = %d, led_loop_num = %d, led_event_loop_cnt = %d",
        //                led_tick_cnt, led_cur_tick_cnt, pwm_led_event_arr[event_index].led_loop_num,
        //                pwm_led_arr.led_cnt_arr[event_index].led_event_loop_cnt);
#endif
        if ((pwm_led_arr.led_cnt_arr[event_index].led_event_loop_cnt > 0)
            && (led_tick_cnt != 0)
            && ((led_cur_tick_cnt % pwm_led_event_arr[event_index].led_loop_num) == 0))
        {
            pwm_led_arr.led_cnt_arr[event_index].led_event_loop_cnt --;
            if (0 == pwm_led_arr.led_cnt_arr[event_index].led_event_loop_cnt)
            {
                /* clear event bit */
                pwm_led_arr.led_map &= ~(1 << event_index);
                if (pwm_led_arr.led_map == 0)
                {
                    led_hw_tim_pwm_turn_off(LED_HW_TIM_PWM_CTL_INDEX);
                    //TIM_Cmd(LED_CNT_TIM, DISABLE);
                    led_tick_cnt = 0;
                    led_hw_cnt_timer_is_working = false;
                    led_hw_tim_pwm_is_working = false;
                    led_driver_disable_boost_power();
                }
            }
        }
    }
}
/**
 * @brief  Get next led blink event which priority is highest
 * @param  bitmap - led event bit map, indicate which led event bit map to get
 * @return Led next event index
 */
static uint32_t led_hw_tim_pwm_get_next_event(uint32_t bitmap)
{
    uint32_t next_event = PWM_LED_TYPE_IDLE;
    for (uint32_t event_index = 0; event_index < PWM_LED_TYPE_MAX; event_index++)
    {
        uint32_t bit_mask = 0;
        bit_mask |= (1 << event_index);
        if (bit_mask & bitmap)
        {
            next_event = event_index;
            break;
        }
    }
#if PWM_LED_DEBUG
    //APP_PRINT_INFO1("[Led] next event type = %d", next_event);
#endif
    return next_event;
}
/**
 * @brief  Led hw timer pwm module pad config
 * @param  None
 * @return None
 */
void led_hw_tim_pwm_ctl_pad_config(void)
{
    for (uint8_t index = 0; index < 3; index++)
    {
#if (LED_ON_LEVEL_TRIG == LED_ON_LEVEL_LOW)
        Pad_Config(led_pwm_list[index].pin_num, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
                   PAD_OUT_HIGH);
#else
        Pad_Config(led_pwm_list[index].pin_num, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
                   PAD_OUT_LOW);
#endif
    }
}
/**
 * @brief  Initialize led hw timer pwm module
 * @param  None
 * @return None
 */
void led_hw_tim_pwm_ctl_init(void)
{
    DBG_DIRECT("led pwm timer config");
    led_pwm_init_states = 0;

    for (uint8_t index = 0; index < 3; index++)
    {
        led_hw_tim_pwm_stop(index);
    }
    led_hw_cnt_timer_init(LED_CNT_TIME_MS);
}
/**
 * @brief  Check if led hw timer pwm allow enter dlps
 * @param  None
 * @return Result - true: allow enter dlps, false: can not enter dlps
 */
bool led_hw_tim_pwm_ctl_dlps_check(void)
{
    return ((led_pwm_init_states == 0) && (led_hw_cnt_timer_is_working == false));
}
/**
 * @brief  Output hardware timer pwm for single led
 * @param  index - hw timer index
 * @param  grayscale - led rgb grayscale
 * @return None
 */
void led_hw_tim_pwm_output(uint8_t index, uint8_t grayscale)
{
    if (grayscale == led_last_rgb_grayscale[index])
    {
        return;
    }
    uint32_t pwm_high_cnt = 0;
    uint32_t pwm_low_cnt = 0;
#if (LED_ON_LEVEL_TRIG == LED_ON_LEVEL_LOW)
    if (grayscale == 0)
    {
        pwm_low_cnt = 0;
        pwm_high_cnt = LED_PWM_PERIOD - 1;
    }
    else
    {
        pwm_low_cnt = (grayscale + 1) * LED_GRAYSCALE - 1;
        pwm_high_cnt = LED_PWM_PERIOD - pwm_low_cnt - 1;
    }
#else
    if (grayscale == 0)
    {
        pwm_high_cnt = 0;
        pwm_low_cnt = LED_PWM_PERIOD - 1;
    }
    else
    {
        pwm_high_cnt = (grayscale + 1) * LED_GRAYSCALE - 1;
        pwm_low_cnt = LED_PWM_PERIOD - pwm_high_cnt - 1;
    }
#endif
    led_hw_tim_pwm_start(index, pwm_high_cnt, pwm_low_cnt);
    led_last_rgb_grayscale[index] = grayscale;
}
/**
 * @brief  Turn on single led
 * @param  led_index - led index
 * @param  grayscale - led rgb grayscale
 * @return None
 */
void led_hw_tim_pwm_turn_on(uint8_t led_index, const uint8_t *p_rgb_grayscale)
{
    if (led_index != LED_HW_TIM_PWM_CTL_INDEX)
    {
#if PWM_LED_DEBUG
        //APP_PRINT_INFO1("[led_hw_tim_pwm_turn_on] error led index %d", led_index);
#endif
        return;
    }
    if (led_hw_tim_pwm_is_working == false)
    {
        led_hw_tim_pwm_is_working = true;
        led_driver_enable_boost_power();
    }
    for (uint8_t index = 0; index < 3; index ++)
    {
        led_hw_tim_pwm_output(index, p_rgb_grayscale[index]);
    }
}
/**
 * @brief  Turn off single led
 * @param  led_index - led index
 * @return None
 */
void led_hw_tim_pwm_turn_off(uint8_t led_index)
{
    if (led_index != LED_HW_TIM_PWM_CTL_INDEX)
    {
#if PWM_LED_DEBUG
        //APP_PRINT_INFO1("[led_hw_tim_pwm_turn_off] error led index %d", led_index);
#endif
        return;
    }
    for (uint8_t index = 0; index < 3; index ++)
    {
        led_hw_tim_pwm_stop(index);
    }
}
/**
 * @brief  Start led blink
 * @param  index - led index to set
 * @param  type - led type to set
 * @param  cnt - led blink loop num
 * @return None
 */
void led_hw_tim_pwm_blink_start(uint16_t led_index, PWM_LED_TYPE type, uint32_t cnt)
{
#if PWM_LED_DEBUG
    //APP_PRINT_INFO3("[Led] led_blink_start: index %d, type %d, loop_num %d", led_index, type, cnt);
#endif
    if (led_index != LED_HW_TIM_PWM_CTL_INDEX)
    {
        return;
    }
    if ((PWM_LED_TYPE_IDLE == type) || (PWM_LED_TYPE_MAX <= type))
    {
        return;
    }
    /* set led event map flag */
    pwm_led_arr.led_map |= (1 << type);

    if (cnt > 0)
    {
        pwm_led_arr.led_cnt_arr[type].led_event_loop_cnt = cnt;
    }
    else
    {
        pwm_led_arr.led_cnt_arr[type].led_event_loop_cnt = 0;
    }
    pwm_led_arr.led_cnt_arr[type].cur_tick_cnt_offset = 0 - led_tick_cnt;

    led_hw_cnt_timer_is_working = true;
}

/**
 * @brief  Terminate led blink
 * @param  led_index - led index to set
 * @param  type - led type to set
 * @return None
 */
void led_hw_tim_pwm_blink_exit(uint16_t led_index, PWM_LED_TYPE type)
{
    if (led_index != LED_HW_TIM_PWM_CTL_INDEX)
    {
        return;
    }
    if ((PWM_LED_TYPE_IDLE == type) || (PWM_LED_TYPE_MAX <= type))
    {
        return;
    }

    /* start led */
    if (PWM_LED_TYPE_IDLE != (pwm_led_arr.led_map & (1 << type)))
    {
        /* clear current led type bit */
        pwm_led_arr.led_map &= ~(1 << type);
#if PWM_LED_DEBUG
        //APP_PRINT_INFO3("[Led] led_blink_exit, led_index = %d, type = %d, led_map = 0x%x", led_index, type,
        //                pwm_led_arr.led_map);
#endif
        /* check whether the led can be turned off */
        if (pwm_led_arr.led_map == 0)
        {
            led_hw_tim_pwm_turn_off(led_index);
            led_hw_cnt_timer_stop();
            led_tick_cnt = 0;
            led_hw_tim_pwm_is_working = false;
            led_driver_disable_boost_power();
        }
    }
}