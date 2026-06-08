#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/irq.h"
#include "tusb.h"

#define USB_VID      0x3689
#define USB_PID      0x8762
#define USB_BCD      0x0200

const uint8_t FIXED_ID[12] = {
    0x75, 0x1A, 0x47, 0x8F, 0xC3, 0xE4,
    0xFF, 0x66, 0x79, 0xE9, 0x6F, 0x18
};

typedef struct {
    uint8_t seed[4];
    uint8_t response[4];
} cr_pair_t;

const cr_pair_t cr_table[] = {
    {{0xE5, 0xC6, 0x2D, 0xF0}, {0x8B, 0x1A, 0x3C, 0x81}},
    {{0x46, 0xA5, 0x3F, 0x4D}, {0xB8, 0x4A, 0x18, 0x77}},
    {{0x9E, 0xE6, 0x10, 0xB6}, {0x4C, 0xAE, 0x54, 0x98}},
    {{0x25, 0x0E, 0x30, 0x14}, {0x72, 0xD7, 0x6F, 0x12}},
    {{0x69, 0xDE, 0xD8, 0x1D}, {0xAB, 0x29, 0xF5, 0x36}},
    {{0x96, 0x95, 0xBF, 0xC1}, {0xF6, 0x2E, 0x04, 0x45}},
    {{0xC1, 0xD4, 0x2D, 0x8D}, {0x8E, 0xFD, 0x00, 0x74}},
    {{0xAA, 0x81, 0xAE, 0x15}, {0xA6, 0xDE, 0x62, 0x16}},
};
#define CR_TABLE_SIZE (sizeof(cr_table) / sizeof(cr_table[0]))

const uint8_t CMD03_RESPONSE[4] = {0x5F, 0x9B, 0xFF, 0x90};
const uint8_t CMD10_SEED[8] = {0x76, 0xCF, 0x7C, 0xEB, 0x75, 0x1A, 0x47, 0x8F};
const uint8_t CMD10_RESPONSE[4] = {0x53, 0xFF, 0x7C, 0xEB};

void build_response(uint8_t *buf, const uint8_t *first4) {
    buf[0] = first4[0];
    buf[1] = first4[1];
    buf[2] = first4[2];
    buf[3] = first4[3];
    memcpy(buf + 4, FIXED_ID, 12);
    memset(buf + 16, 0, 5);
}

bool find_cr04(const uint8_t *seed, uint8_t *response) {
    for (size_t i = 0; i < CR_TABLE_SIZE; i++) {
        if (memcmp(seed, cr_table[i].seed, 4) == 0) {
            memcpy(response, cr_table[i].response, 4);
            return true;
        }
    }
    return false;
}

int main() {
    stdio_init_all();
    tusb_init();
    while (1) {
        tud_task();
    }
    return 0;
}

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
    0x12,
    0x01,
    0x00, 0x02,
    0x00,
    0x00,
    0x00,
    0x08,
    USB_VID & 0xFF, USB_VID >> 8,
    USB_PID & 0xFF, USB_PID >> 8,
    0x00, 0x02,
    0x01,
    0x01,
    0x00,
    0x01
};

uint8_t const desc_configuration[] = {
    0x09,
    0x02,
    0x22, 0x00,
    0x01,
    0x01,
    0x01,
    0x80,
    0x19,

    0x09,
    0x04,
    0x00,
    0x00,
    0x01,
    0x03,
    0x00,
    0x00,
    0x00,

    0x09,
    0x21,
    0x11, 0x01,
    0x00,
    0x01,
    0x22,
    sizeof(desc_hid_report) & 0xFF, sizeof(desc_hid_report) >> 8,

    0x07,
    0x05,
    0x81,
    0x03,
    0x08, 0x00,
    0x0A
};

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

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const* buffer, uint16_t bufsize) {
    (void)itf;
    (void)report_type;

    if (report_id != 2 || bufsize < 21) return;

    uint8_t response[21];
    uint8_t cmd = buffer[1];
    bool valid = false;
    uint8_t first4[4];

    switch (cmd) {
        case 0x03:
            memcpy(first4, CMD03_RESPONSE, 4);
            valid = true;
            break;

        case 0x04:
            if (find_cr04(buffer + 2, first4)) {
                valid = true;
            }
            break;

        case 0x10:
            if (memcmp(buffer + 5, CMD10_SEED, 8) == 0) {
                memcpy(first4, CMD10_RESPONSE, 4);
                valid = true;
            }
            break;
    }

    if (valid) {
        build_response(response, first4);
        tud_hid_report(1, response, 21);
    }
}
