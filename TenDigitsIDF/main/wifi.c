
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <esp_wifi.h>
#include "esp_log.h"

#include "globals.h"


uint8_t my_ip[4];

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    // see https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/wifi.html#ip-event-sta-got-ip

    if (event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(LOG_TAG, "Wifi connecting ...");
    }
    else if (event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(LOG_TAG, "WiFi connected");
    }
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        static int retry_num = 0;

        ESP_LOGW(LOG_TAG, "WiFi lost connection");
        if (retry_num < 5)
        {
            esp_wifi_connect();
            retry_num++;
            ESP_LOGI(LOG_TAG, "WiFi Retrying connect ...");
        }
    }
    else if (event_id == IP_EVENT_STA_GOT_IP)
    {
        // see https://esp32.com/viewtopic.php?t=19156

        // ip_event_got_ip_t
        ESP_LOGI(LOG_TAG, "Wifi got IP!");
        ip_event_got_ip_t *p = (ip_event_got_ip_t*) event_data;

        ESP_LOGI(LOG_TAG, "Wifi got IP      " IPSTR, IP2STR(&(p->ip_info.ip)));
        ESP_LOGI(LOG_TAG, "Wifi got netmask " IPSTR, IP2STR(&(p->ip_info.netmask)));
        ESP_LOGI(LOG_TAG, "Wifi got gateway " IPSTR, IP2STR(&(p->ip_info.gw)));

        my_ip[0] = esp_ip4_addr1(&(p->ip_info.ip));
        my_ip[1] = esp_ip4_addr2(&(p->ip_info.ip));
        my_ip[2] = esp_ip4_addr3(&(p->ip_info.ip));
        my_ip[3] = esp_ip4_addr4(&(p->ip_info.ip));
    }
}


void wifi_connection()
{
    // network interdace initialization
    ESP_LOGI(LOG_TAG, "Initializing interface");
    esp_netif_init();

    // responsible for handling and dispatching events
    ESP_LOGI(LOG_TAG, "Creating event loop");
    esp_event_loop_create_default();

    // sets up necessary data structs for wifi station interface
    ESP_LOGI(LOG_TAG, "Creating WiFi station");
    esp_netif_create_default_wifi_sta();

    // sets up wifi wifi_init_config struct with default values and initializes it
    ESP_LOGI(LOG_TAG, "Initializing WiFi");
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);

    // register event handlers
    ESP_LOGI(LOG_TAG, "Registering WiFi event handler");
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);

    ESP_LOGI(LOG_TAG, "Registering IP event handler");
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

    ESP_LOGI(LOG_TAG, "Setting configuration for ssid %s", ssid);
    wifi_config_t wifi_configuration =
    {
        .sta= {
            .ssid = "",
            .password= "" // these members are char[32], so we can copy into them next
        }
        // also this part is used if you donot want to use Kconfig.projbuild
    };
    strcpy((char*)wifi_configuration.sta.ssid, ssid);
    strcpy((char*)wifi_configuration.sta.password, pass);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);//setting up configs when event ESP_IF_WIFI_STA

    ESP_LOGI(LOG_TAG, "Starting WiFi");
    esp_wifi_start();   //start connection with configurations provided in funtion

    ESP_LOGI(LOG_TAG, "Setting WiFi to Station mode");
    esp_wifi_set_mode(WIFI_MODE_STA);//station mode selected

    ESP_LOGI(LOG_TAG, "Connecting WiFi");
    esp_wifi_connect();

    ESP_LOGI(LOG_TAG, "WiFi setup completed");
}
