#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif

// 1. Define the RP2040 MCU and Pico OS
#define CFG_TUSB_MCU               OPT_MCU_RP2040
#define CFG_TUSB_OS                OPT_OS_PICO

// 2. Set the USB Port to Device Mode (This fixes your error!)
#define CFG_TUSB_RHPORT0_MODE      OPT_MODE_DEVICE

// 3. Enable the Device Stack and HID (Mouse) Class
#define CFG_TUD_ENABLED            1
#define CFG_TUD_HID                1

// 4. Set standard endpoint buffer sizes
#define CFG_TUD_ENDPOINT0_SIZE     64
#define CFG_TUD_HID_EP_BUFSIZE     64

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_CONFIG_H_ */