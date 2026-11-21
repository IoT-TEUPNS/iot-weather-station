
#include "weather_station.h"

#include "app_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "weather";

void app_main(void)
{
    ESP_LOGI(TAG, "IoT weather station - ESP8266");
    if (rain_init() != ESP_OK || dht11_init() != ESP_OK ||
        mq135_init() != ESP_OK) {
        ESP_LOGE(TAG, "GPIO/ADC initialization failed");
        return;
    }
    if (bmp180_init() != ESP_OK) {
        ESP_LOGW(TAG, "BMP180 unavailable; pressure will be zero");
    }
    if (weather_wifi_init() != ESP_OK) {
        ESP_LOGW(TAG, "Wi-Fi unavailable; HTTP sending disabled");
    }
    TickType_t last_read = xTaskGetTickCount();
    for (;;) {
        weather_readings_t readings = {0};
        readings.rain_level = rain_read_and_control();
        if ((xTaskGetTickCount() - last_read) >=
            pdMS_TO_TICKS(WEATHER_INTERVAL_MS)) {
            last_read = xTaskGetTickCount();
            esp_err_t dht_err = dht11_read(&readings.humidity, &readings.temperature);
            esp_err_t mq_err = mq135_read(&readings.air_quality_ppm);
            double bmp_temperature = 0.0;
            esp_err_t bmp_err = bmp180_read(&bmp_temperature,
                                            &readings.pressure_absolute,
                                            &readings.pressure_relative);
            if (bmp_err == ESP_OK && dht_err != ESP_OK) {
                readings.temperature = (float)bmp_temperature;
            }
            if (dht_err != ESP_OK) ESP_LOGW(TAG, "DHT11: %s", esp_err_to_name(dht_err));
            if (mq_err != ESP_OK) ESP_LOGW(TAG, "MQ-135: %s", esp_err_to_name(mq_err));
            if (bmp_err != ESP_OK) ESP_LOGW(TAG, "BMP180: %s", esp_err_to_name(bmp_err));
            ESP_LOGI(TAG,
                     "humidity=%d.%d%% temp=%d.%dC abs=%ld.%02ldhPa "
                     "rel=%ld.%02ldhPa air=%d.%dppm rain=%d",
                     (int)readings.humidity,
                     (int)(readings.humidity * 10) % 10,
                     (int)readings.temperature,
                     (int)(readings.temperature * 10) % 10,
                     (long)readings.pressure_absolute,
                     (long)((readings.pressure_absolute - (int)readings.pressure_absolute) * 100),
                     (long)readings.pressure_relative,
                     (long)((readings.pressure_relative - (int)readings.pressure_relative) * 100),
                     (int)readings.air_quality_ppm,
                     (int)(readings.air_quality_ppm * 10) % 10,
                     readings.rain_level);
            if (weather_wifi_is_connected()) {
                thingspeak_send(&readings);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}