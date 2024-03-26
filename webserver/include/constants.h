#ifndef CONSTANTS_H
#define CONSTANTS_H

// #include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

extern const size_t index_html_length;

extern const uint8_t index_css_start[] asm("_binary_index_css_start");
extern const uint8_t index_css_end[] asm("_binary_index_css_end");

extern const size_t index_css_length;

#ifdef __cplusplus
}   
#endif

#endif /* CONSTANTS_H */