#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <esp_system.h>
#include <esp_http_server.h>

#include "constants.h" // Include the header file for static files
#include "helper.h" // Include the header file for utility functions
#include "endpoints.h" // Include the header file for endpoint handlers



bool server_started = false;
httpd_handle_t server = NULL;



void connectToWifi(const char* ssid, const char* password)
{
  WiFi.enableSTA(true);
  
  delay(2000);

  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void beginServer()
{
  // Start the HTTP server
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  if (httpd_start(&server, &config) == ESP_OK) {
      // Register URI handlers
      httpd_register_uri_handler(server, &index_html_uri);
      httpd_register_uri_handler(server, &index_css_uri);
      httpd_register_uri_handler(server, &test_html_uri);
      server_started = true;
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("Starting HTTP Server");
  
  Serial.print("SSID?: ");
  Serial.flush();
  String ssid = readSerialLine();
  Serial.println();
 
  Serial.print("Password?: ");
  Serial.flush();
  String password = readSerialLine();
  Serial.println();

  Serial.printf("Connecting to %s with password %s\n", ssid, password);

  connectToWifi(ssid.c_str(), password.c_str());
  beginServer();
}

void loop() {
  if (!server_started) return;

  // impl. respond to serial data here (kill on ctrl+c, etc.)
}
