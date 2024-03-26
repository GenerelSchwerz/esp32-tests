#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <stddef.h>


// #include <Arduino.h>

// #ifdef __cplusplus
// extern "C" {
// #endif

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

const size_t index_html_length = index_html_end - index_html_start;

extern const uint8_t index_css_start[] asm("_binary_index_css_start");
extern const uint8_t index_css_end[] asm("_binary_index_css_end");

 const size_t index_css_length = index_css_end - index_css_start;

// #ifdef __cplusplus
// }   
// #endif

#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

#endif /* CONSTANTS_H */