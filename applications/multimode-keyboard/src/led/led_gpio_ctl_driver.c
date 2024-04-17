/*
 * Copyright (c) 2024 Realtek Semiconductor 
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "zephyr/kernel.h"
#include "zephyr/device.h"
#include "zephyr/drivers/gpio.h"
#include "zephyr/logging/log.h"
#include <rtl_pinmux.h>
#include "led_gpio_ctrl_driver.h"

#include "board.h"
#include "led_driver.h"
#include "trace.h"
LOG_MODULE_DECLARE(led, LOG_LEVEL_INF);

/*============================================================================*
 *                              Local Variables
 *============================================================================*/
static uint16_t led_tick_cnt = 0;
static T_LED_DATA_STG led_arr[LED_GPIO_CTL_PWM_NUM] = {0};
static struct k_timer led_gpio_ctrl_timer;
static T_LED_PIN led_pin_pwm[LED_GPIO_CTL_PWM_NUM] =  {0};
static uint8_t led_pin_gpio[LED_GPIO_CTL_NUM] = {0};

static T_LED_EVENT_STG led_event_arr[LED_TYPE_MAX] =
{
    {LED_TYPE_IDLE,                   LED_LOOP_BITS_IDLE,     LED_COLOR_IDLE_MASK,   LED_BIT_MAP_INVALID},
    {LED_TYPE_BLINK_BLE_PAIR_SUCCESS, LED_LOOP_BITS_20_TIMES, LED_COLOR_BLUE_MASK,   LED_BIT_MAP_ALWAYS_ON},
    {LED_TYPE_BLINK_BLE_PAIR_ADV,     LED_LOOP_BITS_20_TIMES, LED_COLOR_BLUE_MASK,   LED_BIT_MAP_BLE_PAIR_ADV},
    {LED_TYPE_BLINK_PPT_PAIR_SUCCESS, LED_LOOP_BITS_20_TIMES, LED_COLOR_GREEN_MASK,  LED_BIT_MAP_ALWAYS_ON},
    {LED_TYPE_BLINK_PPT_PAIR_ADV,     LED_LOOP_BITS_20_TIMES, LED_COLOR_GREEN_MASK,  LED_BIT_MAP_PPT_PAIR_ADV},
    {LED_TYPE_BLINK_RECONNECT_ADV,    LED_LOOP_BITS_20_TIMES, LED_COLOR_CYAN_MASK,   LED_BIT_MAP_RECONNECT_ADV},
    {LED_TYPE_BLINK_REPORT_RATE,      LED_LOOP_BITS_20_TIMES, LED_COLOR_CYAN_MASK,   LED_BIT_MAP_ALWAYS_ON},
    {LED_TYPE_BLINK_DPI_1,            LED_LOOP_BITS_20_TIMES, LED_COLOR_RED_MASK,    LED_BIT_MAP_ALWAYS_ON},
    {LED_TYPE_BLINK_DPI_2,            LED_LOOP_BITS_20_TIMES, LED_COLOR_BLUE_MASK,   LED_BIT_MAP_ALWAYS_ON},
    {LED_TYPE_BLINK_DPI_3,            LED_LOOP_BITS_20_TIMES, LED_COLOR_GREEN_MASK,  LED_BIT_MAP_ALWAYS_ON},
    {LED_TYPE_BLINK_DPI_4,            LED_LOOP_BITS_20_TIMES, LED_COLOR_PURPLR_MASK, LED_BIT_MAP_ALWAYS_ON},
    {LED_TYPE_BLINK_DPI_5,            LED_LOOP_BITS_20_TIMES, LED_COLOR_YELLOW_MASK, LED_BIT_MAP_ALWAYS_ON},
    {LED_TYPE_BLINK_RESET,            LED_LOOP_BITS_20_TIMES, LED_COLOR_WHITE_MASK,  LED_BIT_MAP_ALWAYS_ON},
    {LED_TYPE_BLINK_CHARGING,         LED_LOOP_BITS_20_TIMES, LED_COLOR_RED_MASK,    LED_BIT_MAP_ALWAYS_ON},
    {LED_TYPE_BLINK_LOW_POWER,        LED_LOOP_BITS_20_TIMES, LED_COLOR_RED_MASK, LED_BIT_MAP_LOW_POWER},
};

/*============================================================================*
 *                              Global Variables
 *============================================================================*/
bool led_gpio_ctl_is_working = false;

/*============================================================================*
 *                              Functions Declaration
 *============================================================================*/
