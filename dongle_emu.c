#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "tusb.h"

#define USB_VID      0x3689
#define USB_PID      0x8762
#define USB_BCD      0x0200

#define LED_PIN 25  // Pico's built-in LED

enum {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
uint32_t millis(void);

int main(void) {
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  tusb_init();

  while (1) {
    tud_task();
    led_blinking_task();
  }

  return 0;
}

uint32_t millis(void) {
  return to_ms_since_boot(get_absolute_time());
}

void tud_mount_cb(void) {
  blink_interval_ms = BLINK_MOUNTED;
}

void tud_umount_cb(void) {
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

void tud_suspend_cb(bool remote_wakeup_en) {
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

void tud_resume_cb(void) {
  blink_interval_ms = BLINK_MOUNTED;
}

/* ---- HID Report Descriptor ---- */
uint8_t const desc_hid_report[] = {
    0x06, 0x00, 0xFF,
    0x09, 0x01,
    0xA1, 0x01,
    0x85, 0x01,
    0x09, 0x01,
    0x15, 0x00,
    0x26, 0xFF, 0x00,
    0x75, 0x08,
    0x95, 0x14,
    0xB1, 0x06,
    0x85, 0x02,
    0x09, 0x03,
    0x96, 0x14, 0x00,
    0xB1, 0x06,
    0xC0
};

const char* string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 },
    "USBKey",
};

uint8_t const desc_device[] = {
    0x12, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x08,
    USB_VID & 0xFF, USB_VID >> 8,
    USB_PID & 0xFF, USB_PID >> 8,
    0x00, 0x02, 0x01, 0x01, 0x00, 0x01
};

uint8_t const desc_configuration[] = {
    0x09, 0x02, 0x22, 0x00, 0x01, 0x01, 0x01, 0x80, 0x19,
    0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00,
    0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22,
    sizeof(desc_hid_report) & 0xFF, sizeof(desc_hid_report) >> 8,
    0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x0A
};

uint8_t const *tud_descriptor_device_cb(void) { return desc_device; }
uint8_t const *tud_descriptor_configuration_cb(uint8_t i) { (void)i; return desc_configuration; }

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void)langid;
  static uint16_t str_buf[32];
  if (index == 0) { str_buf[0] = 0x0304; return (uint16_t*)str_buf; }
  const char* str = string_desc_arr[index];
  uint8_t len = strlen(str);
  if (len > 31) len = 31;
  for (uint8_t i = 0; i < len; i++) str_buf[i + 1] = str[i];
  str_buf[0] = (len << 8) | 0x03;
  return str_buf;
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf) { (void)itf; return desc_hid_report; }

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
                                hid_report_type_t report_type,
                                uint8_t* buffer, uint16_t reqlen) {
  (void)itf; (void)report_id; (void)report_type; (void)buffer; (void)reqlen;
  return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const* buffer, uint16_t bufsize) {
  (void)itf; (void)report_id; (void)report_type; (void)buffer; (void)bufsize;
}

void led_blinking_task(void) {
  static uint32_t start_ms = 0;
  static bool led_state = false;
  if (!blink_interval_ms) return;
  if (millis() - start_ms < blink_interval_ms) return;
  start_ms += blink_interval_ms;
  gpio_put(LED_PIN, led_state);
  led_state = !led_state;
}
