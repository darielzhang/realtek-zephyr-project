/*
 * Copyright (c) 2024 dariel Zhang
 * Copyright (c) 2024 Realtek Semiconductor 
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _LED_GPIO_CRTL_DRIVER_H_
#define _LED_GPIO_CRTL_DRIVER_H_

#include "board.h"

#if 1//(SUPPORT_LED_INDICATION_FEATURE & LED_GPIO_CTL_NUM)
/**
*   @brief user guide for led driver
*   The driver support multi prio, multi event, and multi led.
*   If you want to control a led with a event as you designed, there is two way to
*   achieve this, modify the exist event and redefine a new event.
*
*   The led driver is drived by software timer, the timer peroid is 50ms by default.
*
*   #define  LED_PERIOD   50 //you an change the value according your requirement
*
*   For the whole led driver system, there is 5 place you might modify.
*
*   1. Define led num and led pin, which is in file board.h;
*      For example, define 2 leds as followed:
*      #define  LED_NUM_MAX   0x02
*      #define  LED_INDEX(n)   (n<<8)

*      //uint16_t, first byte led index, last byte led pin
*      #define  LED_1         (LED_INDEX(0) | P2_2)
*      #define  LED_2         (LED_INDEX(1) | P0_2)
*
*   2. Define led type index, which is in file led_driver.h, these types must be
*      defined in sequence.
*      For example,
*      #define  LED_TYPE_BLINK_OTA_UPDATE       (8)
*
*   3. Define led loop bits num, in file led_driver.h
*      //max value 32, min value 1, it indicate that there is how many bits to loop
*      //from LSB
*      #define LED_LOOP_BITS_OTA_UPDATE          (15)
*
*   4. Define led event bit map, in file led_driver.h
*      // it must be cooperated with led loop bits num
*      #define  LED_BIT_MAP_OTA_UPDATE       (0x4210)//on 50ms, off 200ms, period 50ms
*
*   5. Update led event table, led_event_arr[LED_TYPE_MAX], in file led_driver.c
*      Before you use the event you define ,you need to add led type, led loop bit num,
*      and led event bit map into event table.
*      const T_LED_EVENT_STG led_event_arr[LED_TYPE_MAX] =
*      {
*        {LED_TYPE_BLINK_OTA_UPDATE, LED_LOOP_BITS_OTA_UPDATE,  LED_BIT_MAP_OTA_UPDATE},
*      };
*
*   There are three interfaces for Led driver, as follow.
*
*   void led_module_init(void); // called when system boot;
*   T_LED_RET_CAUSE led_blink_start(uint16_t led_index, LED_TYPE type, uint8_t cnt);
*   T_LED_RET_CAUSE led_blink_exit(uint16_t led_index, LED_TYPE type);
*/

// #define LED_DEBUG                       0
/*software timer period*/
#define LED_EN_BOOST_POWER_PERIOD       10  /*10ms*/
#define LED_FIRST_PERIOD                10  /*10ms*/
#define LED_PERIOD                      50  /*50ms*/

/*led event types, the less value, the higher prio of the event*/
typedef   uint32_t      LED_TYPE;

typedef enum
{
    LED_TYPE_IDLE,
    LED_TYPE_BLINK_BLE_PAIR_SUCCESS,
    LED_TYPE_BLINK_BLE_PAIR_ADV,
    LED_TYPE_BLINK_PPT_PAIR_SUCCESS,
    LED_TYPE_BLINK_PPT_PAIR_ADV,
    LED_TYPE_BLINK_RECONNECT_ADV,
    LED_TYPE_BLINK_REPORT_RATE,
    LED_TYPE_BLINK_DPI_1,
    LED_TYPE_BLINK_DPI_2,
    LED_TYPE_BLINK_DPI_3,
    LED_TYPE_BLINK_DPI_4,
    LED_TYPE_BLINK_DPI_5,
    LED_TYPE_BLINK_RESET,
    LED_TYPE_BLINK_CHARGING,
    LED_TYPE_BLINK_LOW_POWER,
    LED_TYPE_MAX
} LED_TYPE_ENUM;

/*led loop count for each event*/
#define   LED_LOOP_BITS_IDLE                    20
#define   LED_LOOP_BITS_20_TIMES                20

/*led color mask bit : 32bits */
#define   LED_COLOR_IDLE_MASK                   {0, 0, 0}
#define   LED_COLOR_RED_MASK                    {1, 0, 0}
#define   LED_COLOR_GREEN_MASK                  {0, 1, 0}
#define   LED_COLOR_BLUE_MASK                   {0, 0, 1}
#define   LED_COLOR_YELLOW_MASK                 {1, 1, 0}
#define   LED_COLOR_CYAN_MASK                   {0, 1, 1}
#define   LED_COLOR_PURPLR_MASK                 {1, 0, 1}
#define   LED_COLOR_WHITE_MASK                  {1, 1, 1}

/*led bit map 32bits, High bits(low priority) ---  Low bits(high priority) */
#define LED_BIT_MAP_INVALID                     0x00000000
#define LED_BIT_MAP_BLE_PAIR_ADV                0x000003ff
#define LED_BIT_MAP_PPT_PAIR_ADV                0x000003ff
#define LED_BIT_MAP_RECONNECT_ADV               0x000003ff
#define LED_BIT_MAP_LOW_POWER                   0x0000003f
#define LED_BIT_MAP_ALWAYS_ON                   0xffffffff

typedef struct
{
    uint8_t   led_type_index;
    uint8_t   led_loop_cnt;
    uint8_t   led_color_mask_bit[3];
    uint32_t  led_bit_map;
} T_LED_EVENT_STG;

/*struct support for led blink count*/
typedef struct
{
    uint8_t led_event_loop_cnt;
    float cur_tick_cnt_offset;
} T_LED_CNT_STG;

/*support for each led data*/
typedef struct
{
    uint8_t led_index;
    uint8_t led_current_type;
    T_LED_CNT_STG led_cnt_arr[LED_TYPE_MAX];
    uint32_t led_map;
} T_LED_DATA_STG;

/*support for each led data*/
typedef struct
{
    uint8_t rgb_led_pin[3];
} T_LED_PIN;

/*led return code*/
typedef enum
{
    LED_SUCCESS                      = 0,
    LED_ERR_TYPE                     = 1,
    LED_ERR_INDEX                    = 2,
    LED_ERR_CODE_MAX
} T_LED_RET_CAUSE;

/*============================================================================*
*                        Export Global Variables
*============================================================================*/
extern bool led_gpio_ctl_is_working;

/*============================================================================*
 *                        <Led Module Interface>
 *============================================================================*/
void led_gpio_ctl_module_init(void);
T_LED_RET_CAUSE led_blink_start(uint16_t led_index, LED_TYPE type, uint8_t cnt);
T_LED_RET_CAUSE led_blink_exit(uint16_t led_index, LED_TYPE type);
void led_gpio_ctl_pad_config(void);
bool led_gpio_ctl_driver_dlps_check(void);
void led_timer_state(uint32_t state);
#define LED_BLINK(led_index, type, n)   led_blink_start(led_index, type, n)
#define LED_BLINK_EXIT(led_index, type) led_blink_exit(led_index, type)

#else
#define LED_BLINK(led_index, type, n)
#define LED_BLINK_EXIT(led_index, type)
#endif

#endif