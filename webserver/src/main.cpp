#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"


#include "constants.h" // Assuming this contains constants used in your project
#include "helper.h"    // Utility functions you've defined
#include "endpoints.h" // Endpoint handlers for the HTTP server
#include "sensors.h"   // Sensor functions

static const char *TAG = "root";



void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected. Trying to reconnect...");
        esp_wifi_connect();
    }
}

void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}


void wifi_init_sta(const char *ssid, const char *password)
{
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));

    wifi_config_t wifi_config = {};

    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config((wifi_interface_t) ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}


httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Register URI handlers
        httpd_register_uri_handler(server, &index_html_uri);
        httpd_register_uri_handler(server, &index_css_uri);
        httpd_register_uri_handler(server, &index_js_uri);
        httpd_register_uri_handler(server, &test_html_uri);
        httpd_register_uri_handler(server, &ws_uri);
    }
    return server;
}

void stop_webserver(httpd_handle_t server)
{
    if (server) {
        // Stop the httpd server
        httpd_stop(server);
    }
}


extern "C" void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // esp_log_level_set("*", ESP_LOG_ERROR);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    char ssid[32];
    char password[64];

    promptResponse("Enter SSID: ", ssid, sizeof(ssid));
    ESP_LOGI(TAG, "Got SSID: %s", ssid);

    promptResponse("Enter password: ", password, sizeof(password));
    ESP_LOGI(TAG, "Got password: %s", password);

    wifi_init_sta(ssid, "");
    ESP_LOGI(TAG, "Connected to AP. Starting web server...");

    // The server can be stopped using stop_webserver(server);
    httpd_handle_t server = start_webserver();
    ESP_LOGI(TAG, "Web server started!");


    
    // Initialize sensor
    sensor_init();

    ESP_LOGI(TAG, "Sensor initialized! Starting sensor task...");

    
    xTaskCreate(&sensor_report,        // Task function
                "hc-sensor-task",    // Task name
                4096,                // Stack size (bytes)
                (void*) 500,         // Task parameters
                1,                   // Priority (1 is lowest priority)
                NULL);               // Task handle

}
