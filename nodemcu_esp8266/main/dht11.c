#include "weather_station.h"
#include "app_config.h"

#include <stdint.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"

static const char *TAG = "dht11";

esp_err_t dht11_init(void)
{
    gpio_config_t config = {
        .pin_bit_mask = 1ULL << DHT11_GPIO,
        .mode = GPIO_MODE_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    return gpio_config(&config);
}

static int wait_for_level(int level, uint32_t timeout_us)
{
    while (gpio_get_level(DHT11_GPIO) != level) {
        if (timeout_us-- == 0) return -1;
        ets_delay_us(1);
    }
    return 0;
}

esp_err_t dht11_read(float *humidity, float *temperature)
{
    if (!humidity || !temperature) return ESP_ERR_INVALID_ARG;
    uint8_t data[5] = {0};
    gpio_set_direction(DHT11_GPIO, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(DHT11_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(DHT11_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(DHT11_GPIO, 1);
    ets_delay_us(30);
    gpio_set_direction(DHT11_GPIO, GPIO_MODE_INPUT);

    if (wait_for_level(0, 100) < 0 || wait_for_level(1, 100) < 0 ||
        wait_for_level(0, 100) < 0) {
        gpio_set_direction(DHT11_GPIO, GPIO_MODE_OUTPUT_OD);
        ESP_LOGE(TAG, "sensor did not respond");
        return ESP_ERR_TIMEOUT;
    }
    for (int bit = 0; bit < 40; ++bit) {
        if (wait_for_level(1, 70) < 0) goto timeout;
        ets_delay_us(40);
        data[bit / 8] <<= 1;
        if (gpio_get_level(DHT11_GPIO)) data[bit / 8] |= 1;
        if (wait_for_level(0, 100) < 0) goto timeout;
    }
    gpio_set_direction(DHT11_GPIO, GPIO_MODE_OUTPUT_OD);
    if ((uint8_t)(data[0] + data[1] + data[2] + data[3]) != data[4]) {
        return ESP_ERR_INVALID_CRC;
    }
    *humidity = data[0] + data[1] / 10.0f;
    *temperature = data[2] + data[3] / 10.0f;
    return ESP_OK;

timeout:
    gpio_set_direction(DHT11_GPIO, GPIO_MODE_OUTPUT_OD);
    return ESP_ERR_TIMEOUT;
}