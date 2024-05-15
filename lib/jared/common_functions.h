#ifndef __COMMON_FUNCTIONS_H__
#define __COMMON_FUNCTIONS_H__

#include <Arduino.h>

#define ON(light) digitalWrite(light, HIGH)
#define OFF(light) digitalWrite(light, LOW)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct blink_handle blink_handle_t;

void start_blink(uint8_t light, uint32_t duration, blink_handle_t **handle);
void stop_blink(blink_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_FUNCTIONS_H__ */