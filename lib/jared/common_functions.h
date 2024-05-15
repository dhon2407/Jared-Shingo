#ifndef __COMMON_FUNCTIONS_H__
#define __COMMON_FUNCTIONS_H__

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

void start_blink(uint8_t light, uint32_t duration, TaskHandle_t *handle);
void stop_blink(TaskHandle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_FUNCTIONS_H__ */