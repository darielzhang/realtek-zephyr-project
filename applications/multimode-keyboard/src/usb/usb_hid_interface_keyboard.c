/*
 * Copyright (c) 2024 Realtek Semiconductor 
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*============================================================================*
 *                             Includes
 *============================================================================*/
#include "usb_hid_interface_keyboard.h"
#include "trace.h"
#include "zephyr/kernel.h"
#include "zephyr/device.h"
#include "zephyr/logging/log.h"
#include <zephyr/usb/class/usb_hid.h>
#include <zephyr/usb/usb_device.h>
#include "board.h"

LOG_MODULE_REGISTER(usb, LOG_LEVEL_INF);

/*============================================================================*
 *                             Macros
 *============================================================================*/
#define KEYBOARD_MAX_TRANSMISSION_UNIT_SIZE     1+4 /* report id + data, uint: bytes */
#define KEYBOARD_MAX_PIPE_DATA_NUM              5

#define USB_INTERFACE_NUM       1
#define USB_EP_NUM              1
#define HID_INT_IN_EP_1         USB_DIR_IN | 0x02

/*============================================================================*
 *                             Local Variables
 *============================================================================*/
static enum usb_dc_status_code usb_status;

static const char report_descs[] =
{
    0x05, 0x01,     /* USAGE_PAGE       (Generic Desktop) */
    0x09, 0x06,     /* USAGE            (Keyboard) */
    0xa1, 0x01,     /* COLLECTION       (Application) */
    0x85, KEYBOARD_REPORT_ID,    /* REPORT_ID */
    0x75, 0x01,     /* REPORT_SIZE      (1) */
    0x95, 0x08,     /* REPORT_COUNT     (8) */
    0x15, 0x00,     /* LOGICAL_MINIMUM  (0) */
    0x25, 0x01,     /* LOGICAL_MAXIMUM  (1) */
    0x05, 0x07,     /* USAGE_PAGE       (Keyboard) */
    0x19, 0xe0,     /* USAGE_MINIMUM    (Keyboard Left Control) */
    0x29, 0xe7,     /* USAGE_MAXIMUM    (Keyboard Right GUI) */
    0x81, 0x02,     /* INPUT (Data,variable,Abs)*/
    0x75, 0x01,     /* REPORT_SIZE      (1) */
    0x95, 0x05,     /* REPORT_COUNT     (5) */
    0x05, 0x08,     /* USAGE_PAGE       (LEDs) */
    0x19, 0x01,     /* USAGE_MINIMUM    (Num Lock) */
    0x29, 0x05,     /* USAGE_MAXIMUM    (Kana) */
    0x91, 0x02,     /* OUTPUT           (Data,Var,Abs) */
    0x75, 0x03,     /* REPORT_SIZE      (3) */
    0x95, 0x01,     /* REPORT_COUNT     (1) */
    0x91, 0x03,     /* OUTPUT           (cons,Var,Abs) */
    0x75, 0x08,     /* REPORT_SIZE      (8) */
    0x95, 0x02,     /* REPORT_COUNT     (2) */
    0x15, 0x00,     /* LOGICAL_MINIMUM  (0) */
    0x26, 0xFF, 0x00,     /* LOGICAL_MAXIMUM  (255) */
    0x05, 0x07,     /* USAGE_PAGE       (Keyboard) */
    0x19, 0x00,     /* USAGE_MINIMUM    (Reserved-no event indicated) */
    0x29, 0xFF,     /* USAGE_MAXIMUM    (Keyboard Application) */
    0x81, 0x00,     /* INPUT            (Data,Ary,Abs) */
    0xc0,           /* END_COLLECTION */

    0x05, 0x0c,     /* USAGE_PAGE       (Consumer) */
    0x09, 0x01,     /* USAGE            (Consumer Control) */
    0xa1, 0x01,     /* COLLECTION       (Application) */
    0x85, CONSUMER_REPORT_ID,     /* REPORT_ID */
    0x75, 0x10,         /* REPORT_SIZE      (16) */
    0x95, 0x01,         /* REPORT_COUNT     (1) */
    0x15, 0x00,         /* LOGICAL_MINIMUM  (0) */
    0x26, 0x80, 0x03,   /* LOGICAL_MAXIMUM  (896) */
    0x19, 0x00,         /* USAGE_MINIMUM  */
    0x2a, 0x80, 0x03,   /* USAGE_MAXIMUM  */
    0x81, 0x00,     /* INPUT            (Data,Ary,Abs) */
    0xc0,           /* END_COLLECTION */

    0x06, 0x01, 0xff, /* USAGE_PAGE       (vendor) */
    0x09, 0x01,     /* USAGE            (vendor) */
    0xa1, 0x01,     /* COLLECTION       (Application) */
    0x85, VENDOR_REPORT_ID, /* REPORT_ID */
    0x75, 0x08,     /* REPORT_SIZE      (8) */
    0x95, 0x04,     /* REPORT_COUNT     (4) */
    0x15, 0x00,     /* LOGICAL_MINIMUM  (0) */
    0x26, 0xff, 0x00,     /* LOGICAL_MAXIMUM  (0xff) */
    0x19, 0x00,     /* USAGE_MINIMUM    (0) */
    0x29, 0xff,     /* USAGE_MAXIMUM    (0xff) */
    0x81, 0x02,     /* INPUT            (Data,Var,Abs) */
    0xc0,           /* END_COLLECTION */
};

