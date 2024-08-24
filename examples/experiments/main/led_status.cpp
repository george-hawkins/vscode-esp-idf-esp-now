#include "led_indicator.h"
#include "led_status.hpp"

#define GPIO_LED_PIN       GPIO_NUM_8
#define GPIO_ACTIVE_LEVEL  false

static led_indicator_handle_t led_handle = NULL;

/**
 * @brief Blinking twice times has a priority level of 0 (highest).
 *
 */
static const blink_step_t double_blink[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_STOP, 0, 0},
};

/**
 * @brief Blinking three times has a priority level of 1.
 *
 */
static const blink_step_t triple_blink[] = {
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_STOP, 0, 0},
};

/**
 * @brief Brightness set to 75% with a priority level of 2.
 *
 */
static const blink_step_t bright_75_percent[] = {
    {LED_BLINK_BRIGHTNESS, LED_STATE_75_PERCENT, 0},
    {LED_BLINK_STOP, 0, 0},
};

/**
 * @brief Brightness set to 25% with a priority level of 3.
 *
 */
static const blink_step_t bright_25_percent[] = {
    {LED_BLINK_BRIGHTNESS, LED_STATE_25_PERCENT, 0},
    {LED_BLINK_STOP, 0, 0},
};

/**
 * @brief Slow breathing with a priority level of 4.
 *
 */
static const blink_step_t breath_slow_blink[] = {
    {LED_BLINK_HOLD, LED_STATE_OFF, 0},
    {LED_BLINK_BREATHE, LED_STATE_ON, 1000},
    {LED_BLINK_BREATHE, LED_STATE_OFF, 1000},
    {LED_BLINK_LOOP, 0, 0},
};

/**
 * @brief Fast breathing with a priority level of 5(lowest).
 *
 */
static const blink_step_t breath_fast_blink[] = {
    {LED_BLINK_HOLD, LED_STATE_OFF, 0},
    {LED_BLINK_BREATHE, LED_STATE_ON, 500},
    {LED_BLINK_BREATHE, LED_STATE_OFF, 500},
    {LED_BLINK_LOOP, 0, 0},
};

blink_step_t const *led_mode[] = {
    [BLINK_DOUBLE] = double_blink,
    [BLINK_TRIPLE] = triple_blink,
    [BLINK_BRIGHT_75_PERCENT] = bright_75_percent,
    [BLINK_BRIGHT_25_PERCENT] = bright_25_percent,
    [BLINK_BREATHE_SLOW] = breath_slow_blink,
    [BLINK_BREATHE_FAST] = breath_fast_blink,
    [BLINK_MAX] = NULL,
};

void app_led_init() {
    led_indicator_ledc_config_t ledc_config = {
        .is_active_level_high = GPIO_ACTIVE_LEVEL,
        .timer_inited = false,
        .timer_num = LEDC_TIMER_0,
        .gpio_num = GPIO_LED_PIN,
        .channel = LEDC_CHANNEL_0,
    };

    const led_indicator_config_t config = {
        .mode = LED_LEDC_MODE,
        .led_indicator_ledc_config = &ledc_config,
        .blink_lists = led_mode,
        .blink_list_num = BLINK_MAX,
    };

    led_handle = led_indicator_create(&config);
    assert(led_handle != NULL);

}

void set_led_mode(int blink_type) {
    led_indicator_start(led_handle, blink_type);
}