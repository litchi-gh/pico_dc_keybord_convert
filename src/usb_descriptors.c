#include "tusb.h"

enum {
  ITF_NUM_HID,
  ITF_NUM_TOTAL
};

#define USB_VID 0xCAFE
#define USB_PID 0x4001
#define EPNUM_HID 0x81
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

static tusb_desc_device_t const device_descriptor = {
  .bLength = sizeof(tusb_desc_device_t),
  .bDescriptorType = TUSB_DESC_DEVICE,
  .bcdUSB = 0x0200,
  .bDeviceClass = 0x00,
  .bDeviceSubClass = 0x00,
  .bDeviceProtocol = 0x00,
  .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
  .idVendor = USB_VID,
  .idProduct = USB_PID,
  .bcdDevice = 0x0100,
  .iManufacturer = 0x01,
  .iProduct = 0x02,
  .iSerialNumber = 0x03,
  .bNumConfigurations = 0x01
};

uint8_t const *tud_descriptor_device_cb(void) {
  return (uint8_t const *)&device_descriptor;
}

static uint8_t const hid_report_descriptor[] = {
  TUD_HID_REPORT_DESC_KEYBOARD()
};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
  (void)instance;
  return hid_report_descriptor;
}

static uint8_t const configuration_descriptor[] = {
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
                        TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
  TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_KEYBOARD,
                     sizeof(hid_report_descriptor), EPNUM_HID, 16, 1)
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
  (void)index;
  return configuration_descriptor;
}

static char const *string_descriptors[] = {
  (const char[]){0x09, 0x04},
  "DIY Maple Adapter",
  "Dreamcast HKT-4000 Keyboard",
  "0001"
};

static uint16_t string_descriptor[32];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void)langid;
  uint8_t count;

  if (index == 0) {
    string_descriptor[1] = 0x0409;
    count = 1;
  } else {
    if (index >= sizeof(string_descriptors) / sizeof(string_descriptors[0])) {
      return NULL;
    }
    const char *str = string_descriptors[index];
    count = 0;
    while (str[count] && count < 31) {
      string_descriptor[1 + count] = str[count];
      count++;
    }
  }

  string_descriptor[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * count + 2));
  return string_descriptor;
}

