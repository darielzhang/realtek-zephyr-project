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
#include "zephyr/drivers/kscan.h"
#include "zephyr/logging/log.h"
#include "soc.h"

#include "board.h"
#include "key_handle.h"
#include "led_driver.h"
#include "app_task.h"
#include "usb_handle.h"
#include "app_msg.h"
#include "trace.h"

LOG_MODULE_REGISTER(key, LOG_LEVEL_INF);

static const T_KEY_INDEX_DEF KEY_MAPPING_TABLE[8][18] =
{
    {
        VK_PAUSE,      VK_Q,         VK_W,        VK_E,          VK_R,             VK_U,           VK_I,               VK_O,          VK_P,
        VK_SCROLL,     VK_K14,       VK_SK_7,     VK_SK_8,       VK_SK_9,          VK_SK_PLUS,     VK_WAKEUP_L,        VK_MAIL,       VK_K150
    },

    {
        VK_POWER_L,    VK_TAB,       VK_CAP,      VK_F3,         VK_T,             VK_Y,           VK_BRACKET_R,       VK_F7,         VK_BRACKET_L,
        VK_000,        VK_BACK,      VK_SK_4,     VK_SK_5,       VK_SK_6,          VK_K107,        VK_SHIFT_L,         VK_WIN_L,      VK_W_FAV
    },

    {
        VK_EURO,      VK_A,         VK_S,        VK_D,          VK_F,             VK_J,           VK_K,               VK_L,          VK_SEMICOLON,
        VK_FN,        VK_SLASH_R,   VK_SK_1,     VK_SK_2,       VK_SK_3,          VK_SK_ENTER,    VK_SHIFT_R,         VK_W_FORWARD,  VK_WIN_R
    },

    {
        VK_SLEEP_L,   VK_ESC,       VK_K45,      VK_F4,         VK_G,             VK_H,           VK_F6,              VK_DOLLAR,     VK_LEFT_COLON,
        VK_ALT_L,     VK_F11,       VK_SPACE,    VK_SK_0,       VK_SK_END,        VK_ARROW_U,     VK_VOLUMN_DOWN,     VK_W_STOP,     VK_COMPUTER
    },

    {
        VK_INS,       VK_Z,         VK_X,        VK_C,          VK_V,             VK_M,           VK_COMMA,           Vk_PERIOD,     VK_K42,
        VK_MODE,      VK_ENTER,     VK_NUM,      VK_SK_SLASH_L,  VK_SK_ASTERISK,  VK_PLAY,        VK_VOLUMN_PLUS,     VK_W_BACK,     VK_STOP
    },

    {
        VK_WAKEUP_R,  VK_N_CHG,     VK_CHG,      VK_ROMA,       VK_B,             VK_N,           VK_K56,             VK_APP,        VK_SLASH_L,
        VK_ALT_R,     VK_F12,       VK_ARROW_D,  VK_ARROW_R,    VK_SK_UNDERLINE,  VK_ARROW_L,     VK_NEXTTRACK,       VK_W_REFRESH,  VK_CAL
    },

    {
        VK_CTRL_L,   VK_TILDE,      VK_F1,       VK_F2,         VK_5,             VK_6,           VK_PLUS,            VK_F8,         VK_UNDERLINE,
        VK_00,       VK_F9,         VK_DEL,      VK_CTRL_R,     VK_PAGE_UP,       VK_HOME,        VK_PREVTRACK,       VK_MUTE,       VK_W_WEB
    },

    {
        VK_F5,       VK_1,          VK_2,        VK_3,          VK_4,             VK_7,           VK_8,               VK_9,          VK_0,
        VK_PRINT,    VK_F10,        VK_POWER_R,  VK_SLEEP_R,    VK_PAGE_DOWN,     VK_END,         VK_MEDIA,           VK_W_SEARCH,   VK_K151
    },
};
#if SHOW_DEMO
/*comsumer key, Fn+Fx*/
static const T_FN_KEY_CODE_DEF fn_code_table[FN_CODE_NUM] =
{
    {VK_F1,   HID_CONSUMER_PAGE,  0x0070},    //mute fn key id
    {VK_F2,   HID_CONSUMER_PAGE,  0x006f},    //volume decrease
    {VK_F3,   HID_KEYBOARD_PAGE,  0x2b08},    //volumn increase
    {VK_F4,   HID_KEYBOARD_PAGE,  0x1d01},    //backward
    {VK_F5,   HID_CONSUMER_PAGE,  0x0221},    //pause or stop
    {VK_F6,   HID_CONSUMER_PAGE,  0x00b6},    //forward
    {VK_F7,   HID_CONSUMER_PAGE,  0x00cd},    //brightness decrease
    {VK_F8,   HID_CONSUMER_PAGE,  0x00b5},    //brightness increase
    {VK_F9,   HID_CONSUMER_PAGE,  0x00e2},
    {VK_F10,  HID_CONSUMER_PAGE,  0x00ea},
    {VK_F11,  HID_CONSUMER_PAGE,  0x00e9},
    {VK_F12,  HID_KEYBOARD_PAGE,  0x2c08},
    {VK_DEL,  HID_KEYBOARD_PAGE,  0x0f08},
};
#else
/*comsumer key, Fn+Fx*/
static const T_FN_KEY_CODE_DEF fn_code_table[FN_CODE_NUM] =
{
    {VK_F1,      HID_CONSUMER_PAGE,    0x00e2},    //consumer key, mute fn key id
    {VK_F2,      HID_CONSUMER_PAGE,    0x00ea},    //consumer key, volume decrease
    {VK_F3,      HID_CONSUMER_PAGE,    0x00e9},    //consumer key, volumn increase
    {VK_F4,      HID_CONSUMER_PAGE,    0x00b6},    //consumer key, backward
    {VK_F5,      HID_CONSUMER_PAGE,    0x00cd},    //consumer key, pause or stop
    {VK_F6,      HID_CONSUMER_PAGE,    0x00b5},    //consumer key, forward
    {VK_F7,      HID_CONSUMER_PAGE,    0x0070},    //consumer key, brightness decrease
    {VK_F8,      HID_CONSUMER_PAGE,    0x006f},    //consumer key, brightness increase
    {VK_F9,      HID_KEYBOARD_PAGE,    0x0401},    //ctrl+A
    {VK_F10,     HID_KEYBOARD_PAGE,    0x0601},    //copy
    {VK_F11,     HID_KEYBOARD_PAGE,    0x1901},    //paste
    {VK_F12,     HID_KEYBOARD_PAGE,    0x1b01},    //cut
    {VK_DEL,     HID_KEYBOARD_PAGE,    0x0f08},    //lock screan
    {VK_ARROW_U, HID_KEYBOARD_PAGE,    0x4b00},    //page up
    {VK_ARROW_D, HID_KEYBOARD_PAGE,    0x4e00},    //page down
    {VK_ARROW_R, HID_KEYBOARD_PAGE,    0x4d00},    //end
    {VK_ARROW_L, HID_KEYBOARD_PAGE,    0x4a00},    //home
};
#endif

