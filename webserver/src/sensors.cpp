#include "constants.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include <esp_log.h>
#include <freertos/timers.h>



static const gpio_num_t hc_trigPin = GPIO_NUM_5;
static const gpio_num_t hc_echoPin = GPIO_NUM_18;

static const char* TAG = "HC-SR04";

volatile uint32_t pulseDuration = 0;
volatile uint64_t pulseStartTime = 0;

static QueueHandle_t gpio_evt_queue = NULL;



// GPIO interrupt handler
void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;

    ESP_LOGI(TAG, "Interrupt triggered");

    // Check if the interrupt is for the input pin
    if(gpio_num == hc_echoPin) {
        // Check the pin level to determine if it's rising or falling edge
        if(gpio_get_level(hc_echoPin) == 1) {
            // Rising edge detected, record start time
            pulseStartTime = esp_timer_get_time();
        } else {
            // Falling edge detected, calculate pulse duration
            pulseDuration = (uint32_t)(esp_timer_get_time() - pulseStartTime);
        }
    }
}

// Function to measure pulse duration
uint32_t pulseIn(int delay) {
    // Reset pulse duration
    pulseDuration = 0;

    // Wait for pulse duration to be captured
    while(pulseDuration == 0) {
        ESP_LOGI(TAG, "Waiting for pulse duration...");
        vTaskDelay(pdMS_TO_TICKS(delay));
    }

    return pulseDuration;
}

void sensor_init() {
    gpio_set_direction(hc_trigPin, GPIO_MODE_OUTPUT);
    gpio_set_direction(hc_echoPin, GPIO_MODE_INPUT);

    gpio_set_pull_mode(hc_echoPin, GPIO_PULLUP_ONLY);
    // Configure GPIO interrupt
    gpio_set_intr_type(hc_echoPin, GPIO_INTR_ANYEDGE);

    // Install ISR service with default configuration
    gpio_install_isr_service(0);

    // Hook ISR handler to the pin
    gpio_isr_handler_add(hc_echoPin, gpio_isr_handler, (void*) hc_echoPin);
}





void sensor_report(void* xTimer) {
    ESP_LOGI(TAG, "Starting sensor task...");
    while (1) {
        gpio_set_level(hc_trigPin, 0);
        vTaskDelay(pdMS_TO_TICKS(2));

        gpio_set_level(hc_trigPin, 1);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(hc_trigPin, 0);

        uint32_t duration = pulseIn(10);

        // Calculate distance in cm
        float distance = duration * SOUND_SPEED / 2;

        // Convert distance to inches
        float distanceIn =  distance * CM_TO_INCH;

        ESP_LOGI(TAG, "Distance: %.2f inches, %.2f", distance, distanceIn);  

        vTaskDelay(pdMS_TO_TICKS(1000));
    }


}