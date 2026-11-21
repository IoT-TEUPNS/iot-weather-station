#include "weather_station.h"
#include "app_config.h"

#include "driver/gpio.h"

esp_err_t rain_init(void)
{
    gpio_config_t input = {
        .pin_bit_mask = 1ULL << RAIN_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    esp_err_t err = gpio_config(&input);
    if (err != ESP_OK) return err;
    gpio_config_t output = {
        .pin_bit_mask = (1ULL << RAIN_LED_GPIO) | (1ULL << RAIN_BUZZER_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    err = gpio_config(&output);
    if (err == ESP_OK) {
        gpio_set_level(RAIN_LED_GPIO, 0);
        gpio_set_level(RAIN_BUZZER_GPIO, 0);
    }
    return err;
}

int rain_read_and_control(void)
{
    int level = gpio_get_level(RAIN_GPIO);
    int raining = level == 0;
    gpio_set_level(RAIN_LED_GPIO, raining);
    gpio_set_level(RAIN_BUZZER_GPIO, raining);
    return level;
}