static bool fn_lock = false;
/* BLE HID code table definition, keyboard key*/
const T_KEY_CODE_DEF KEY_CODE_TABLE[KEY_CODE_TABLE_SIZE] =
{
    {HID_UNDEFINED_PAGE,    0x00},      // VK_NONE,
    {HID_KEYBOARD_PAGE,     0x48},      // VK_PAUSE,
    {HID_KEYBOARD_PAGE,     0x14},      // VK_Q,                0x14
    {HID_KEYBOARD_PAGE,     0x1a},      // VK_W,                0x1a
    {HID_KEYBOARD_PAGE,     0x08},      // VK_E,                0x08
    {HID_KEYBOARD_PAGE,     0x15},      // VK_R,                0x15;
    {HID_KEYBOARD_PAGE,     0x18},      // VK_U,                0x18;
    {HID_KEYBOARD_PAGE,     0x0c},      // VK_I,                0x0c;
    {HID_KEYBOARD_PAGE,     0x12},      // VK_O,                0x12;
    {HID_KEYBOARD_PAGE,     0x13},      // VK_P,                0x13;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SCROLL,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_K14,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_7,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_8,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_9,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_PLUS,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_WAKEUP_L,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_MAIL,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_K150,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_POWER_L,
    {HID_KEYBOARD_PAGE,     0x2b},      // VK_TAB,              0x2b;
    {HID_KEYBOARD_PAGE,     0x39},      // VK_CAP,              0X39;
    {HID_KEYBOARD_PAGE,     0x3c},      // VK_F3,               0x3c;
    {HID_KEYBOARD_PAGE,     0x17},      // VK_T,                0x17;
    {HID_KEYBOARD_PAGE,     0x1c},      // VK_Y,                0x1c;
    {HID_KEYBOARD_PAGE,     0x30},      // VK_BRACKET_R,        0x30;
    {HID_KEYBOARD_PAGE,     0x40},      // VK_F7,               0x40;
    {HID_KEYBOARD_PAGE,     0x2f},      // VK_BRACKET_L,        0x2f;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_000,
    {HID_KEYBOARD_PAGE,     0x2a},      // VK_BACK,             0x2a;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_4,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_5,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_6,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_K107,
    {HID_SPECIAL_KEYBOARD_PAGE,      0x02},      // VK_SHIFT_L,          0x02;
    {HID_SPECIAL_KEYBOARD_PAGE,      0x08},      // VK_WIN_L,            0x08;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_W_FAV,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_EURO,
    {HID_KEYBOARD_PAGE,     0x04},      // VK_A                 0x04;
    {HID_KEYBOARD_PAGE,     0x16},      // VK_S,                0x16;
    {HID_KEYBOARD_PAGE,     0x07},      // VK_D,                0x07;
    {HID_KEYBOARD_PAGE,     0x09},      // VK_F,                0x09;
    {HID_KEYBOARD_PAGE,     0x0d},      // VK_J,                0x0d;
    {HID_KEYBOARD_PAGE,     0x0e},      // VK_K,                0x0e;
    {HID_KEYBOARD_PAGE,     0x0f},      // VK_L,                0x0f;
    {HID_KEYBOARD_PAGE,     0x33},      // VK_SEMICOLON,        0x33;
    {HID_UNDEFINED_PAGE,    0x00},      // VK_FN,
    {HID_KEYBOARD_PAGE,     0x31},      // VK_SLASH_R,      0x31;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_1,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_2,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_3,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_ENTER,
    {HID_SPECIAL_KEYBOARD_PAGE,      0x20},      // VK_SHIFT_R,          0x20
    {HID_KEYBOARD_PAGE,     0x00},      // VK_W_FORWARD,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_WIN_R,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SLEEP_L,
    {HID_KEYBOARD_PAGE,     0x29},      // VK_ESC,              0x29;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_K45,
    {HID_KEYBOARD_PAGE,     0x3d},      // VK_F4,               0x3d;
    {HID_KEYBOARD_PAGE,     0x0a},      // VK_G,                0x0a;
    {HID_KEYBOARD_PAGE,     0x0b},      // VK_H,                0x0b;
    {HID_KEYBOARD_PAGE,     0x3f},      // VK_F6,               0x3f;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_DOLLAR,
    {HID_KEYBOARD_PAGE,     0x34},      // VK_LEFT_COLON,       0x34;
    {HID_SPECIAL_KEYBOARD_PAGE,      0x04},      // VK_ALT_L,            0x04;
    {HID_KEYBOARD_PAGE,     0x44},      // VK_F11,              0x44;
    {HID_KEYBOARD_PAGE,     0x2c},      // VK_SPACE,            0x2c;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_0,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_END,
    {HID_KEYBOARD_PAGE,     0x52},      // VK_ARROW_U,          0x52;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_VOLUMN_DOWN,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_W_STOP,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_COMPUTER,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_INS,
    {HID_KEYBOARD_PAGE,     0x1d},      // VK_Z,                0x1d;
    {HID_KEYBOARD_PAGE,     0x1b},      // VK_X,                0x1b;
    {HID_KEYBOARD_PAGE,     0x06},      // VK_C,                0x06;
    {HID_KEYBOARD_PAGE,     0x19},      // VK_V,                0x19;
    {HID_KEYBOARD_PAGE,     0x10},      // VK_M,                0x10;
    {HID_KEYBOARD_PAGE,     0x36},      // VK_COMMA,            0x36;
    {HID_KEYBOARD_PAGE,     0x37},      // Vk_PERIOD,           0x37;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_K42,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_MODE,
    {HID_KEYBOARD_PAGE,     0x28},      // VK_ENTER,            0x28;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_NUM,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_SLASH_L,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_ASTERISK,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_PLAY,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_VOLUMN_PLUS,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_W_BACK,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_STOP,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_WAKEUP_R,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_N_CHG,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_CHG,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_ROMA,
    {HID_KEYBOARD_PAGE,     0x05},      // VK_B,                0x05;
    {HID_KEYBOARD_PAGE,     0x11},      // VK_N,                0x11;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_K56,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_APP,
    {HID_KEYBOARD_PAGE,     0x38},      // VK_SLASH_L,       0x38;
    {HID_SPECIAL_KEYBOARD_PAGE,      0x40},      // VK_ALT_R,            0x40;
    {HID_KEYBOARD_PAGE,     0x45},      // VK_F12,              0x45;
    {HID_KEYBOARD_PAGE,     0x51},      // VK_ARROW_D,          0x51;
    {HID_KEYBOARD_PAGE,     0x4f},      // VK_ARROW_R,          0x4f;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SK_UNDERLINE,
    {HID_KEYBOARD_PAGE,     0x50},      // VK_ARROW_L,          0x50;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_NEXTTRACK,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_W_REFRESH,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_CAL,
    {HID_SPECIAL_KEYBOARD_PAGE,      0x01},      // VK_CTRL_L,           0x01;
    {HID_KEYBOARD_PAGE,     0x35},      // VK_TILDE,            0x35;
    {HID_KEYBOARD_PAGE,     0x3a},      // VK_F1,               0x3a;
    {HID_KEYBOARD_PAGE,     0x3b},      // VK_F2,               0x3b;
    {HID_KEYBOARD_PAGE,     0x22},      // VK_5,                0x22;
    {HID_KEYBOARD_PAGE,     0x23},      // VK_6,                0x23;
    {HID_KEYBOARD_PAGE,     0x2e},      // VK_PLUS,             0x2e;
    {HID_KEYBOARD_PAGE,     0x41},      // VK_F8,               0x41;
    {HID_KEYBOARD_PAGE,     0x2d},      // VK_UNDERLINE,        0x2d;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_00,
    {HID_KEYBOARD_PAGE,     0x42},      // VK_F9,               0x42;
    {HID_KEYBOARD_PAGE,     0x4c},      // VK_DEL,              0x4c;
    {HID_SPECIAL_KEYBOARD_PAGE,      0x10},      // VK_CTRL_R,           0x10;
#if FEAUTRE_SUPPORT_VENDOR_SHORTCUT_KEY
    {HID_VENDOR_PAGE,       0x0201},      // VK_PAGE_UP,          0x4b;
    {HID_VENDOR_PAGE,       0x0101},      // VK_HOME,             0x4a;
#else
    {HID_KEYBOARD_PAGE,     0x4b},      // VK_PAGE_UP,          0x4b;
    {HID_KEYBOARD_PAGE,     0x4a},      // VK_HOME,             0x4a;
#endif
    {HID_KEYBOARD_PAGE,     0x00},      // VK_PREVTRACK,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_MUTE,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_W_WEB,
    {HID_KEYBOARD_PAGE,     0x3e},      // VK_F5,               0x3e;
    {HID_KEYBOARD_PAGE,     0x1e},      // VK_1,                0x1e;
    {HID_KEYBOARD_PAGE,     0x1f},      // VK_2,                0x1f;
    {HID_KEYBOARD_PAGE,     0x20},      // VK_3,                0x20;
    {HID_KEYBOARD_PAGE,     0x21},      // VK_4,                0x21;
    {HID_KEYBOARD_PAGE,     0x24},      // VK_7,                0x24;
    {HID_KEYBOARD_PAGE,     0x25},      // VK_8,                0x25;
    {HID_KEYBOARD_PAGE,     0x26},      // VK_9,                0x26;
    {HID_KEYBOARD_PAGE,     0x27},      // VK_0,                0x27;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_PRINT,
    {HID_KEYBOARD_PAGE,     0x43},      // VK_F10,              0x43;
    {HID_KEYBOARD_PAGE,     0x00},      // VK_POWER_R,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_SLEEP_R,
#if FEAUTRE_SUPPORT_VENDOR_SHORTCUT_KEY
    {HID_VENDOR_PAGE,       0x0301},      // VK_PAGE_DOWN,        0x4e;
    {HID_VENDOR_PAGE,       0x1001},      // VK_END,              0x4d;
#else
    {HID_KEYBOARD_PAGE,       0x4e},      // VK_PAGE_DOWN,        0x4e;
    {HID_KEYBOARD_PAGE,       0x4d},      // VK_END,              0x4d;
#endif
    {HID_KEYBOARD_PAGE,     0x00},      // VK_MEDIA,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_W_SEARCH,
    {HID_KEYBOARD_PAGE,     0x00},      // VK_K151,
    // KEY_INDEX_ENUM_GUAID,
};

