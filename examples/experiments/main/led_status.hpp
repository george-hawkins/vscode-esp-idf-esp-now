#pragma once

enum {
    BLINK_DOUBLE = 0,
    BLINK_TRIPLE,
    BLINK_BRIGHT_75_PERCENT,
    BLINK_BRIGHT_25_PERCENT,
    BLINK_BREATHE_SLOW,
    BLINK_BREATHE_FAST,
    BLINK_MAX,
};

extern void app_led_init();

extern void set_led_mode(int blink_type);