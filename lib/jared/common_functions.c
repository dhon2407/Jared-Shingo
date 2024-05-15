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

typedef struct 
{
    const char *taskName;
    uint8_t lightCode;
    TaskHandle_t handle;
    uint32_t duration;
    bool active;
} blinking_data_t;


static blinking_data_t blinklink_list[10] =
{
    { "blinking01", NULL, 0U },
    { "blinking02", NULL, 0U },
    { "blinking03", NULL, 0U },
    { "blinking04", NULL, 0U },
    { "blinking05", NULL, 0U },
    { "blinking06", NULL, 0U },
    { "blinking07", NULL, 0U },
    { "blinking08", NULL, 0U },
    { "blinking09", NULL, 0U },
    { "blinking10", NULL, 0U },
};

SemaphoreHandle_t mutex = NULL;

static void blinkLight_execute_task(void *blinkParam);


void start_blink(uint8_t light, uint32_t duration, TaskHandle_t *handle)
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
        if (blinklink_list[index].active == false && blinklink_list[index].handle == NULL)
        {
            blink_data = &blinklink_list[index];
            break;
        }
    }

    if (blink_data == NULL)
    {
        //No available allocations
        xSemaphoreGive(mutex);
        return;
    }

    blink_data->duration = duration;
    blink_data->lightCode = light;

    xTaskCreate(blinkLight_execute_task, blink_data->taskName, 1000, blink_data, 1, &blink_data->handle);

    *handle = blink_data->handle;
    xSemaphoreGive(mutex);

    return;
}

void stop_blink(TaskHandle_t handle)
{
    size_t index = 0U;
    size_t list_size = sizeof(blinklink_list) / sizeof(blinklink_list[0]);

    if (mutex == NULL)
    {
        mutex = xSemaphoreCreateMutex();
    }

    xSemaphoreTake(mutex, portMAX_DELAY);
    for (size_t index = 0; index < list_size; index++)
    {
        if (blinklink_list[index].handle == handle)
        {
            blinklink_list->active = false;
            vTaskDelete(blinklink_list->handle);
            blinklink_list->handle = NULL;
            break;
        }
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