#define LED_TIMER_STATE()                k_timer_status_get(&led_gpio_ctrl_timer)
#define LED_TIMER_START_ONESHOT(time)    k_timer_start(&led_gpio_ctrl_timer, K_MSEC(time),K_NO_WAIT)
#define LED_TIMER_STOP()                 k_timer_stop(&led_gpio_ctrl_timer)
#define LED_TIMER_START_PERIODLY(timeout,period) \    
                                         k_timer_start(&led_gpio_ctrl_timer, K_MSEC(timeout),K_MSEC(period))

#if (LED_ON_LEVEL_TRIG == LED_ON_LEVEL_HIGH)
#define led_on(index)            Pad_Config(index, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH)
#define led_off(index)           Pad_Config(index, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW)
#elif (LED_ON_LEVEL_TRIG == LED_ON_LEVEL_LOW)
#define led_on(index)            Pad_Config(index, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW)
#define led_off(index)           Pad_Config(index, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH)
#endif

static void led_gpio_ctl_turn_on(uint8_t led_index);
static void led_gpio_ctl_turn_off(uint8_t led_index);
static uint32_t led_get_next_event(const uint32_t bitmap);
static void led_tick_handle(uint8_t led_index);
static void led_gpio_ctrl_timer_cb(struct k_timer *p_timer);
void led_gpio_ctl_pad_config(void);
void led_gpio_ctl_module_init(void);
bool led_gpio_ctl_driver_dlps_check(void);
T_LED_RET_CAUSE led_blink_start(uint16_t led_index, LED_TYPE type, uint8_t cnt);
T_LED_RET_CAUSE led_blink_exit(uint16_t led_index, LED_TYPE type);
void led_timer_state(uint32_t state);

static const struct gpio_dt_spec gpio_leds[7] = {
    GPIO_SPEC(LED1_NODE),
    GPIO_SPEC(LED2_NODE),
    GPIO_SPEC(LED3_NODE),
    GPIO_SPEC(LED4_NODE),
    GPIO_SPEC(LED5_NODE),
    GPIO_SPEC(LED6_NODE),
    GPIO_SPEC(LED7_NODE)
};
static const uint32_t led[7] = {
    LED_1, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7
};
/*============================================================================*
 *                              Private Funcitons
 *============================================================================*/
/**
 * @brief  Turn on single led
 * @param  led_index - led index
 * @param  grayscale - led rgb grayscale
 * @return None
 */
static void led_gpio_ctl_turn_on(uint8_t led_index)
{
    if(led_index == 0)
    {
        if (led_gpio_ctl_is_working == false)
        {
            led_gpio_ctl_is_working = true;
            led_driver_enable_boost_power();
        }
        for (uint8_t i = 0; i < 3; i++)
        {
#if LED_DEBUG
        //DBG_DIRECT("[led_gpio_ctl_turn_on] led_index = %d, led_current_type = %d, color_mask = %d",
        //                led_index, led_arr[led_index].led_current_type,
        //                led_event_arr[led_arr[led_index].led_current_type].led_color_mask_bit[i]);
#endif
            led_on(led_pin_pwm[led_index].rgb_led_pin[i]);
        }
    }
    else
    {
        led_on(led_pin_gpio[led_index-1]);
    }
}

/**
 * @brief  Turn off single led
 * @param  led_index - led index
 * @return None
 */
static void led_gpio_ctl_turn_off(uint8_t led_index)
{
#if LED_DEBUG
    //DBG_DIRECT("[Led] led_gpio_ctl_turn_off");
#endif
    if(led_index == 0)
    {
        for (uint8_t i = 0; i < 3; i++)
        {
            led_off(led_pin_pwm[led_index].rgb_led_pin[i]);
        }
    }
    else
    {
        led_off(led_pin_gpio[led_index]);
    }
}

/**
 * @brief  Handle tick message for each led event.
 * @param  None
 * @return None
 */
