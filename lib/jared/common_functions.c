#include <stdbool.h>

#include "common_functions.h"

#define MAX_BLINKING_COUNT 10
#define MAX_TASK_COUNT 10

#define BLINKLIGHT(light, duration) \
    ON(light); \
    vTaskDelay(duration / portTICK_PERIOD_MS); \
    OFF(light); \
    vTaskDelay(duration / portTICK_PERIOD_MS);

typedef struct task_handle
{
    int8_t index;
    TaskHandle_t task;
} task_handle_t;

typedef struct 
{
    const char *taskName;
    uint8_t lightCode;
    task_handle_t handle;
    uint32_t duration;
    volatile bool active;
} blinking_data_t;

typedef struct 
{
    const char *taskName;
    actionCallback startAction;
    actionCallback endAction;
    task_handle_t handle;
    uint32_t duration;
    volatile bool active;
} interval_task_data_t;


static blinking_data_t blinklink_list[MAX_BLINKING_COUNT] =
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

static interval_task_data_t intervalTask_list[MAX_TASK_COUNT] =
{
    { "interv_task01", NULL, NULL, { -1, NULL }, 0U, false },
    { "interv_task02", NULL, NULL, { -1, NULL }, 0U, false },
    { "interv_task03", NULL, NULL, { -1, NULL }, 0U, false },
    { "interv_task04", NULL, NULL, { -1, NULL }, 0U, false },
    { "interv_task05", NULL, NULL, { -1, NULL }, 0U, false },
    { "interv_task06", NULL, NULL, { -1, NULL }, 0U, false },
    { "interv_task07", NULL, NULL, { -1, NULL }, 0U, false },
    { "interv_task08", NULL, NULL, { -1, NULL }, 0U, false },
    { "interv_task09", NULL, NULL, { -1, NULL }, 0U, false },
    { "interv_task10", NULL, NULL, { -1, NULL }, 0U, false }
};

static SemaphoreHandle_t blinkListMutex = NULL;
static SemaphoreHandle_t intervalTaskListMutex = NULL;

static void blinkLight_execute_task(void *blinkParam);
static void execute_task_interval(void *taskParam);


void start_blink(uint8_t light, uint32_t duration, task_handle_t **handle)
{
    size_t list_size = sizeof(blinklink_list) / sizeof(blinklink_list[0]);
    blinking_data_t *blink_data = NULL;

    if (blinkListMutex == NULL)
    {
        blinkListMutex = xSemaphoreCreateMutex();
    }

    xSemaphoreTake(blinkListMutex, portMAX_DELAY);
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
        xSemaphoreGive(blinkListMutex);
        return;
    }

    blink_data->duration = duration;
    blink_data->lightCode = light;

    xTaskCreate(blinkLight_execute_task, blink_data->taskName, 1000, blink_data, 1, &blink_data->handle.task);

    xSemaphoreGive(blinkListMutex);

    return;
}

void stop_blink(task_handle_t *handle)
{
    size_t list_size = sizeof(blinklink_list) / sizeof(blinklink_list[0]);

    if (handle == NULL || handle->index >= list_size)
    {
        //Invalid parameter
        return;
    }

    if (blinkListMutex == NULL)
    {
        blinkListMutex = xSemaphoreCreateMutex();
    }

    xSemaphoreTake(blinkListMutex, portMAX_DELAY);
    if (blinklink_list[handle->index].active == true && 
        blinklink_list[handle->index].handle.task != NULL)
    {
        blinklink_list[handle->index].active = false;
        vTaskDelete(blinklink_list[handle->index].handle.task);
        blinklink_list[handle->index].handle.task = NULL;
        blinklink_list[handle->index].handle.index = -1;
    }
    xSemaphoreGive(blinkListMutex);

    return;
}

void start_interval_action(actionCallback startAction, actionCallback endAction,
                           uint32_t duration, task_handle_t **handle)
{
    size_t list_size = sizeof(intervalTask_list) / sizeof(intervalTask_list[0]);
    interval_task_data_t *interval_data = NULL;

    if (intervalTaskListMutex == NULL)
    {
        intervalTaskListMutex = xSemaphoreCreateMutex();
    }

    xSemaphoreTake(intervalTaskListMutex, portMAX_DELAY);
    for (size_t index = 0; index < list_size; index++)
    {
        if (intervalTask_list[index].active == false &&
            intervalTask_list[index].handle.task == NULL)
        {
            interval_data = &intervalTask_list[index];
            intervalTask_list[index].handle.index = index;
            *handle = &intervalTask_list[index].handle;
            break;
        }
    }

    if (interval_data == NULL)
    {
        //No available allocations
        *handle = NULL;
        xSemaphoreGive(intervalTaskListMutex);
        return;
    }

    interval_data->duration = duration;
    interval_data->startAction = startAction;
    interval_data->endAction = endAction;

    xTaskCreate(execute_task_interval, interval_data->taskName, 1000, interval_data, 1, &interval_data->handle.task);

    xSemaphoreGive(intervalTaskListMutex);

    return;
}
void stop_interval_action(task_handle_t *handle)
{
    size_t list_size = sizeof(intervalTask_list) / sizeof(intervalTask_list[0]);

    if (handle == NULL || handle->index >= list_size)
    {
        //Invalid parameter
        return;
    }

    if (intervalTaskListMutex == NULL)
    {
        intervalTaskListMutex = xSemaphoreCreateMutex();
    }

    xSemaphoreTake(intervalTaskListMutex, portMAX_DELAY);
    if (intervalTask_list[handle->index].active == true && 
        intervalTask_list[handle->index].handle.task != NULL)
    {
        intervalTask_list[handle->index].active = false;
        vTaskDelete(intervalTask_list[handle->index].handle.task);
        intervalTask_list[handle->index].handle.task = NULL;
        intervalTask_list[handle->index].handle.index = -1;
    }
    xSemaphoreGive(intervalTaskListMutex);

    return;
}


/*========================= STATIC FUNCTIONS =========================*/
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

void execute_task_interval(void *taskParam)
{
    interval_task_data_t *data = taskParam;

    if (data == NULL)
    {
        return;
    }

    data->active = true;
    while (data->active == true)
    {
        if (data->startAction != NULL)
        {
            data->startAction();
        }
        vTaskDelay(data->duration / portTICK_PERIOD_MS);

        if (data->endAction != NULL)
        {
            data->endAction();
        }
        vTaskDelay(data->duration / portTICK_PERIOD_MS);
    }
}