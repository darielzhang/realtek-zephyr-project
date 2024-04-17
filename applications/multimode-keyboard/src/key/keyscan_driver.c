/*
 * Copyright (c) 2024 Realtek Semiconductor 
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "zephyr/kernel.h"
#include "zephyr/device.h"
#include "zephyr/drivers/gpio.h"
#include "zephyr/drivers/kscan.h"
#include "zephyr/logging/log.h"

#include "board.h"
#include "key_handle.h"
#include "keyscan_driver.h"


LOG_MODULE_DECLARE(key, LOG_LEVEL_INF);

/*============================================================================*
 *                              Global Variables
 *============================================================================*/
T_KEYSCAN_GLOBAL_DATA keyscan_global_data;

void keyscan_fifo_push(uint32_t row,uint32_t col)
{
        keyscan_global_data.cur_fifo_data.key[keyscan_global_data.cur_fifo_data.len].row = row;
        keyscan_global_data.cur_fifo_data.key[keyscan_global_data.cur_fifo_data.len].column = col;
        keyscan_global_data.cur_fifo_data.len++;
}
void keyscan_fifo_pop(uint32_t row,uint32_t col)
{
    for(uint8_t i=0;i<keyscan_global_data.cur_fifo_data.len; i++)
    {
        if(keyscan_global_data.cur_fifo_data.key[i].row == row && keyscan_global_data.cur_fifo_data.key[i].column == col)
        for(uint8_t j = i; j < keyscan_global_data.cur_fifo_data.len-1; j++)
        {
            keyscan_global_data.cur_fifo_data.key[j] = keyscan_global_data.cur_fifo_data.key[j+1];
        }
    }
    keyscan_global_data.cur_fifo_data.len--;
}

