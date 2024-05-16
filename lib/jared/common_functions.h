#ifndef __COMMON_FUNCTIONS_H__
#define __COMMON_FUNCTIONS_H__

#include <Arduino.h>

#define ON(light) digitalWrite(light, HIGH)
#define OFF(light) digitalWrite(light, LOW)

#define MUTEX_INIT(mutex) mutex = xSemaphoreCreateMutex()
#define MUTEX_LOCK(mutex) xSemaphoreTake(mutex, portMAX_DELAY)
#define MUTEX_UNLOCK(mutex) xSemaphoreGive(mutex)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct task_handle task_handle_t;
typedef void (*actionCallback)(void);

void start_blink(uint8_t light, uint32_t duration, task_handle_t **handle);
void stop_blink(task_handle_t *handle);
void start_interval_action(actionCallback startAction, actionCallback endAction,
                           uint32_t duration, task_handle_t **handle);
void stop_interval_action(task_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_FUNCTIONS_H__ */