/*============================================================================*
 *                              Function Declartion
 *============================================================================*/
bool keyboard_app_notify_data(T_KEYBOARD_DATA keyboard_data);
static uint8_t key_handle_get_hid_usage_page_by_index(T_KEY_INDEX_DEF key_index);
static uint16_t key_handle_get_hid_usage_id_by_index(T_KEY_INDEX_DEF key_index);
static bool key_handle_detect_fn_comb_keys(T_KEYSCAN_FIFO_DATA *p_keyscan_fifo_data);
static bool key_handle_is_key_index_in_fn_array(T_KEY_INDEX_DEF key_index,
                                                const T_FN_KEY_CODE_DEF *fn_code_table);
static bool key_handle_is_key_index_in_fifo(T_KEY_INDEX_DEF key_index,
                                            T_KEYSCAN_FIFO_DATA *p_keyscan_fifo_data);
/*============================================================================*
 *                              Global Variables
 *============================================================================*/
T_KEY_HANDLE_GLOBAL_DATA key_handle_global_data;  /* Value to indicate the reconnection key data */
struct k_timer combine_keys_detection_timer;
struct k_timer notify_key_data_after_reconn_timer;
struct k_timer typematic_timer;
const struct device *const kscan_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_keyboard_scan));

static void kb_callback(const struct device *dev, uint32_t row, uint32_t col,
			bool pressed)
{
    T_IO_MSG io_msg;
    io_msg.type = IO_MSG_TYPE_KEYSCAN;

	ARG_UNUSED(dev);
	DBG_DIRECT("Key row,col = [%d,%d] Pressed = %d\n", row, col, pressed);

	if (pressed) {
        keyscan_fifo_push(row,col);
        io_msg.u.buf = (void *)(&keyscan_global_data.cur_fifo_data);
        key_handle_notify_pressed_event(&keyscan_global_data.cur_fifo_data);
        DBG_DIRECT("[notify_key_data_timer_cb] send IO_MSG_KEYSCAN_RX_PKT start");
        if (false == app_send_msg_to_apptask(&io_msg))
        {
            DBG_DIRECT("[notify_key_data_timer_cb] send IO_MSG_KEYSCAN_RX_PKT failed");
        }
        DBG_DIRECT("[notify_key_data_timer_cb] send IO_MSG_KEYSCAN_RX_PKT end");
		// k_timer_start(&typematic_timer,
		// 	      K_MSEC(100), K_NO_WAIT);
	} else {
        keyscan_fifo_pop(row,col);
        if(keyscan_global_data.cur_fifo_data.len == 0)
        {
            key_handle_notify_all_release_event(&keyscan_global_data.cur_fifo_data);
        }
        else
        {
            key_handle_notify_pressed_event(&keyscan_global_data.cur_fifo_data);
        }
		// k_timer_stop(&typematic_timer);
	}
}

