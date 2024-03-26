// #include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h> // for isspace
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "driver/uart.h" // Include UART driver header file


void promptResponse(const char* msg, char* wanted, size_t max_len) {
    printf("%s", msg);
    fflush(stdout); // Ensure message is printed immediately

    int pos = 0;
    while (1) {
        int data = getchar(); // Read data from stdin

        if (data != EOF) {
            if (data == '\n' || pos >= max_len - 1) {
                // Exit loop when newline is received or buffer is full
                break;
            }

            wanted[pos++] = data; // Append character to wanted buffer
        } else {
            vTaskDelay(10 / portTICK_PERIOD_MS); // Delay a bit if EOF is received
        }
    }

    wanted[pos] = '\0'; // Null-terminate the string

    // Remove leading and trailing whitespaces
    char *start = wanted;
    while (*start && isspace(*start)) {
        start++;
    }

    char *end = wanted + strlen(start) - 1;
    while (end > start && isspace(*end)) {
        *end-- = '\0';
    }

    // Copy the trimmed string back to wanted
    if (start != wanted) {
        memmove(wanted, start, end - start + 2);
    }



    // clear stdin buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    printf("\n");
    fflush(stdout);
}