/*============================================================================*
*                              Functions Declaration
*============================================================================*/
static int usb_hid_get_report(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data);
static int usb_hid_set_report(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data);
void usb_hid_keyboard_driver_init(void);

/*============================================================================*
*                              Private Funcitons
*============================================================================*/
static void app_usb_state_change_cb(enum usb_dc_status_code status, const uint8_t *param)
{
	usb_status = status;
    switch(status)
    {
        case USB_DC_ERROR:
        {
            LOG_INF("[USB status_cb] USB_DC_ERROR");
            break;
        }
        case USB_DC_RESET:
        {
            LOG_INF("[USB status_cb] USB_DC_RESET");
            break;
        }
        case USB_DC_CONNECTED:
        {
            LOG_INF("[USB status_cb] USB_DC_CONNECTED");
            break;
        }
        case USB_DC_CONFIGURED:
        {
            LOG_INF("[USB status_cb] USB_DC_CONFIGURED");
            break;
        }
        case USB_DC_DISCONNECTED:
        {
            LOG_INF("[USB status_cb] USB_DC_DISCONNECTED");
            break;
        }
        case USB_DC_SUSPEND:
        {
            LOG_INF("[USB status_cb] USB_DC_SUSPEND");
            break;
        }
        case USB_DC_RESUME:
        {
            LOG_INF("[USB status_cb] USB_DC_RESUME");
            break;
        }
        default:
        {
            LOG_INF("[USB status_cb] USB_DC_INTERFACE or USB_DC_HALT or USB_DC_SOF or unkonwn");
            break;
        }
    }
}

void usb_hid_keyboard_driver_init(void)
{
    LOG_INF("usb hid keyboard driver init");
    static const struct hid_ops ops = {
    .get_report = usb_hid_get_report,
    .set_report = usb_hid_set_report,
    };
    uint8_t ret = 0;
	const struct device *hid_dev;
	hid_dev = device_get_binding("RTK_HID_KEYBOARD_0"); 

    if (hid_dev == NULL) {
		LOG_ERR("Cannot get USB HID Device");
		return 0;
	}

	usb_hid_register_device(hid_dev,
				report_descs, sizeof(report_descs),
				&ops);

	usb_hid_init(hid_dev);

	ret = usb_enable(app_usb_state_change_cb);
	if (ret != 0) {
		LOG_ERR("Failed to enable USB");
		return 0;
	}
}

static int usb_hid_get_report(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data)
{
    LOG_INF("usb get report");
    return 0;
}
static int usb_hid_set_report(const struct device *dev, struct usb_setup_packet *setup, int32_t *len, uint8_t **data)
{
    LOG_INF("usb set report");
    return 0;
}