void key_handle_init_config(void)
{
    DBG_DIRECT("keys handle init config");
    if (!device_is_ready(kscan_dev)) {
		LOG_ERR("kscan device %s not ready", kscan_dev->name);
		return;
	}

	kscan_config(kscan_dev, kb_callback);
    kscan_enable_callback(kscan_dev);
}
/******************************************************************
 * @brief    Initialize key handle data
 * @param    none
 * @return   none
 * @retval   void
 */
void key_handle_init_data(void)
{
    DBG_DIRECT("key_handle_init_data");
    memset(&key_handle_global_data, 0, sizeof(key_handle_global_data));
}

void key_handle_comb_keys_timer_cb(struct k_timer timer)
{
    return;
}
void notify_key_data_timer_cb(struct k_timer timer)
{
    return;
}
void handle_key_data_timer_cb(struct k_timer timer)
{
    return;
}
/******************************************************************
 * @brief    key handler init timer
 * @param    none
 * @return   none
 * @retval   void
 */
void key_handle_init_timer(void)
{
    DBG_DIRECT("key_handle_init_timer");
    k_timer_init(&combine_keys_detection_timer,key_handle_comb_keys_timer_cb,NULL);
    k_timer_init(&notify_key_data_after_reconn_timer,notify_key_data_timer_cb,NULL);
    k_timer_init(&typematic_timer,handle_key_data_timer_cb,NULL);
}

