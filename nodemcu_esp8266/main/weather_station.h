#ifndef WEATHER_STATION_H
#define WEATHER_STATION_H

#include <stdbool.h>
#include "esp_err.h"

typedef struct {
    float humidity;
    float temperature;
    double pressure_absolute;
    double pressure_relative;
    float air_quality_ppm;
    int rain_level;
} weather_readings_t;

esp_err_t dht11_init(void);
esp_err_t dht11_read(float *humidity, float *temperature);

esp_err_t bmp180_init(void);
esp_err_t bmp180_read(double *temperature, double *pressure_absolute,
                      double *pressure_relative);

esp_err_t mq135_init(void);
esp_err_t mq135_read(float *ppm);

esp_err_t rain_init(void);
int rain_read_and_control(void);

esp_err_t weather_wifi_init(void);
bool weather_wifi_is_connected(void);

esp_err_t thingspeak_send(const weather_readings_t *readings);

#endif
