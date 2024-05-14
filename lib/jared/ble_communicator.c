#include <Arduino.h>
#include <stdbool.h>

#include "ble_communicator.h"

#define ON(light) digitalWrite(light, HIGH)
#define OFF(light) digitalWrite(light, LOW)

#define BLINKLIGHT(light, duration) \
    ON(light); \
    vTaskDelay(duration); \
    OFF(light); \
    vTaskDelay(duration);

static void ble_connectingMode(void);
static void ble_error(const char *error_log);
static void ble_blinkLight(void *blinkParam);

typedef struct
{
    const byte lightCode;
    const TickType_t duration;
    volatile bool active;
    TaskHandle_t taskHandle;

} blink_param_t;

blink_param_t connectingBlink = { LED_INDICATOR_CONNECTED, 200 / portTICK_PERIOD_MS , false };

void ble_comm_init(void)
{
    pinMode(LED_INDICATOR_CONNECTED, OUTPUT);
    pinMode(LED_INDICATOR_ERROR, OUTPUT);

    ble_connectingMode();
}

void ble_comm_deinit(void)
{
    if (connectingBlink.active == true)
    {
        connectingBlink.active = false;
    }

}


/* INTERNAL FUNCTIONS */
void ble_connectingMode(void)
{
    if (connectingBlink.active == false)
    {
        xTaskCreate(ble_blinkLight, "BlinkConnecting", 1000, &connectingBlink, 1, &connectingBlink.taskHandle);
    }
}

void ble_blinkLight(void *blinkParam)
{
    blink_param_t *blink_param = blinkParam;

    if (blink_param == NULL)
    {
        ble_error("Blink param is null!");
        return;
    }

    blink_param->active = true;
    while (blink_param->active == true)
    {
        BLINKLIGHT(blink_param->lightCode, blink_param->duration);
    }

    if (blink_param->taskHandle != NULL)
    {
        vTaskDelete(blink_param->taskHandle);
        blink_param->taskHandle = NULL;
    }
}

void ble_error(const char *error_log)
{
    //TODO print error log
    (void)error_log;

    digitalWrite(LED_INDICATOR_ERROR, HIGH);
}
