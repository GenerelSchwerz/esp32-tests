// #include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "driver/uart.h" // Include UART driver header file


void promptResponse(const char* msg, char* ssid, size_t max_len) {
    printf(msg);
    fflush(stdout); // Ensure 'Please enter SSID' message is printed immediately

 
   // Loop until input buffer ends with '\n'
    while (1) {
        // Read input
        if (fgets(ssid, max_len, stdin) == NULL) {
            ESP_LOGE("INPUT", "Failed to read SSID from input");
            vTaskDelay(500 / portTICK_PERIOD_MS);
            continue;
        }

        // Check if the input ends with '\n'
        size_t len = strlen(ssid);
        if (len > 0 && ssid[len - 1] == '\n') {
            // Remove newline character
            ssid[len - 1] = '\0';
            break; // Exit the loop
        }

        // If the input is too long for the buffer, clear the input stream
        if (len == max_len - 1 && ssid[len - 1] != '\n') {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            ESP_LOGE("INPUT", "Input too long. Please enter a shorter value.");
            // Prompt the user again
            printf("%s", msg);
            fflush(stdout);
        }
    }

    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    // fflush(stdin); // Clear the input buffer
}