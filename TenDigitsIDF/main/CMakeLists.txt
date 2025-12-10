idf_component_register(SRCS "display.c" "web.c" "wifi.c" "nvs.c" "main.c"
                    PRIV_REQUIRES nvs_flash spi_flash esp_driver_gpio esp_timer esp_wifi esp_http_server
                    INCLUDE_DIRS "."
)