// https://medium.com/@fatehsali517/how-to-connect-esp32-to-wifi-using-esp-idf-iot-development-framework-d798dc89f0d6

// https://developer.espressif.com/blog/getting-started-with-wifi-on-esp-idf/

// https://developer.espressif.com/blog/getting-started-with-wifi-on-esp-idf/

// https://stackoverflow.com/questions/78808438/esp-idf-wifi-always-fails-to-connect-the-first-time

#include <stdio.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_task_wdt.h"

#include "driver/gpio.h"
#include "hal/gpio_ll.h"

#include <esp_http_server.h>
#include <esp_event.h>
#include <esp_log.h>

#include "globals.h"

#include "display.h"
#include "nvs.h"
#include "wifi.h"
#include "web.h"

// wifi structure:
//      https://developer.espressif.com/blog/getting-started-with-wifi-on-esp-idf/

const char* LOG_TAG = "TenDigits";

const char *pass = "girlmoose";
const char *ssid = "blaszczak2g";





/*
    Prints a sign on message about the satus of the device
*/
static void start_up_message(void)
{
    // Print chip information
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    ESP_LOGI(LOG_TAG, "This is %s chip with %d CPU core(s), %s%s%s%s",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    ESP_LOGI(LOG_TAG, "silicon revision v%d.%d", major_rev, minor_rev);

    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
    {
        ESP_LOGW(LOG_TAG, "Get flash size failed");
    }
    else
    {
        ESP_LOGI(LOG_TAG, "%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
               (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    }

    ESP_LOGI(LOG_TAG, "Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
}


/*
    toggles the state of the built-in LED
*/
static /* IRAM_ATTR */ void flash_builtin_callback(void* arg)
{
    static bool builtin_state = true;

    builtin_state = !builtin_state;
    gpio_set_level(LED_BUILTIN, builtin_state);

    if (++dpPlace > 9)
        dpPlace = 0;
}


/*
    Sets up a periodic timer that will flash the builtin LED
*/
static void start_builtin_timer(void)
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &flash_builtin_callback,
        .name = "builtin",
        .dispatch_method = ESP_TIMER_ISR
    };
    
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    // value in microseconds
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 500333));

    ESP_LOGI(LOG_TAG, "Started timer for builtin flash, time since boot: %lld us", esp_timer_get_time());
}


/*
    Increments our count, then breaks it into individual digits
    for display by the scanner.
*/
static /* IRAM_ATTR */ void counter_callback(void* arg)
{
    if (++counter > 9999999999)
    {
        int64_t time_since_boot = esp_timer_get_time();
        ESP_LOGI(LOG_TAG, "Periodic timer: time since boot: %lld us, counter wrapped", time_since_boot);
        counter = 0;
    }

    counterDigits[9] = counter % 10;
    counterDigits[8] = (counter / 10) % 10;
    counterDigits[7] = (counter / 100) % 10;
    counterDigits[6] = (counter / 1000) % 10;
    counterDigits[5] = (counter / 10000) % 10;
    counterDigits[4] = (counter / 100000) % 10;
    counterDigits[3] = (counter / 1000000) % 10;
    counterDigits[2] = (counter / 10000000) % 10;
    counterDigits[1] = (counter / 100000000) % 10;
    counterDigits[0] = (counter / 1000000000) % 10;

    counter_to_segments();
}


/*
    Sets up a periodic timer that counts, then builds the digits array
*/
static void start_count_timer(void)
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &counter_callback,
        .name = "counter",
        .dispatch_method = ESP_TIMER_ISR
    };
    
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    // value in microseconds
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 15000));

    ESP_LOGI(LOG_TAG, "Started timer for counter, time since boot: %lld us", esp_timer_get_time());
}


void scan_task(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (true)
    {
        static uint8_t working_digit = 0;

        // turn segments off because switching isn't atomic
        writeSegments(0);

        // turn on this digit
        activateDecoderChannel(working_digit);

        // and write its segments
        // writeSegments(digitToSegments[counterDigits[working_digit]]);
        writeSegments(segmentValues[working_digit]);

        // manage the DP
        // gpio_set_level(SEGMENT_DP, (working_digit == dpPlace) ? LOW : HIGH);

        // and think about the next digit
        if (++working_digit > 9)
        {
            working_digit = 0;
        }

        // vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
        // vTaskDelayUntil(&xLastWakeTime, 1);
        vTaskDelay(pdMS_TO_TICKS(5));
        // vTaskDelay(1);
        taskYIELD();
        // esp_task_wdt_reset();
    }
}


static void start_scan_task(void)
{
    // this disables the watchdog timer, which gives errors
    // because the idle task on the same CPU as the scan task won't run enough
    esp_task_wdt_delete(xTaskGetIdleTaskHandleForCPU(APP_CPU_NUM));

    xTaskCreatePinnedToCore(
        scan_task,      // Task function
        "ScanTask",     // Task name
        4096,           // Stack size in bytes
        NULL,           // Parameters to pass to the task
        tskIDLE_PRIORITY + 2,              // Task priority
        NULL,           // Task handle (not needed for this example)
        APP_CPU_NUM     // Pin to APP_CPU (Core 1)
    );
}


/*
    Scan the digits. This places the content of counterDigits[] into each
    digit by translating the byte value into segments, and poking the segments.
    Each call does one digit, working thruogh the nine.
*/
static /* IRAM_ATTR */ void scan_callback(void* arg)
{
    static uint8_t working_digit = 0;

    // turn segments off because switching isn't atomic
    // writeSegments(0);

    // turn on this digit
    activateDecoderChannel(working_digit);

    // and write its segments
    // writeSegments(digitToSegments[counterDigits[working_digit]]);
    writeSegments(segmentValues[working_digit]);

    // manage the DP
    setDP((working_digit == dpPlace) ? LOW : HIGH);

    // and think about the next digit
    if (++working_digit > 9)
        working_digit = 0;

    // ESP_LOGI(LOG_TAG, "workingDigit %d, counter %d, digit is %d", workingDigit, counter, counterDigits[workingDigit]);
}


/*
    Sets up a periodic timer that will flash the builtin LED
*/
static void start_scan_timer(void)
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &scan_callback,
        .name = "digitscan",
        .dispatch_method = ESP_TIMER_ISR
    };
    
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    // value in microseconds
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000));

    ESP_LOGI(LOG_TAG, "Started timer for digit scan, time since boot: %lld us", esp_timer_get_time());
}



/*
    here's our main entry point.

    Get the I/O initialized, set a timer, and that's that.
    All the work is in the timer handler.
*/
void app_main(void)
{
    ESP_LOGI(LOG_TAG, "**************");
    ESP_LOGI(LOG_TAG, " TenDigitsIDF");
    ESP_LOGI(LOG_TAG, "**************");
    start_up_message();

    setup_pins();

    initialize_nvs();

    wifi_connection();

    start_webserver();

    start_builtin_timer();

    // start_scan_timer();

    start_scan_task();

    // start_count_timer();

    ESP_LOGI(LOG_TAG, "main is done, time since boot: %lld us", esp_timer_get_time());
}


