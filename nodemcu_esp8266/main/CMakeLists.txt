idf_component_register(
    SRCS
        "app_main.c"
        "bmp180.c"
        "dht11.c"
        "mq135.c"
        "rain.c"
        "thingspeak.c"
        "wifi.c"
    INCLUDE_DIRS "."
    REQUIRES esp_http_client esp8266 esp_event nvs_flash tcpip_adapter pthread
)