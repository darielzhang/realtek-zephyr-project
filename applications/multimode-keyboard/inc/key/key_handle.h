/*
 * Copyright(c) 2020, Realtek Semiconductor Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _KEY_HANDLE_H_
#define _KEY_HANDLE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <keyscan_driver.h>
#include "board.h"

/*============================================================================*
 *                              Macro Definitions
 *============================================================================*/
/* define the bit mask of combine keys */
#define INVALID_COMBINE_KEYS_BIT_MASK               0x0000
#define PAIRING_COMBINE_KEYS_BIT_MASK               0x0001
#define IR_LEARNING_COMBINE_KEYS_BIT_MASK           0x0002
#define HCI_UART_TEST_COMBINE_KEYS_BIT_MASK         0x0004
#define DATA_UART_TEST_COMBINE_KEYS_BIT_MASK        0x0008
#define SINGLE_TONE_TEST_COMBINE_KEYS_BIT_MASK      0x0010
#define FAST_PAIR_1_COMBINE_KEYS_BIT_MASK           0x0020
#define FAST_PAIR_2_COMBINE_KEYS_BIT_MASK           0x0040
#define FAST_PAIR_3_COMBINE_KEYS_BIT_MASK           0x0080
#define FAST_PAIR_4_COMBINE_KEYS_BIT_MASK           0x0100
#define FAST_PAIR_5_COMBINE_KEYS_BIT_MASK           0x0200
#define BUG_REPORT_COMBINE_KEYS_BIT_MASK            0x0400
#define FACTORY_RESET_COMBINE_KEYS_BIT_MASK         0x8000

#define COMBINE_KEYS_DETECT_TIMEOUT         4000  /* 4 sec */
#define FACTORY_RESET_KEYS_DETECT_TIMEOUT   4000  /* 4 sec */
#define BUG_REPOERT_DETECT_TIMEOUT          1000  /* 1 sec */
#define NOTIFY_KEY_DATA_TIMEOUT             300   /* 300 ms */

#define KEYBOARD_DATA_SIZE                  4
#define CONSUMER_DATA_SIZE                  2
#define VENDOR_DATA_SIZE                    4

#define FN_CODE_NUM                     18

#define KEYBOARD_REPORT_ID      0x07
#define CONSUMER_REPORT_ID      0x0C
#define VENDOR_REPORT_ID        0x10
/*============================================================================*
 *                              Types
 *============================================================================*/
typedef enum
{
    VK_NONE             = 0x00,
    VK_PAUSE            = 0x01,
    VK_Q                = 0x02,
    VK_W                = 0x03,
    VK_E                = 0x04,
    VK_R,
    VK_U,
    VK_I,
    VK_O,
    VK_P,
    VK_SCROLL,
    VK_K14,
    VK_SK_7,
    VK_SK_8,
    VK_SK_9,
    VK_SK_PLUS,
    VK_WAKEUP_L,
    VK_MAIL,
    VK_K150,
    VK_POWER_L,
    VK_TAB,
    VK_CAP,
    VK_F3,
    VK_T,
    VK_Y,
    VK_BRACKET_R,
    VK_F7,
    VK_BRACKET_L,
    VK_000,
    VK_BACK,
    VK_SK_4,
    VK_SK_5,
    VK_SK_6,
    VK_K107,
    VK_SHIFT_L,
    VK_WIN_L,
    VK_W_FAV,
    VK_EURO,
    VK_A,
    VK_S,
    VK_D,
    VK_F,
    VK_J,
    VK_K,
    VK_L,
    VK_SEMICOLON,
    VK_FN,
    VK_SLASH_R,
    VK_SK_1,
    VK_SK_2,
    VK_SK_3,
    VK_SK_ENTER,
    VK_SHIFT_R,
    VK_W_FORWARD,
    VK_WIN_R,
    VK_SLEEP_L,
    VK_ESC,
    VK_K45,
    VK_F4,
    VK_G,
    VK_H,
    VK_F6,
    VK_DOLLAR,
    VK_LEFT_COLON,
    VK_ALT_L,
    VK_F11,
    VK_SPACE,
    VK_SK_0,
    VK_SK_END,
    VK_ARROW_U,
    VK_VOLUMN_DOWN,
    VK_W_STOP,
    VK_COMPUTER,
    VK_INS,
    VK_Z,
    VK_X,
    VK_C,
    VK_V,
    VK_M,
    VK_COMMA,
    Vk_PERIOD,
    VK_K42,
    VK_MODE,
    VK_ENTER,
    VK_NUM,
    VK_SK_SLASH_L,
    VK_SK_ASTERISK,
    VK_PLAY,
    VK_VOLUMN_PLUS,
    VK_W_BACK,
    VK_STOP,
    VK_WAKEUP_R,
    VK_N_CHG,
    VK_CHG,
    VK_ROMA,
    VK_B,
    VK_N,
    VK_K56,
    VK_APP,
    VK_SLASH_L,
    VK_ALT_R,
    VK_F12,
    VK_ARROW_D,
    VK_ARROW_R,
    VK_SK_UNDERLINE,
    VK_ARROW_L,
    VK_NEXTTRACK,
    VK_W_REFRESH,
    VK_CAL,
    VK_CTRL_L,
    VK_TILDE,
    VK_F1,
    VK_F2,
    VK_5,
    VK_6,
    VK_PLUS,
    VK_F8,
    VK_UNDERLINE,
    VK_00,
    VK_F9,
    VK_DEL,
    VK_CTRL_R,
    VK_PAGE_UP,
    VK_HOME,
    VK_PREVTRACK,
    VK_MUTE,
    VK_W_WEB,
    VK_F5,
    VK_1,
    VK_2,
    VK_3,
    VK_4,
    VK_7,
    VK_8,
    VK_9,
    VK_0,
    VK_PRINT,
    VK_F10,
    VK_POWER_R,
    VK_SLEEP_R,
    VK_PAGE_DOWN,
    VK_END,
    VK_MEDIA,
    VK_W_SEARCH,
    VK_K151,
    KEY_INDEX_ENUM_GUAID,
} T_KEY_INDEX_DEF;


