/*
 * Copyright (c) 2024 Realtek Semiconductor 
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _BOARD_H_
#define _BOARD_H_

/*******************************************************
*                 Button Config
*******************************************************/
#define SUPPORT_KEYSCAN_FEATURE     1
/* if set KEYSCAN_FIFO_LIMIT larger than 3, need to caution ghost key issue */
#if SUPPORT_KEYSCAN_FEATURE
#define KEYBOARD_KEYSCAN_AUTO_SCAN         1
#define KEYBOARD_KEYSCAN_MANUAL_SCAN       2
#define KEYBOARD_KEYSCAN_SEL               KEYBOARD_KEYSCAN_MANUAL_SCAN

#define SHOW_DEMO                   0   /*different fn comb program*/

#define KEYSCAN_FIFO_LIMIT    3  /* value range from 1 to 8 */

#define KEYPAD_ROW_SIZE       4
#define KEYPAD_COLUMN_SIZE    4

#define ROW0                  P2_6
#define ROW1                  P2_5
#define ROW2                  P3_4
#define ROW3                  P3_3

#define COLUMN0               P4_2
#define COLUMN1               P4_1
#define COLUMN2               P4_0
#define COLUMN3               P4_3
#endif
/*******************************************************
*                 LED Config
*******************************************************/
#define LED_DEBUG                       1
#define SUPPORT_LED_INDICATION_FEATURE  1
#define LED_FOR_TEST                    0

#if SUPPORT_LED_INDICATION_FEATURE
#define LED_0                           1

#define LED_1                           MICBIAS//GPIOA_16, white
#define LED_2                           P2_7//GPIOA_28,white
#define LED_3                           P2_6 //GPIOA_27,blue
#define LED_4                           P2_5 //GPIOA_26,green
#define LED_5                           P2_4 //GPIOA_25,R
#define LED_6                           P2_3 //GPIOA_24,G
#define LED_7                           P9_3 //GPIOA_23,B

#define LED_NUM_MAX                     2
#define LED_GPIO_CTL_NUM                4 /* start from LED_1 , range from 0 to 32*/
#define LED_GPIO_CTL_PWM_NUM            1
#define LED_HW_TIM_PWM_CTL_NUM          1 /* can only be 0 or 1 */
#define LED_HW_TIM_PWM_CTL_INDEX        LED_0 /* must follow gpio register control led index */

#define LED_EN_PIN                      P10_0 //GPIOB_29, power on(ctrl led1-7)

#define LED0_R_PIN                      P9_4 //GPIOB_25
#define LED0_G_PIN                      P9_5 //GPIOB_26
#define LED0_B_PIN                      P9_6 //GPIOB_27

// #define LED2_R_PIN                      P1_0
// #define LED2_G_PIN                      P1_1
// #define LED2_B_PIN                      P3_5
/* voltage level to trigger LED On action */
#define LED_ON_LEVEL_HIGH               0
#define LED_ON_LEVEL_LOW                1

#define LED_ON_LEVEL_TRIG               LED_ON_LEVEL_LOW

#endif

/**led config*/
#define SHOW_DEMO       0
#define FEAUTRE_SUPPORT_VENDOR_SHORTCUT_KEY     0

/* keyscan */
#define FEAUTRE_SUPPORT_VENDOR_SHORTCUT_KEY 0

#endif