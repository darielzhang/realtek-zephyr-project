/*
 * Copyright (c) 2024 Realtek Semiconductor 
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef _APP_TASK_H_
#define _APP_TASK_H_

#include <zephyr/kernel.h>
#include "key_handle.h"

typedef enum
{
    NO_TRANS_MODE = 0x00,
    PPT_2_4G = 0x01,
    BLE_MODE = 0x02,
    USB_MODE = 0x03,
    MP_TEST_MODE = 0x04,
} T_MODE_TYPE;

typedef enum
{
    HID_REPORT_KEYBOARD = 0x00,
    HID_REPORT_CONSUMER = 0x01,
    HID_REPORT_VENDOR = 0x02,
} T_KEYBOARD_REPORT_TYPE;

typedef struct
{
    T_KEYBOARD_REPORT_TYPE hid_report_type;
    uint8_t keyboard_common_data[KEYBOARD_DATA_SIZE];       /*common key, send by HID_REPORT_KEYBOARD */
    uint8_t keyboard_consumer_data[CONSUMER_DATA_SIZE];        /*fn combine key, send by HID_REPORT_CONSUMER*/
    uint8_t keyboard_vendor_data[VENDOR_DATA_SIZE];      /*vendor key, send by HID_REPORT_VENDOR */
} T_KEYBOARD_DATA;

typedef struct t_app_global_data
{
    bool is_usb_enumeration_success;
    bool is_usb_working;  /* to indicate whether usb translation is normal working or not */
    bool is_ble_link_key_existed;  /* to indicate whether link key is existed or not */
    uint8_t direct_adv_cnt;  /* to indicate the count of high duty adv */
    uint8_t updt_conn_params_retry_cnt;  /* to indicate the update conneciton parameters retry count */
    uint8_t keyboard_con_id;  /* to indicate the conntion id */
    uint8_t mtu_size;  /* to indicate the current mtu size */
    uint8_t pair_failed_retry_cnt;  /* to indicate the retry cnt for pair failed scenario */
    // T_SERVER_ID hid_srv_id;  /* to indicate the service id for HID */
    // T_SERVER_ID bas_srv_id;  /* to indicate the service id for BAS */
    // T_SERVER_ID dis_srv_id;  /* to indicate the service id for DIS */
    // T_GAP_DEV_STATE gap_dev_state;  /* to indicate the current GAP device state */
    // T_GAP_CONN_STATE gap_conn_state;  /* to indicate the current GAP connection state */
    // T_KEYBOARD_BLE_STATUS
    // keyboard_ble_status;  /* to indicate the current status of KEYBOARD state machine */
    T_MODE_TYPE mode_type; /* to indicate the current mode */
    // T_CHARGE_MODE_TYPE charge_mode;  /* to indicate the current charging mode */
    // T_ADV_TYPE adv_type;  /* to indicate the current advertising type */
    // T_STOP_ADV_REASON stop_adv_reason;  /* to indicate the reason of stop advertising */
    // T_DISCONN_REASON disconn_reason;  /* to indicate the reason of disconnec  */
    // T_BATTERY_STATUS battery_status;  /* to indicate the status of battery information */
    // T_latency_status latency_status;  /* to indicate KEYBOARD app latency status */
    uint16_t conn_interval;  /* to indicate KEYBOARD connect interval */
    uint16_t conn_latency;  /* to indicate KEYBOARD connect latency */
    uint16_t conn_supervision_timeout; /* to indicate KEYBOARD supervision time out */
// #if (FEATURE_SUPPORT_REMOVE_LINK_KEY_BEFORE_PAIRING && FEATURE_SUPPORT_RECOVER_PAIR_INFO)
//     uint16_t bond_info_length;  /* to indicate the length of the bond information */
//     uint8_t *p_last_bond_info;  /* to indicate the last bond information */
// #endif
    T_KEYBOARD_DATA keyboard_data;
// #if (FEATURE_MAC_ADDR_TYPE == FEATURE_SUPPORT_SINGLE_LOCAL_STATIC_ADDR) || (FEATURE_MAC_ADDR_TYPE == FEATURE_SUPPORT_MULTIPLE_LOCAL_STATIC_ADDR)
//     uint8_t static_random_addr[6];  /* to indicate the current static random address */
// #endif
// #if (FEATURE_MAC_ADDR_TYPE == FEATURE_SUPPORT_SINGLE_LOCAL_STATIC_ADDR)
//     uint8_t last_static_random_addr[8];  /* to indicate the last static random address */
// #elif (FEATURE_MAC_ADDR_TYPE == FEATURE_SUPPORT_MULTIPLE_LOCAL_STATIC_ADDR)
//     uint32_t static_random_addr_index;  /* to indicate the current static random address index */
// #endif
    uint32_t usb_report_rate_index;
    uint32_t ppt_report_rate_index;
} T_APP_GLOBAL_DATA;

extern T_APP_GLOBAL_DATA app_global_data;

void app_global_data_init(void);

#endif