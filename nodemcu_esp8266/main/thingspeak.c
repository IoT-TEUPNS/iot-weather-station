#include "weather_station.h"
#include "app_config.h"

#include <stdio.h>
#include <stdlib.h>
#include "esp_http_client.h"
#include "esp_log.h"

static const char *TAG = "thingspeak";

esp_err_t thingspeak_send(const weather_readings_t *readings)
{
    if (!readings) return ESP_ERR_INVALID_ARG;
    char humidity[16], temperature[16], pressure_relative[16];
    char pressure_absolute[16], air_quality[16];
    snprintf(humidity, sizeof(humidity), "%d.%02d",
             (int)readings->humidity,
             abs((int)(readings->humidity * 100) % 100));
    snprintf(temperature, sizeof(temperature), "%d.%02d",
             (int)readings->temperature,
             abs((int)(readings->temperature * 100) % 100));
    snprintf(pressure_relative, sizeof(pressure_relative), "%ld.%02ld",
             (long)readings->pressure_relative,
             labs((long)(readings->pressure_relative * 100) % 100));
    snprintf(pressure_absolute, sizeof(pressure_absolute), "%ld.%02ld",
             (long)readings->pressure_absolute,
             labs((long)(readings->pressure_absolute * 100) % 100));
    snprintf(air_quality, sizeof(air_quality), "%d.%02d",
             (int)readings->air_quality_ppm,
             abs((int)(readings->air_quality_ppm * 100) % 100));
    char body[256];
    int length = snprintf(body, sizeof(body),
        "api_key=%s&field1=%s&field2=%s&field3=%s&field4=%s&field5=%s&field6=%d",
        CONFIG_WEATHER_THINGSPEAK_API_KEY, humidity, temperature,
        pressure_relative, pressure_absolute, air_quality, readings->rain_level);
    if (length < 0 || (size_t)length >= sizeof(body)) return ESP_ERR_INVALID_SIZE;
    esp_http_client_config_t config = {
        .url = THINGSPEAK_URL,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 10000
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) return ESP_ERR_NO_MEM;
    esp_http_client_set_header(client, "Content-Type",
                               "application/x-www-form-urlencoded");
    esp_http_client_set_post_field(client, body, length);
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP status: %d", status);
        if (status != 200) err = ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    return err;
}