/******************************************************************
 * @brief  handle key prepare hid usage buffer
 * @param  p_cur_buf - point of current hid usage buffer
 * @param  p_prev_buf - point of previous hid usage buffer
 * @return bool - ture or false
 * @retval void
 */
static bool key_handle_prepare_hid_usage_buffer(T_KEY_INDEX_DEF key_index,
                                                T_KEY_HID_USAGES_BUFFER *p_cur_buf)
{
    bool result = false;
    uint8_t buffer_index = 0;
    uint8_t hid_usage_page = key_handle_get_hid_usage_page_by_index(key_index);
    uint16_t hid_usage_id = key_handle_get_hid_usage_id_by_index(key_index);

    if (HID_KEYBOARD_PAGE == hid_usage_page)
    {
        if (p_cur_buf->keyboard_usage_cnt < (KEYBOARD_DATA_SIZE - 1))
        {
            DBG_DIRECT("[key_handle_prepare_hid_usage_buffer]:prepare data, keyboard cnt = %d",p_cur_buf->keyboard_usage_cnt);
            buffer_index = (p_cur_buf->keyboard_usage_cnt) % (KEYBOARD_DATA_SIZE - 1);
            p_cur_buf->keyboard_usage_buffer[buffer_index] = (uint8_t)hid_usage_id;
            p_cur_buf->keyboard_usage_cnt += 1;
            result = true;
        }
        else
        {
            DBG_DIRECT("[key_handle_prepare_hid_usage_buffer] keyboard_usage_buffer is full");
            result = false;
        }
    }
    else if (HID_SPECIAL_KEYBOARD_PAGE == hid_usage_page)
    {
        p_cur_buf->special_kb_value |= (uint8_t)hid_usage_id;
        result = true;
    }
// #if FEAUTRE_SUPPORT_VENDOR_SHORTCUT_KEY
//     else if (HID_VENDOR_PAGE == hid_usage_page)
//     {
//         p_buf->keyboard_vendor_buffer = hid_usage_id;
//         result = true;
//     }
// #endif
    else if (HID_UNDEFINED_PAGE == hid_usage_id)
    {
        result = true;
    }
    return result;
}
/******************************************************************
 * @brief  handle key notify hid usage buffer
 * @param  p_cur_buf - point of current hid usage buffer
 * @param  p_prev_buf - point of previous hid usage buffer
 * @return bool - ture or false
 * @retval void
 */
