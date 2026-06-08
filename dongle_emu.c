#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "tusb.h"

#define USB_VID      0x3689
#define USB_PID      0x8762
#define USB_BCD      0x0200

int main() {
    tusb_init();
    while (1) {
        tud_task();
    }
    return 0;
}

/* ---- HID Report Descriptor ---- */
uint8_t const desc_hid_report[] = {
    0x06, 0x00, 0xFF,  // Usage Page (Vendor 0xFF00)
    0x09, 0x01,        // Usage (0x01)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1) - Device to Host
    0x09, 0x01,        //   Usage (0x01)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x14,        //   Report Count (20)
    0xB1, 0x06,        //   Feature (Data,Var,Rel)
    0x85, 0x02,        //   Report ID (2) - Host to Device
    0x09, 0x03,        //   Usage (0x03)
    0x96, 0x14, 0x00,  //   Report Count (20)
    0xB1, 0x06,        //   Feature (Data,Var,Rel)
    0xC0               // End Collection
};

/* ---- String Descriptors ---- */
const char* string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 },
    "USBKey",
};

/* ---- Device Descriptor ---- */
uint8_t const desc_device[] = {
    0x12,       // bLength
    0x01,       // bDescriptorType
    0x00, 0x02, // bcdUSB 2.0
    0x00,       // bDeviceClass
    0x00,       // bDeviceSubClass
    0x00,       // bDeviceProtocol
    0x08,       // bMaxPacketSize0
    USB_VID & 0xFF, USB_VID >> 8,
    USB_PID & 0xFF, USB_PID >> 8,
    0x00, 0x02, // bcdDevice
    0x01,       // iManufacturer
    0x01,       // iProduct
    0x00,       // iSerialNumber
    0x01        // bNumConfigurations
};

/* ---- Configuration Descriptor ---- */
uint8_t const desc_configuration[] = {
    0x09,       // bLength
    0x02,       // bDescriptorType
    0x22, 0x00, // wTotalLength
    0x01,       // bNumInterfaces
    0x01,       // bConfigurationValue
    0x01,       // iConfiguration
    0x80,       // bmAttributes
    0x19,       // bMaxPower

    0x09,       // bLength (Interface)
    0x04,       // bDescriptorType
    0x00,       // bInterfaceNumber
    0x00,       // bAlternateSetting
    0x01,       // bNumEndpoints
    0x03,       // bInterfaceClass (HID)
    0x00,       // bInterfaceSubClass
    0x00,       // bInterfaceProtocol
    0x00,       // iInterface

    0x09,       // bLength (HID)
    0x21,       // bDescriptorType
    0x11, 0x01, // bcdHID
    0x00,       // bCountryCode
    0x01,       // bNumDescriptors
    0x22,       // bDescriptorType (Report)
    sizeof(desc_hid_report) & 0xFF, sizeof(desc_hid_report) >> 8,

    0x07,       // bLength (Endpoint)
    0x05,       // bDescriptorType
    0x81,       // bEndpointAddress (IN)
    0x03,       // bmAttributes (Interrupt)
    0x08, 0x00, // wMaxPacketSize
    0x0A        // bInterval
};

/* ---- TinyUSB Callbacks ---- */
uint8_t const *tud_descriptor_device_cb(void) {
    return desc_device;
}

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_configuration;
}

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    static uint16_t str_buf[32];
    uint8_t len;

    if (index == 0) {
        str_buf[0] = 0x0304;
        return (uint16_t*)str_buf;
    }

    const char* str = string_desc_arr[index];
    len = strlen(str);
    if (len > 31) len = 31;
    for (uint8_t i = 0; i < len; i++) {
        str_buf[i + 1] = str[i];
    }
    str_buf[0] = (len << 8) | 0x03;
    return str_buf;
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf) {
    (void)itf;
    return desc_hid_report;
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
                                hid_report_type_t report_type,
                                uint8_t* buffer, uint16_t reqlen) {
    (void)itf;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const* buffer, uint16_t bufsize) {
    (void)itf;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}
