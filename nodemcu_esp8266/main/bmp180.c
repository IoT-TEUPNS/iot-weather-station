#include "weather_station.h"
#include "app_config.h"

#include <math.h>
#include <stdint.h>
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "bmp180";
static int16_t ac1, ac2, ac3, b1, b2, mb, mc, md;
static uint16_t ac4, ac5, ac6;
static bool initialized;

static esp_err_t write_reg(uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) return ESP_ERR_NO_MEM;
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP180_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return err;
}

static esp_err_t read_regs(uint8_t reg, uint8_t *data, size_t length)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) return ESP_ERR_NO_MEM;
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP180_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BMP180_I2C_ADDRESS << 1) | I2C_MASTER_READ, true);
    if (length > 1) i2c_master_read(cmd, data, length - 1, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, data + length - 1, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return err;
}

static int16_t read_s16(uint8_t reg)
{
    uint8_t data[2];
    return read_regs(reg, data, 2) == ESP_OK ? (int16_t)((data[0] << 8) | data[1]) : 0;
}

static uint16_t read_u16(uint8_t reg)
{
    uint8_t data[2];
    return read_regs(reg, data, 2) == ESP_OK ? (uint16_t)((data[0] << 8) | data[1]) : 0;
}

esp_err_t bmp180_init(void)
{
    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = BMP180_SDA_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = BMP180_SCL_GPIO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .clk_stretch_tick = I2C_CLOCK_STRETCH_TICKS
    };
    esp_err_t err = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;
    err = i2c_param_config(I2C_NUM_0, &config);
    if (err != ESP_OK) return err;
    ac1 = read_s16(0xAA); ac2 = read_s16(0xAC); ac3 = read_s16(0xAE);
    ac4 = read_u16(0xB0); ac5 = read_u16(0xB2); ac6 = read_u16(0xB4);
    b1 = read_s16(0xB6); b2 = read_s16(0xB8); mb = read_s16(0xBA);
    mc = read_s16(0xBC); md = read_s16(0xBE);
    initialized = ac1 != 0 && ac4 != 0 && ac5 != 0 && ac6 != 0;
    if (!initialized) {
        ESP_LOGE(TAG, "calibration read failed");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t bmp180_read(double *temperature, double *pressure_absolute,
                      double *pressure_relative)
{
    if (!initialized || !temperature || !pressure_absolute || !pressure_relative) {
        return ESP_ERR_INVALID_STATE;
    }
    if (write_reg(0xF4, 0x2E) != ESP_OK) return ESP_FAIL;
    vTaskDelay(pdMS_TO_TICKS(5));
    int32_t ut = read_u16(0xF6);
    int32_t x1 = ((ut - ac6) * ac5) >> 15;
    int32_t x2 = (mc << 11) / (x1 + md);
    int32_t b5 = x1 + x2;
    *temperature = ((b5 + 8) >> 4) / 10.0;
    if (write_reg(0xF4, 0xF4) != ESP_OK) return ESP_FAIL;
    vTaskDelay(pdMS_TO_TICKS(26));
    uint8_t data[3];
    if (read_regs(0xF6, data, 3) != ESP_OK) return ESP_FAIL;
    int32_t up = ((data[0] << 16) | (data[1] << 8) | data[2]) >> 5;
    int32_t b6 = b5 - 4000;
    x1 = (b2 * (b6 * b6 >> 12)) >> 11;
    x2 = (ac2 * b6) >> 11;
    int32_t x3 = x1 + x2;
    int32_t b3 = (((int32_t)ac1 * 4 + x3 + 2) / 4);
    x1 = (ac3 * b6) >> 13;
    x2 = (b1 * (b6 * b6 >> 12)) >> 16;
    x3 = (x1 + x2 + 2) >> 2;
    uint32_t b4 = (ac4 * (uint32_t)(x3 + 32768)) >> 15;
    uint32_t b7 = ((uint32_t)up - b3) * 50000;
    int32_t pressure = b7 < 0x80000000U ? (b7 * 2) / b4 : (b7 / b4) * 2;
    x1 = (pressure >> 8) * (pressure >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * pressure) >> 16;
    pressure += (x1 + x2 + 3791) >> 4;
    *pressure_absolute = pressure / 100.0;
    *pressure_relative = *pressure_absolute *
                         pow(1.0 - WEATHER_ALTITUDE_M / 44330.0, -5.255);
    return ESP_OK;
}