static bool key_handle_notify_hid_usage_buffer(T_KEY_HID_USAGES_BUFFER *p_cur_buf,
                                               T_KEY_HID_USAGES_BUFFER *p_prev_buf)
{
    DBG_DIRECT("[key_handle_notify_hid_usage_buffer] enter");
    bool result = false;

    /* check parameters and status */
    if ((p_cur_buf == NULL) || (p_prev_buf == NULL))
    {
        DBG_DIRECT("[key_handle_notify_hid_usage_buffer] Invalid parameters");
        return false;
    }
// #if FEAUTRE_SUPPORT_VENDOR_SHORTCUT_KEY
//     if (p_cur_buf->keyboard_vendor_buffer != p_prev_buf->keyboard_vendor_buffer)
//     {
//         app_global_data.keyboard_data.keyboard_vendor_data[0] = (uint8_t)(
//                                                                     p_cur_buf->keyboard_vendor_buffer);
//         app_global_data.keyboard_data.keyboard_vendor_data[3] = (uint8_t)(
//                                                                     p_cur_buf->keyboard_vendor_buffer >> 8);

//         app_global_data.keyboard_data.hid_report_type = HID_REPORT_VENDOR;
//         result = keyboard_app_notify_data(app_global_data.keyboard_data);
//     }
//     else
// #endif
        if (0 != memcmp(p_cur_buf->keyboard_usage_buffer, p_prev_buf->keyboard_usage_buffer,
                        sizeof(p_cur_buf->keyboard_usage_buffer)) ||
            p_cur_buf->special_kb_value != p_prev_buf->special_kb_value)
        {
            app_global_data.keyboard_data.keyboard_common_data[0] = p_cur_buf->special_kb_value;
            for (int i = 0; i < (KEYBOARD_DATA_SIZE - 1); i++)
            {
                DBG_DIRECT("key_handle_notify_hid_usage_buffer:common buffer:%d",p_cur_buf->keyboard_usage_buffer[i]);
                app_global_data.keyboard_data.keyboard_common_data[i + 1] = p_cur_buf->keyboard_usage_buffer[i];
            }
            app_global_data.keyboard_data.hid_report_type = KEYBOARD_REPORT_ID;

            result = keyboard_app_notify_data(app_global_data.keyboard_data);
            DBG_DIRECT("keyboard_app_notify_data: %d(1:true.0:false)",result);
        }

        else if (0 != memcmp(p_cur_buf->keyboard_consumer_usage_buffer, p_prev_buf->keyboard_consumer_usage_buffer,
                    sizeof(p_cur_buf->keyboard_usage_buffer)))
        {
            DBG_DIRECT("key_handle_notify_hid_usage_buffer:consumer cur buffer:%d, %d",p_cur_buf->keyboard_consumer_usage_buffer[0],
                        p_cur_buf->keyboard_consumer_usage_buffer[1]);
            DBG_DIRECT("key_handle_notify_hid_usage_buffer:consumer pre buffer:%d, %d",p_prev_buf->keyboard_consumer_usage_buffer[0],
                        p_prev_buf->keyboard_consumer_usage_buffer[1]);
            memcpy(&app_global_data.keyboard_data.keyboard_consumer_data,
                   &p_cur_buf->keyboard_consumer_usage_buffer, CONSUMER_DATA_SIZE);
            app_global_data.keyboard_data.hid_report_type = CONSUMER_REPORT_ID;
            result = keyboard_app_notify_data(app_global_data.keyboard_data);
        }
    return result;
}
/**
  * @brief keyboard_app_notify_data
  * @param[in] T_KEYBOARD_DATA
  * @retval bool
  */
bool keyboard_app_notify_data(T_KEYBOARD_DATA keyboard_data)
{
    bool result = false;
    if(keyboard_data.hid_report_type == KEYBOARD_REPORT_ID)
    {
        result = app_usb_send_keyboard_data(keyboard_data); 
    }
    if(keyboard_data.hid_report_type == CONSUMER_REPORT_ID)
    {
        result = app_usb_send_consumer_data(keyboard_data); 
    }
    if(keyboard_data.hid_report_type == VENDOR_REPORT_ID)
    {
        result = app_usb_send_vendor_data(keyboard_data); 
    }
  
    return result;
}
/******************************************************************
 * @brief    key handle notify pressed event
 * @param    void
 * @return   bool - true of false
 * @retval   none
 */
