#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "tusb.h"

#define LED_PIN 25

enum { BLINK_NOT_MOUNTED=250, BLINK_MOUNTED=1000, BLINK_SUSPENDED=2500 };
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

uint32_t millis(void) { return to_ms_since_boot(get_absolute_time()); }

void led_blinking_task(void) {
    static uint32_t start_ms = 0;
    static bool led_state = false;
    if (!blink_interval_ms) return;
    if (millis()-start_ms < blink_interval_ms) return;
    start_ms += blink_interval_ms;
    gpio_put(LED_PIN, led_state);
    led_state = !led_state;
}

int main(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    tusb_init();
    while (1) { tud_task(); led_blinking_task(); }
    return 0;
}

void tud_mount_cb(void)   { blink_interval_ms = BLINK_MOUNTED; }
void tud_umount_cb(void)  { blink_interval_ms = BLINK_NOT_MOUNTED; }
void tud_suspend_cb(bool r) { (void)r; blink_interval_ms = BLINK_SUSPENDED; }
void tud_resume_cb(void)  { blink_interval_ms = BLINK_MOUNTED; }

/* ---- Standard HID Keyboard Report Descriptor ---- */
uint8_t const desc_hid_report[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xa1, 0x01,        // Collection (Application)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x05, 0x07,        //   Usage Page (Keyboard/Keypad)
    0x19, 0xe0,        //   Usage Minimum (224)
    0x29, 0xe7,        //   Usage Maximum (231)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x03,        //   Input (Const,Var,Abs)
    0x95, 0x05,        //   Report Count (5)
    0x75, 0x01,        //   Report Size (1)
    0x05, 0x08,        //   Usage Page (LEDs)
    0x19, 0x01,        //   Usage Minimum (1)
    0x29, 0x05,        //   Usage Maximum (5)
    0x91, 0x02,        //   Output (Data,Var,Abs)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x03,        //   Report Size (3)
    0x91, 0x03,        //   Output (Const,Var,Abs)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x65,        //   Logical Maximum (101)
    0x05, 0x07,        //   Usage Page (Keyboard/Keypad)
    0x19, 0x00,        //   Usage Minimum (0)
    0x29, 0x65,        //   Usage Maximum (101)
    0x81, 0x00,        //   Input (Data,Array)
    0xc0               // End Collection
};

/* ---- Standard Keyboard: 8-byte boot report ---- */
const char* string_desc_arr[] = {
    (const char[]){0x09,0x04},
    "Pico Test",
};

uint8_t const desc_device[] = {
    0x12,0x01,0x00,0x02,0x00,0x00,0x00,0x08,
    0xCA,0xFE,   // VID 0xFECA (generic test)
    0x0D,0xF0,   // PID 0xF00D
    0x00,0x01,0x01,0x02,0x00,0x01
};

uint8_t const desc_configuration[] = {
    0x09,0x02,0x22,0x00,0x01,0x01,0x01,0x80,0x19,
    0x09,0x04,0x00,0x00,0x01,0x03,0x01,0x01,0x00,
    0x09,0x21,0x11,0x01,0x00,0x01,0x22,
    sizeof(desc_hid_report)&0xFF, sizeof(desc_hid_report)>>8,
    0x07,0x05,0x81,0x03,0x08,0x00,0x0A
};

uint8_t const *tud_descriptor_device_cb(void) { return desc_device; }
uint8_t const *tud_descriptor_configuration_cb(uint8_t i) { (void)i; return desc_configuration; }
uint8_t const *tud_hid_descriptor_report_cb(uint8_t i) { (void)i; return desc_hid_report; }

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    static uint16_t s[32];
    if(index==0){ s[0]=0x0304; return (uint16_t*)s; }
    const char* str=string_desc_arr[index];
    uint8_t l=strlen(str); if(l>31)l=31;
    for(uint8_t i=0;i<l;i++) s[i+1]=str[i];
    s[0]=(l<<8)|0x03;
    return s;
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t rid, hid_report_type_t rt, uint8_t* buf, uint16_t rl) {
    (void)itf;(void)rid;(void)rt;(void)buf;(void)rl;
    return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t rid, hid_report_type_t rt, uint8_t const* buf, uint16_t len) {
    (void)itf;(void)rid;(void)rt;(void)buf;(void)len;
}
