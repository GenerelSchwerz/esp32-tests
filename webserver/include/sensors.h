#ifndef SENSORS_H
#define SENSORS_H


// #ifdef __cplusplus
// extern "C" {
// #endif

// Handler function prototypes
void sensor_init();
void sensor_report(void* xTimer);

extern volatile const float current_distance;

// #ifdef __cplusplus
// }
// #endif


#endif /* SENSORS_H */
