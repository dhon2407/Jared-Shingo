#include <stdbool.h>

#include "common_functions.h"

#define MAX_BLINKING_COUNT 10

#define ON(light) digitalWrite(light, HIGH)
#define OFF(light) digitalWrite(light, LOW)

#define BLINKLIGHT(light, duration) \
    ON(light); \
    vTaskDelay(duration / portTICK_PERIOD_MS); \
    OFF(light); \
    vTaskDelay(duration / portTICK_PERIOD_MS);

typedef struct blink_handle
{
    int8_t index;
    TaskHandle_t task;
} blink_handle_t;

typedef struct 
{
    const char *taskName;
    uint8_t lightCode;
    blink_handle_t handle;
    uint32_t duration;
    volatile bool active;
} blinking_data_t;


static blinking_data_t blinklink_list[10] =
{
    { "blinking01", 0, { -1, NULL }, 0U, false },
    { "blinking02", 0, { -1, NULL }, 0U, false },
    { "blinking03", 0, { -1, NULL }, 0U, false },
    { "blinking04", 0, { -1, NULL }, 0U, false },
    { "blinking05", 0, { -1, NULL }, 0U, false },
    { "blinking06", 0, { -1, NULL }, 0U, false },
    { "blinking07", 0, { -1, NULL }, 0U, false },
    { "blinking08", 0, { -1, NULL }, 0U, false },
    { "blinking09", 0, { -1, NULL }, 0U, false },
    { "blinking10", 0, { -1, NULL }, 0U, false }
};

SemaphoreHandle_t mutex = NULL;

static void blinkLight_execute_task(void *blinkParam);


void start_blink(uint8_t light, uint32_t duration, blink_handle_t **handle)
{
    size_t index = 0U;
    size_t list_size = sizeof(blinklink_list) / sizeof(blinklink_list[0]);
    blinking_data_t *blink_data = NULL;

    if (mutex == NULL)
    {
        mutex = xSemaphoreCreateMutex();
    }

    xSemaphoreTake(mutex, portMAX_DELAY);
    for (size_t index = 0; index < list_size; index++)
    {
        if (blinklink_list[index].active == false && blinklink_list[index].handle.task == NULL)
        {
            blink_data = &blinklink_list[index];
            blinklink_list[index].handle.index = index;
            *handle = &blinklink_list[index].handle;
            break;
        }
    }

    if (blink_data == NULL)
    {
        //No available allocations
        *handle = NULL;
        xSemaphoreGive(mutex);
        return;
    }

    blink_data->duration = duration;
    blink_data->lightCode = light;

    xTaskCreate(blinkLight_execute_task, blink_data->taskName, 1000, blink_data, 1, &blink_data->handle.task);

    xSemaphoreGive(mutex);

    return;
}

void stop_blink(blink_handle_t *handle)
{
    size_t index = 0U;
    size_t list_size = sizeof(blinklink_list) / sizeof(blinklink_list[0]);

    if (handle->index >= list_size)
    {
        //Invalid index
        return;
    }

    if (mutex == NULL)
    {
        mutex = xSemaphoreCreateMutex();
    }

    xSemaphoreTake(mutex, portMAX_DELAY);
    if (blinklink_list[handle->index].active == true && 
        blinklink_list[handle->index].handle.task != NULL)
    {
        blinklink_list[handle->index].active = false;
        vTaskDelete(blinklink_list[handle->index].handle.task);
        blinklink_list[handle->index].handle.task = NULL;
        blinklink_list[handle->index].handle.index = -1;
    }
    xSemaphoreGive(mutex);

    return;
}

void blinkLight_execute_task(void *blinkParam)
{
    blinking_data_t *blink_param = blinkParam;

    if (blink_param == NULL)
    {
        return;
    }

    blink_param->active = true;
    while (blink_param->active == true)
    {
        BLINKLIGHT(blink_param->lightCode, blink_param->duration);
    }
}