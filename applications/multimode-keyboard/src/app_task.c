/*
 * Copyright (c) 2024 Realtek Semiconductor 
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/arch/cpu.h>
#include "app_task.h"
#include "string.h"
#include "zephyr/logging/log.h"
#include "trace.h"
#include "app_msg.h"
#include "osif_zephyr.h"

LOG_MODULE_REGISTER(app_task, LOG_LEVEL_INF);

/** @defgroup  PERIPH_APP_TASK Peripheral App Task
    * @brief This file handles the implementation of application task related functions.
    *
    * Create App task and handle events & messages
    * @{
    */
/*============================================================================*
 *                              Macros
 *============================================================================*/
#define APP_TASK_PRIORITY             1         //!< Task priorities
#define APP_TASK_STACK_SIZE           512 * 4   //!<  Task stack size
#define MAX_NUMBER_OF_GAP_MESSAGE     0x20      //!<  GAP message queue size
#define MAX_NUMBER_OF_IO_MESSAGE      0x20      //!<  IO message queue size
#define MAX_NUMBER_OF_EVENT_MESSAGE   (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE)   //!< Event message queue size

void *app_task_handle;   //!< APP Task handle
void *evt_queue_handle;  //!< Event queue handle
void *io_queue_handle;   //!< IO queue handle

T_APP_GLOBAL_DATA app_global_data = {0};
struct k_msgq io_queue;
char io_queue_buf[MAX_NUMBER_OF_IO_MESSAGE*sizeof(T_IO_MSG)];
struct k_msgq evt_queue;
char evt_queue_buf[MAX_NUMBER_OF_GAP_MESSAGE*sizeof(T_IO_MSG)];
struct k_thread my_thread_data;

K_THREAD_STACK_DEFINE(my_stack_area, APP_TASK_STACK_SIZE);

static void app_main_task(void *,void *,void *);
bool app_send_msg_to_apptask(T_IO_MSG *p_msg);

void app_main_task(void *,void *, void *)
{
    DBG_DIRECT("app_main_task START");
    k_msgq_init(&io_queue, io_queue_buf, sizeof(T_IO_MSG),MAX_NUMBER_OF_IO_MESSAGE);
    k_msgq_init(&evt_queue, evt_queue_buf, sizeof(T_IO_MSG),MAX_NUMBER_OF_GAP_MESSAGE);
    uint8_t event;
 
    // k_msgq_init(&io_queue, io_queue_buf, sizeof(T_IO_MSG),MAX_NUMBER_OF_IO_MESSAGE);
    // k_msgq_init(&evt_queue, evt_queue_buf, sizeof(T_IO_MSG),MAX_NUMBER_OF_GAP_MESSAGE);

    if(app_global_data.mode_type == USB_MODE)
    {
        //usb_start();
    }
    while(true)
    {
        if (k_msgq_get(&evt_queue, &event, K_NO_WAIT) == 0)
        {
            if (event == EVENT_IO_TO_APP)
            {
                T_IO_MSG io_msg;
                if (k_msgq_get(&io_queue, &io_msg, K_NO_WAIT) == 0)
                {
                    app_handle_io_msg(io_msg);
                }
            }
            else
            {
                //gap_handle_msg(event);
            }
        }
    }
}

void app_task_init(void)
{
    k_thread_create(&my_thread_data, my_stack_area,
                    K_THREAD_STACK_SIZEOF(my_stack_area),
                    app_main_task,
                    NULL, NULL, NULL,
                    CONFIG_ZEPHYR_PRI_MAX-APP_TASK_PRIORITY, 0, K_NO_WAIT);
}
void app_global_data_init(void)
{
    DBG_DIRECT("app_global_data_init");
    memset(&app_global_data, 0, sizeof(T_APP_GLOBAL_DATA));
    app_global_data.mode_type = USB_MODE;
}
/******************************************************************
 * @brief  send message to app task
 * @param  p_msg - point to message
 * @retval void
 */
bool app_send_msg_to_apptask(T_IO_MSG *p_msg)
{
    uint8_t event = EVENT_IO_TO_APP;

    int ret = k_msgq_put(&io_queue,p_msg,K_NO_WAIT);
    if(ret != 0)
    {
        DBG_DIRECT("send io msg to app fail (err %d)",ret);
    }
    ret = k_msgq_put(&evt_queue, &event, K_NO_WAIT);
    if(ret != 0)
    {
        DBG_DIRECT("send event msg to app fail (err %d)",ret);
        return false;
    }

    return true;
}
void app_handle_io_msg(T_IO_MSG io_msg)
{
    if (app_global_data.mode_type == BLE_MODE)
    {
        //app_handle_ble_io_msg(io_msg);
    }
    else if (app_global_data.mode_type == PPT_2_4G)
    {
        //app_handle_ppt_io_msg(io_msg);
    }
    else if (app_global_data.mode_type == USB_MODE)
    {
        app_handle_usb_io_msg(io_msg);
    }
    else/*app in no trans mode*/
    {
       // app_handle_no_trans_io_msg(io_msg);
    }
}
void app_handle_usb_io_msg(T_IO_MSG io_msg)
{
    uint16_t msg_type = io_msg.type;
    switch (msg_type)
    {
        case IO_MSG_TYPE_KEYSCAN:
        {
            keyscan_msg_handle(io_msg);
            //to do
        }
        break;
        default:
        break;
    }
}
void keyscan_msg_handle(T_IO_MSG io_msg)
{
    DBG_DIRECT("keyscan_msg_handle");
}