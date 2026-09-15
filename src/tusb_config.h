#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "pico/stdlib.h"

#ifndef CFG_TUSB_MCU
#define CFG_TUSB_MCU OPT_MCU_RP2040
#endif
#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS OPT_OS_NONE
#endif
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)

#define CFG_TUD_ENDPOINT0_SIZE 64
#define CFG_TUD_HID 1
#define CFG_TUD_HID_EP_BUFSIZE 16

#ifdef __cplusplus
}
#endif