/* define the HID usage pages */
typedef enum
{
    HID_UNDEFINED_PAGE  = 0x00,
    HID_KEYBOARD_PAGE   = 0x07,
    HID_CONSUMER_PAGE   = 0x0C,
    HID_VENDOR_PAGE     = 0x10,
    HID_SPECIAL_KEYBOARD_PAGE = 0x11,
} T_HID_USAGE_PAGES_DEF;

/* define the key code table size, the value should modify according to KEY_CODE_TABLE */
#define KEY_CODE_TABLE_SIZE KEY_INDEX_ENUM_GUAID

/* define the struct of key code */
typedef struct
{
    uint8_t hid_usage_page;
    uint16_t hid_usage_id;
} T_KEY_CODE_DEF;

typedef struct
{
    T_KEY_INDEX_DEF type;
    uint8_t hid_usage_page;
    uint16_t keyboard_fn_data_id;
} T_FN_KEY_CODE_DEF;

typedef struct
{
    uint8_t keyboard_usage_cnt;
    uint8_t keyboard_usage_buffer[KEYBOARD_DATA_SIZE - 1];
    uint8_t special_kb_value;
    uint8_t keyboard_consumer_usage_buffer[CONSUMER_DATA_SIZE];
#if FEAUTRE_SUPPORT_VENDOR_SHORTCUT_KEY
    uint16_t keyboard_vendor_buffer;
#endif
} T_KEY_HID_USAGES_BUFFER;

/* Key global parameters' struct */
typedef struct
{
    uint32_t combine_keys_status;  /* to indicate the status of combined keys */
    T_KEY_HID_USAGES_BUFFER current_hid_usage_buf;  /* to indicate the current key HID usage buffer */
    T_KEY_HID_USAGES_BUFFER prev_hid_usage_buf;  /* to indicate the preivous key HID usage buffer */
    T_KEYSCAN_FIFO_DATA pending_keyscan_fifo_data;  /* to indicate the pending keyscan FIFO data */
    T_KEY_INDEX_DEF last_long_pressed_key_index; /* to indicate the last long pressed key index */
} T_KEY_HANDLE_GLOBAL_DATA;

typedef enum
{
    FNQ = 1,
    FNW = 2,
    FNE = 3,
    FNNULL = 4,
} T_CHANGE_MODE_KEY_TYPE;

/*============================================================================*
*                           Export Global Variables
*============================================================================*/
extern T_KEY_HANDLE_GLOBAL_DATA key_handle_global_data;
/*============================================================================*
 *                          Functions
 *============================================================================*/
void keyscan_init_config(void);
void key_handle_init_data(void);
void key_handle_notify_pressed_event(T_KEYSCAN_FIFO_DATA *keyscan_fifo_data);
void key_handle_notify_all_release_event(T_KEYSCAN_FIFO_DATA *keyscan_fifo_data);
void key_handle_init_timer(void);

#ifdef __cplusplus
}
#endif

#endif
