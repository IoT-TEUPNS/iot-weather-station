#include "weather_station.h"
#include "app_config.h"

#include <math.h>
#include "driver/adc.h"

esp_err_t mq135_init(void)
{
    adc_config_t adc_config = {
        .mode = ADC_READ_TOUT_MODE,
        .clk_div = 8
    };
    return adc_init(&adc_config);
}

esp_err_t mq135_read(float *ppm)
{
    if (!ppm) return ESP_ERR_INVALID_ARG;
    uint16_t raw = 0;
    esp_err_t err = adc_read(&raw);
    if (err != ESP_OK) return err;
    float voltage = raw * MQ135_ADC_FULL_SCALE_V / MQ135_ADC_MAX;
    if (voltage <= 0.001f || voltage >= MQ135_ADC_FULL_SCALE_V) {
        return ESP_ERR_INVALID_STATE;
    }
    float rs = (MQ135_LOAD_RESISTOR_KOHM * MQ135_ADC_FULL_SCALE_V / voltage) -
               MQ135_LOAD_RESISTOR_KOHM;
    float ratio = rs / MQ135_R0_KOHM;
    if (ratio <= 0.0f) return ESP_ERR_INVALID_STATE;
    *ppm = powf(10.0f, (log10f(ratio) - MQ135_CURVE_B) / MQ135_CURVE_M);
    return ESP_OK;
}