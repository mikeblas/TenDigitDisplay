
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include <nvs_flash.h>

#include "globals.h"

void initialize_nvs(void)
{
    ESP_LOGI(LOG_TAG, "initializing flash");
    esp_err_t ret = nvs_flash_init();
    ESP_LOGI(LOG_TAG, "flash init returned %d", ret);
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGI(LOG_TAG, "erasing flash");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
        ESP_LOGI(LOG_TAG, "flash erase returned %d", ret);
    }
    ESP_ERROR_CHECK(ret);
}
