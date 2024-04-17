/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2018 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */
#ifndef _KEYSCAN_DRIVER_H_
#define _KEYSCAN_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "zephyr/kernel.h"
/*============================================================================*
 *                         Macros
 *============================================================================*/

/*============================================================================*
 *                         Types
 *============================================================================*/
/**
 * @brief  KeyScan FIFO data struct definition.
 */
typedef struct
{
    uint32_t len;               /**< Keyscan state register        */
    struct
    {
        uint16_t column: 5;      /**< Keyscan raw buffer data       */
        uint16_t row: 4;         /**< Keyscan raw buffer data       */
        uint16_t reserved: 7;
    } key[8];
} T_KEYSCAN_FIFO_DATA;

/**
 * @brief  KeyScan global data struct definition.
 */
typedef struct
{
    bool is_allowed_to_repeat_report;  /* to indicate whether to allow to report repeat keyscan data event or not */
    bool is_allowed_to_enter_dlps;  /* to indicate whether to allow to enter dlps or not */
    bool is_all_key_released;  /* to indicate whether all keys are released or not */
    T_KEYSCAN_FIFO_DATA pre_fifo_data;  /* to indicate the previous keyscan FIFO data */
    T_KEYSCAN_FIFO_DATA cur_fifo_data;  /* to indicate the current keyscan FIFO data */

} T_KEYSCAN_GLOBAL_DATA;

/*============================================================================*
*                        Export Global Variables
*============================================================================*/
extern T_KEYSCAN_GLOBAL_DATA keyscan_global_data;

void keyscan_fifo_push(uint32_t row,uint32_t col);
void keyscan_fifo_pop(uint32_t row,uint32_t col);

#ifdef __cplusplus
}
#endif

#endif
