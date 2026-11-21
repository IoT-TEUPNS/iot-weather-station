#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "driver/gpio.h"

/* NodeMCU ESP8266 mapping: D1=SCL, D2=SDA, D4=GPIO2, D5=GPIO14,
 * D6=GPIO12, D7=GPIO13 and A0 is the ADC input. Confirm this against
 * the physical wiring before powering the kit. */
#define DHT11_GPIO          GPIO_NUM_2
#define BMP180_SCL_GPIO     GPIO_NUM_5
#define BMP180_SDA_GPIO     GPIO_NUM_4
#define RAIN_GPIO           GPIO_NUM_14
#define RAIN_LED_GPIO       GPIO_NUM_12
#define RAIN_BUZZER_GPIO    GPIO_NUM_13

#define WEATHER_ALTITUDE_M  400.5
#define WEATHER_INTERVAL_MS 15000

#define I2C_CLOCK_STRETCH_TICKS 200
#define BMP180_I2C_ADDRESS 0x77

#define MQ135_ADC_MAX       1023.0f
/* NodeMCU boards commonly scale A0 to the ADC's 1.0 V input range.
 * Calibrate this value for the exact board and MQ-135 module. */
#define MQ135_ADC_FULL_SCALE_V 1.0f
#define MQ135_LOAD_RESISTOR_KOHM 10.0f
#define MQ135_R0_KOHM 10.55f
#define MQ135_CURVE_M (-0.3376f)
#define MQ135_CURVE_B 0.7165f

#define THINGSPEAK_URL "http://api.thingspeak.com/update"

#endif
