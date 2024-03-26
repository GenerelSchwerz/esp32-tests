// #include <Arduino.h>
#include "esp_system.h"


extern const uint8_t index_html_start[] asm("_binary_src_frontend_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_src_frontend_index_html_end");
const size_t index_html_length = index_html_end - index_html_start;



extern const uint8_t index_css_start[] asm("_binary_src_frontend_index_css_start");
extern const uint8_t index_css_end[] asm("_binary_src_frontend_index_css_end");
const size_t index_css_length = index_css_end - index_css_start;


