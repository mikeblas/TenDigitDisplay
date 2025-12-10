

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"

#include <esp_http_server.h>
#include <esp_log.h>

#include "globals.h"
#include "display.h"

/*  */
// URI handler for the root path "/"
esp_err_t /* IRAM_ATTR */ root_get_handler(httpd_req_t *req)
{
    const char* resp_str = "Hello from ESP32 Web Server!";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    int digit = 0;
    bool carry = true;
    while (carry && digit < 10)
    {
        counterDigits[digit]++;
        if (counterDigits[digit] >= 10)
        {
            counterDigits[digit] = 0;
            carry = true;
        }
        else
        {
            carry = false;
        }
        digit++;
    }

    counter_to_segments();
    return ESP_OK;
}


static uint8_t hexdigit(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';

    if (ch >= 'a' && ch <= 'f')
        return 10 + ch - 'a';

    if (ch >= 'A' && ch <= 'F')
        return 10 + ch - 'A';

    return 0;
}


esp_err_t /* IRAM_ATTR */ segments_get_handler(httpd_req_t *req)
{
    char sz[100];
    int ret = snprintf(sz, sizeof(sz), "Got the segments! \"%s\"</br>\n", req->uri);
    if (ret < 0)
        return ESP_FAIL;

    char *qmark = strchr(req->uri, '?');
    if (qmark != NULL)
    {
        uint8_t charIndex = 1;
        bool over = false;

        for (uint8_t digit = 0; digit < 10; digit++)
        {
            // if we find the end of URL string, we're over
            if (!over && (qmark[charIndex] == 0 || qmark[charIndex+1] == 0))
                over = true;

            // if we're not done, eat two characters and carry on
            if (!over)
            {
                // convert ASCII hex to binary, that's the segment pattern
                segmentValues[digit] = (hexdigit(qmark[charIndex]) << 4) + hexdigit(qmark[charIndex+1]);
                charIndex += 2;
            }
            else
            {
                // set the remaining digits blank
                segmentValues[digit] = 0;
            }
        }
    }

    /*
    char second[100];
    sprintf(second, "%2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x</br>\n",
        segmentValues[0], segmentValues[1], segmentValues[2], segmentValues[3], 
        segmentValues[4], segmentValues[5], segmentValues[6], segmentValues[7], 
        segmentValues[8], segmentValues[9]);
    strcat(sz, second);
    */

    httpd_resp_send(req, sz, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}


esp_err_t /* IRAM_ATTR */ digits_get_handler(httpd_req_t *req)
{
    char sz[100];
    int ret = snprintf(sz, sizeof(sz), "Got the digits! \"%s\"", req->uri);
    if (ret < 0)
        return ESP_FAIL;
    
    char *qmark = strchr(req->uri, '?');
    bool over = false;
    if (qmark != NULL)
    {
        int digit = 0;
        int character = 1;
        while (digit < 10)
        {
            if (!over && qmark[character] == 0)
                over = true;

            if (over)
            {  
                // ran out of string characters, everything else is just blank
                counterDigits[digit] = 0;
                digit++;
            }
            else
            {
                counterDigits[digit] = qmark[character] - '0';
                // is the next charact er a dot?
                if (qmark[character+1] == '.')
                {
                    // yes, eat it and set the DP on this digit
                    character++;
                    // counterDigits[digit] |= SEGMENT_DP;
                }

                digit++;
            }

            character++;
        }

        counter_to_segments();
    }

    httpd_resp_send(req, sz, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}


esp_err_t /* IRAM_ATTR */ digits2_get_handler(httpd_req_t *req)
{
    char sz[100];
    int ret = snprintf(sz, sizeof(sz), "Got the digits2! \"%s\"", req->uri);
    if (ret < 0)
        return ESP_FAIL;
    
    char *qmark = strchr(req->uri, '?');
    bool over = false;
    if (qmark != NULL)
    {
        int digit = 0;
        int character = 1;
        while (digit < 10)
        {
            if (!over && qmark[character] == 0)
                over = true;

            if (over)
            {  
                // ran out of string characters, everything else is just blank
                counterDigits[digit] = 0;
                digit++;
            }
            else
            {
                counterDigits[digit] = qmark[character] - '0';
                // is the next charact er a dot?
                if (qmark[character+1] == '.')
                {
                    // yes, eat it and set the DP on this digit
                    character++;
                    // counterDigits[digit] |= SEGMENT_DP;
                }

                digit++;
            }

            character++;
        }

        counter_to_segments();
    }

    httpd_resp_send(req, sz, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}



// Function to start the web server
httpd_handle_t start_webserver(void) 
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(LOG_TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Register URI handlers
        httpd_uri_t root_uri = {
            .uri      = "/",
            .method   = HTTP_GET,
            .handler  = root_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &root_uri);

        // Register URI handlers
        httpd_uri_t digits_uri = {
            .uri      = "/digits",
            .method   = HTTP_GET,
            .handler  = digits_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &digits_uri);

        // Register URI handlers
        httpd_uri_t digits2_uri = {
            .uri      = "/digits2",
            .method   = HTTP_GET,
            .handler  = digits2_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &digits2_uri);

        httpd_uri_t segments_uri = {
            .uri      = "/segments",
            .method   = HTTP_GET,
            .handler  = segments_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &segments_uri);
    }
    return server;
}