void key_handle_notify_pressed_event(T_KEYSCAN_FIFO_DATA *keyscan_fifo_data)
{
    bool result = false;
    if(true == key_handle_detect_fn_comb_keys(keyscan_fifo_data))
    {
        DBG_DIRECT("key_handle_detect_fn_comb_keys is true");
        result = true;
        return result;
    }
    T_KEY_INDEX_DEF key_index;
    memset(&key_handle_global_data.current_hid_usage_buf, 0, sizeof(T_KEY_HID_USAGES_BUFFER));
    for(uint8_t fifo_len=0; fifo_len < keyscan_fifo_data->len; fifo_len++)
    {
        key_index = KEY_MAPPING_TABLE[keyscan_fifo_data->key[fifo_len].row][keyscan_fifo_data->key[fifo_len].column];
        DBG_DIRECT("key_handle_notify_pressed_event: KEY INDEX = %d",key_index);
        if (true == key_handle_prepare_hid_usage_buffer(key_index,
                                                &key_handle_global_data.current_hid_usage_buf))
        {
            /* get invalid hid usage */
            result = true;
        }
    }
    if (true == result)
    {
    if (true == key_handle_notify_hid_usage_buffer(&key_handle_global_data.current_hid_usage_buf,
                                                &key_handle_global_data.prev_hid_usage_buf))
        {
            memcpy(&key_handle_global_data.prev_hid_usage_buf, &key_handle_global_data.current_hid_usage_buf,
                   sizeof(T_KEY_HID_USAGES_BUFFER));
            result = true;
        }
        else
        {
            result = false;
        }
    }
    return result;
}
/******************************************************************
 * @brief    key handle notify release event
 * @param    void
 * @return   bool - true of false
 * @retval   none
 */
void key_handle_notify_all_release_event(T_KEYSCAN_FIFO_DATA *keyscan_fifo_data)
{
    DBG_DIRECT("key_handle_notify_all_release_event");
    bool result = false;
    memset(&key_handle_global_data.current_hid_usage_buf, 0, sizeof(T_KEY_HID_USAGES_BUFFER));
    if (true == key_handle_notify_hid_usage_buffer(&key_handle_global_data.current_hid_usage_buf,
                                                   &key_handle_global_data.prev_hid_usage_buf))
    {
        memcpy(&key_handle_global_data.prev_hid_usage_buf, &key_handle_global_data.current_hid_usage_buf,
               sizeof(T_KEY_HID_USAGES_BUFFER));
        result = true;
    }
    return result;
}
/******************************************************************
 * @brief    Get HID usage page by key index
 * @param    key_index - key index
 * @return   uint8_t - hid usage page
 * @retval   void
 */
static uint8_t key_handle_get_hid_usage_page_by_index(T_KEY_INDEX_DEF key_index)
{
    return KEY_CODE_TABLE[key_index].hid_usage_page;
}

/******************************************************************
 * @brief    Get HID usage id by key index
 * @param    key_index - key index
 * @return   uint16_t - hid usage id
 * @retval   void
 */
static uint16_t key_handle_get_hid_usage_id_by_index(T_KEY_INDEX_DEF key_index)
{
    return KEY_CODE_TABLE[key_index].hid_usage_id;
}
/******************************************************************
 * @brief  key handle check specific key index is in keyscan FIFO or not
 * @param  key_index - key index
 * @param  p_keyscan_fifo_data - the point of keyscan fifo data
 * @return bool - true or false
 * @retval void
 */
static bool key_handle_is_key_index_in_fifo(T_KEY_INDEX_DEF key_index,
                                            T_KEYSCAN_FIFO_DATA *p_keyscan_fifo_data)
{
    for (uint32_t loop_index = 0; loop_index < p_keyscan_fifo_data->len; loop_index++)
    {
        DBG_DIRECT("key_handle_is_key_index_in_fifo:key index is %d !",KEY_MAPPING_TABLE[p_keyscan_fifo_data->key[loop_index].row][p_keyscan_fifo_data->key[loop_index].column]);
        if (key_index ==
            KEY_MAPPING_TABLE[p_keyscan_fifo_data->key[loop_index].row][p_keyscan_fifo_data->key[loop_index].column])
        {
            return true;
        }
    }
    return false;
}
/******************************************************************
 * @brief  key_handle_detect_fn_comb_keys
 * @param
 * @return
 * @retval
 */