static void led_tick_handle(uint8_t led_index)
{
    if (0 == led_arr[led_index].led_map)
    {
        return;
    }

    for (uint8_t event_index = 0; event_index < LED_TYPE_MAX; event_index++)
    {
        /* update led state, when new tick come */
        uint32_t mask = (1 << event_index);

        if (0 == (mask & led_arr[led_index].led_map))
        {
            continue;
        }

        uint16_t led_cur_tick_cnt = led_tick_cnt +
                                    led_arr[led_index].led_cnt_arr[event_index].cur_tick_cnt_offset;
        if ((led_arr[led_index].led_cnt_arr[event_index].led_event_loop_cnt > 0)
            && (led_tick_cnt != 0)
            && ((led_cur_tick_cnt % led_event_arr[event_index].led_loop_cnt) == 0))
        {
            led_arr[led_index].led_cnt_arr[event_index].led_event_loop_cnt --;
            if (0 == led_arr[led_index].led_cnt_arr[event_index].led_event_loop_cnt)
            {
                /* clear event bit */
                led_arr[led_index].led_map &= ~(1 << event_index);
                if (led_arr[led_index].led_map == 0)
                {
                    led_gpio_ctl_turn_off(led_index);
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
static uint32_t led_get_next_event(uint32_t bitmap)
{
    uint32_t next_event = LED_TYPE_IDLE;
    for (uint32_t event_index = 0; event_index < LED_TYPE_MAX; event_index++)
    {
        uint32_t bit_mask = 0;
        bit_mask |= (1 << event_index);
        if (bit_mask & bitmap)
        {
            next_event = event_index;
            break;
        }
    }
#if LED_DEBUG
    //APP_PRINT_INFO1("[Led] next event type = %d", next_event);
#endif
    return next_event;
}

/**
* @brief  Battery control timer callback, will be called periodically to check if pwm led status need change
* @param  p_timer - timer handler
* @return None
*/
static uint8_t led_flag = 0;
void led_gpio_ctrl_timer_cb(struct k_timer *p_timer)
{
    DBG_DIRECT("led_gpio_ctrl_timer_cb");
#if LED_DEBUG
    //APP_PRINT_INFO1("[Led] led_gpio_ctrl_timer_cb, led_tick_cnt = 0x%x", led_tick_cnt);
#endif
   
    // uint32_t mask = 0;
    // bool event_flag = false;

    // for (uint8_t led_index = 0; led_index < LED_GPIO_CTL_PWM_NUM; led_index++)
    // {
        
    //     uint32_t map = 0;
    //     uint32_t type = 0;

    //     /* get highest prio event type */
    //     type = led_get_next_event(led_arr[led_index].led_map);
    //     if (type != led_arr[led_index].led_current_type)
    //     {
    //         led_arr[led_index].led_current_type = type;
    //     }
    //     if (led_arr[led_index].led_map != 0)
    //     {
    //         event_flag = true;
    //     }
    //     else
    //     {
    //         continue;
    //     }

    //     /* get highest event mask */
    //     mask = (1 << (led_tick_cnt % led_event_arr[type].led_loop_cnt));

    //     /* get highest prio bit map */
    //     if (led_event_arr[type].led_type_index == type)
    //     {
    //         map = led_event_arr[type].led_bit_map;
    //     }

    //     /* set led state according to bit map */
    //     if (mask & map)
    //     {
    //        // led_gpio_ctl_turn_on(led_index);
    //     }
    //     else
    //     {
    //         //led_gpio_ctl_turn_off(led_index);
    //     }

    //     /* led tick msg handle, update left events state */
    //     led_tick_handle(led_index);
    // }

    // /**update led blink tick*/
    // led_tick_cnt ++;

    // if (event_flag == false)
    // {
    //     LED_TIMER_STOP();
    //     led_tick_cnt = 0;
    //     led_gpio_ctl_is_working = false;
    //     led_driver_disable_boost_power();
    // }
}

/*============================================================================*
 *                              Public Funcitons
 *============================================================================*/
/**
 * @brief  Led gpio control module pad config
 * @param  None
 * @return None
 */
void led_gpio_ctl_pad_config(void)
{
    for (uint8_t led_index = 0; led_index < LED_GPIO_CTL_PWM_NUM; led_index++)
    {
        for (uint8_t i = 0; i < 3; i++)
        {
            led_off(led_pin_pwm[led_index].rgb_led_pin[i]);
        }
    }
    for (uint8_t led_index = 0; led_index < LED_GPIO_CTL_NUM; led_index++)
    {

        led_off(led_pin_gpio[led_index]);
    }
}

/**
 * @brief  Initialize led gpio ctl module
 * @param  None
 * @return None
 */
void led_gpio_ctl_module_init(void)
{
    LOG_INF("led gpio timer config");
    uint8_t i,j;
    for(i=0; i<LED_GPIO_CTL_PWM_NUM; i++)
    {
        for(j=0; j<3; j++)
        {
            led_pin_pwm[i].rgb_led_pin[j] = led[4+j];      
        }
    }
    for(i=0; i<LED_GPIO_CTL_NUM; i++)
    {
        DBG_DIRECT("led turn on led %d,led_pin is %d",i+1,led[i]);
        led_pin_gpio[i] = led[i];
        led_on(led_pin_gpio[i]);
    }
    //APP_PRINT_INFO0("[led_module_init] led gpio and pwm init");
    k_timer_init(&led_gpio_ctrl_timer, led_gpio_ctrl_timer_cb, NULL);
    DBG_DIRECT("KMESC PERIOD=%d",K_MSEC(50));//100 ticks = 1 second
    
    //LED_TIMER_START_ONESHOT(LED_PERIOD*5);
    //led_blink_start(0,LED_TYPE_BLINK_BLE_PAIR_SUCCESS,10);

}

/**
 * @brief  Check if led gpio control module allow enter dlps
 * @param  None
 * @return Result - true: allow enter dlps, false: can not enter dlps
 */
bool led_gpio_ctl_driver_dlps_check(void)
{
    return true;
}

/**
 * @brief  Start led blink
 * @param  index - led index to set
 * @param  type - led type to set
 * @param  cnt  - led blink count
 * @return Result of starting led
 */
T_LED_RET_CAUSE led_blink_start(uint16_t led_index, LED_TYPE type, uint8_t cnt)
{
#if LED_DEBUG
    //APP_PRINT_INFO3("[Led] led_blink_start: index %d, type %d, cnt %d", led_index, type, cnt);
#endif
    if (led_index >= LED_GPIO_CTL_PWM_NUM)
    {
        return LED_ERR_INDEX;
    }

    if ((LED_TYPE_IDLE == type) || (LED_TYPE_MAX <= type))
    {
        return LED_ERR_TYPE;
    }

    uint32_t state = 0;

    /* set led event map flag */
    led_arr[led_index].led_map |= (1 << type);

    if (cnt > 0)
    {
        led_arr[led_index].led_cnt_arr[type].led_event_loop_cnt = cnt;
    }
    else
    {
        led_arr[led_index].led_cnt_arr[type].led_event_loop_cnt = 0;
    }
    led_arr[led_index].led_cnt_arr[type].cur_tick_cnt_offset = 0 - led_tick_cnt;

    /* start led tick */
    led_timer_state(state);
    /* state>0, timer has expired
       state=0, timer was stopped
       state<0, timer is still running
    */
    if (state)
    {
        LED_TIMER_START_ONESHOT(LED_FIRST_PERIOD);
    }

    return LED_SUCCESS;
}

/**
 * @brief  Terminate led blink according to the led type
 * @param  index - led index to set
 * @param  type - led type to set
 * @return Result of exiting led
 */
T_LED_RET_CAUSE led_blink_exit(uint16_t led_index, LED_TYPE type)
{
    if (led_index >= LED_GPIO_CTL_PWM_NUM)
    {
        return LED_ERR_INDEX;
    }
    if ((LED_TYPE_IDLE == type) || (LED_TYPE_MAX <= type))
    {
        return LED_ERR_TYPE;
    }

    /* start led */
    if (LED_TYPE_IDLE != (led_arr[led_index].led_map & (1 << type)))
    {
        /* clear current led type bit */
        led_arr[led_index].led_map &= ~(1 << type);
#if LED_DEBUG
        //APP_PRINT_INFO3("[Led] led_blink_exit, led_index = %d, type = %d, led_map = 0x%x", led_index, type,
        //                led_arr[led_index].led_map);
#endif
        /* check whether the led can be turned off */
        if (led_arr[led_index].led_map == 0)
        {
            led_gpio_ctl_turn_off(led_index);
        }

        /* check whether the led can be turned off */
        bool event_flag = false;
        for (uint8_t index = 0; index < LED_GPIO_CTL_PWM_NUM; index++)
        {
            /* get highest prio event type */
            if (led_arr[index].led_map != 0)
            {
                event_flag = true;
                break;
            }
        }

        if (!event_flag)
        {
            LED_TIMER_STOP();
            led_tick_cnt = 0;
            led_gpio_ctl_is_working = false;
            led_driver_disable_boost_power();
        }
    }
    return LED_SUCCESS;
}

/**
 * @brief  Get led software timer state
 * @param  None
 * @return Led timer state
 */
void led_timer_state(uint32_t state)
{
    state = LED_TIMER_STATE();
}