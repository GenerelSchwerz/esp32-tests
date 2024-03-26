#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <esp_system.h>
#include <esp_http_server.h>

#include "constants.h" // Include the header file for static files
#include "helper.h" // Include the header file for utility functions
#include "endpoints.h" // Include the header file for endpoint handlers


// weird structure due to not understanding PlatformIO's build system.
// I'll fix it later.

bool server_started = false;
httpd_handle_t server = NULL;

// constants.cpp


extern const uint8_t index_html_start[] asm("_binary_src_frontend_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_src_frontend_index_html_end");
const size_t index_html_length = index_html_end - index_html_start;


extern const uint8_t index_css_start[] asm("_binary_src_frontend_index_css_start");
extern const uint8_t index_css_end[] asm("_binary_src_frontend_index_css_end");
const size_t index_css_length = index_css_end - index_css_start;



// helper.cpp


String readSerialLine() {
  String line = "";
  while (true) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      if (c == '\n') { // Check for newline character
       
        break; // Exit loop when newline is received
      }
      line += c; // Append character to line
    }
  }

  line.trim(); // Remove leading and trailing whitespaces
  return line;
}


// endpoints.cpp


#include "constants.h" // Include static files.


esp_err_t index_html_get_handler(httpd_req_t *req);
esp_err_t index_css_get_handler(httpd_req_t *req);
esp_err_t test_html_get_handler(httpd_req_t *req);



// Handler implementations
esp_err_t index_html_get_handler(httpd_req_t *req) {
    httpd_resp_send(req, (char*) index_html_start, index_html_length);
    return ESP_OK;
}

esp_err_t index_css_get_handler(httpd_req_t *req) {
    httpd_resp_send(req, (char*) index_css_start, index_css_length);
    return ESP_OK;
}

esp_err_t test_html_get_handler(httpd_req_t *req) {
    // Handle another endpoint here
    const char* resp_str = "This is another endpoint!";
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}


// Define URI configurations
httpd_uri_t index_html_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = index_html_get_handler,
    .user_ctx  = NULL
};


httpd_uri_t index_css_uri = {
    .uri       = "/styles/index.css",
    .method    = HTTP_GET,
    .handler   = index_css_get_handler,
    .user_ctx  = NULL
};


httpd_uri_t test_html_uri = {
    .uri       = "/test",
    .method    = HTTP_GET,
    .handler   = test_html_get_handler,
    .user_ctx  = NULL
};


// main.cpp


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