static bool key_handle_detect_fn_comb_keys(T_KEYSCAN_FIFO_DATA *p_keyscan_fifo_data)
{
    bool result = false;
    if ((p_keyscan_fifo_data->len == 2) &&
        key_handle_is_key_index_in_fifo(VK_FN, p_keyscan_fifo_data) &&
        key_handle_is_key_index_in_fifo(VK_ESC, p_keyscan_fifo_data))
    {
        if (fn_lock == false)
        {
            DBG_DIRECT("key_handle_detect_fn_comb_keys:lock fn !");
            fn_lock = true;
        }
        else
        {
            DBG_DIRECT("key_handle_detect_fn_comb_keys:unlock fn !");
            fn_lock = false;
        }
    }
    T_KEY_INDEX_DEF key_index = 0;
    DBG_DIRECT("key_handle_detect_fn_comb_keys:fifo len=%d !",p_keyscan_fifo_data->len);
    if (fn_lock == false)
    {
        memset(&key_handle_global_data.current_hid_usage_buf, 0, sizeof(T_KEY_HID_USAGES_BUFFER));
        if ((p_keyscan_fifo_data->len == 2) && key_handle_is_key_index_in_fifo(VK_FN, p_keyscan_fifo_data))
        {
            DBG_DIRECT("key_handle_detect_fn_comb_keys:detect fn !");
            for (uint8_t i = 0; i < 2; i++)
            {
                if (VK_FN != KEY_MAPPING_TABLE[p_keyscan_fifo_data->key[i].row][p_keyscan_fifo_data->key[i].column])
                {
                    key_index = KEY_MAPPING_TABLE[p_keyscan_fifo_data->key[i].row][p_keyscan_fifo_data->key[i].column];
                    DBG_DIRECT("key_handle_detect_fn_comb_keys:key index = %d !",key_index);
                    break;
                }
            }
            if (false == key_handle_is_key_index_in_fn_array(key_index, fn_code_table))
            {
                DBG_DIRECT("[key_handle_detect_fn_comb_keys] do not have fn comb key rightly");
                return false;
            }
            DBG_DIRECT("key_handle_detect_fn_comb_keys:detect fn comb key !");
        }
        else
        {
            DBG_DIRECT("[key_handle_detect_fn_comb_keys] not fn comb key");
            return false;
        }
    }
    else     /*decect key in fn lock status*/
    {
        memset(&key_handle_global_data.current_hid_usage_buf, 0, sizeof(T_KEY_HID_USAGES_BUFFER));

        if (p_keyscan_fifo_data->len == 1)
        {
            key_index = KEY_MAPPING_TABLE[p_keyscan_fifo_data->key[0].row][p_keyscan_fifo_data->key[0].column];

            if (false == key_handle_is_key_index_in_fn_array(key_index, fn_code_table))
            {
                DBG_DIRECT("[key_handle_detect_fn_comb_keys] do not have fn comb key rightly");
                return false;
            }
        }
        else
        {
            DBG_DIRECT("[key_handle_detect_fn_comb_keys] not fn comb key");
            return false;
        }
    }

    if (true == key_handle_notify_hid_usage_buffer(&key_handle_global_data.current_hid_usage_buf,
                                                   &key_handle_global_data.prev_hid_usage_buf))
    {
        memcpy(&key_handle_global_data.prev_hid_usage_buf, &key_handle_global_data.current_hid_usage_buf,
               sizeof(T_KEY_HID_USAGES_BUFFER));
        result = true;
    }

    return result;
}
/******************************************************************
 * @brief  find the value of the fn key combination
 * @param  key_index - key index
 * @param  fn_code_table - the point of fn_code_table
 * @return bool - true or false
 */
static bool key_handle_is_key_index_in_fn_array(T_KEY_INDEX_DEF key_index,
                                                const T_FN_KEY_CODE_DEF *fn_code_table)
{
    for (uint8_t i = 0; i <= FN_CODE_NUM; i++)
    {
        if (key_index == fn_code_table[i].type)
        {
            if (fn_code_table[i].hid_usage_page == HID_CONSUMER_PAGE)
            {
                key_handle_global_data.current_hid_usage_buf.keyboard_consumer_usage_buffer[0] =
                    (uint8_t)fn_code_table[i].keyboard_fn_data_id;
                key_handle_global_data.current_hid_usage_buf.keyboard_consumer_usage_buffer[1] = (uint8_t)(
                            fn_code_table[i].keyboard_fn_data_id >> 8);
            }
            else if (fn_code_table[i].hid_usage_page == HID_KEYBOARD_PAGE)
            {
                key_handle_global_data.current_hid_usage_buf.special_kb_value = (uint8_t)
                                                                                fn_code_table[i].keyboard_fn_data_id;
                key_handle_global_data.current_hid_usage_buf.keyboard_usage_buffer[0] = (uint8_t)(
                                                                                            fn_code_table[i].keyboard_fn_data_id >> 8);
            }
            else
            {
                return false;
            }
            return true;
        }
    }
    return false;
}