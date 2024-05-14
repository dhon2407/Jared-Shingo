#include <Arduino.h>

#include "trafficlight.h"

#define ON(light)   digitalWrite(light, HIGH)
#define OFF(light)  digitalWrite(light, LOW)

typedef enum {
    CARS_GO,
    CARS_STOPPING,
    CARS_STOP,
    PED_GO,
    PED_STOPPING,
    PED_STOP,
    UNKNOWN,
} traffic_state_t;

static traffic_state_t g_current_state = UNKNOWN;
static volatile bool traffic_light_running = false;


/* STATIC FUNCTIONS */
static void main_traffic_light_loop(void *params);
static TaskHandle_t main_taskHandle;


void traffic_light_init(void)
{
    pinMode(LED_CAR_GREEN, OUTPUT);
    pinMode(LED_CAR_RED, OUTPUT);
    pinMode(LED_CAR_YELLOW, OUTPUT);
    pinMode(LED_PED_GREEN, OUTPUT);
    pinMode(LED_PED_RED, OUTPUT);
}

void traffic_light_start(void)
{
    if (traffic_light_running != true)
    {
        xTaskCreate(main_traffic_light_loop, "traffic_light_loop", 1000, NULL, 1, &main_taskHandle);
    }

    return;
}

void traffic_light_stop(void)
{
    if (traffic_light_running != true) return;

    traffic_light_running = false;
    
    return;
}

void main_traffic_light_loop(void *params)
{
    (void)params;

    traffic_light_running = true;
    while (traffic_light_running == true)
    {
        //TODO run state machine
        g_current_state = CARS_GO;
        ON(LED_CAR_GREEN);
        ON(LED_CAR_YELLOW);
        ON(LED_CAR_RED);
        ON(LED_PED_RED);
        ON(LED_PED_GREEN);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    if (main_taskHandle != NULL)
    {
        OFF(LED_CAR_GREEN);
        OFF(LED_CAR_YELLOW);
        OFF(LED_CAR_RED);
        OFF(LED_PED_RED);
        OFF(LED_PED_GREEN);

        //Should be last as this function will be terminated
        vTaskDelete(main_taskHandle);
        main_taskHandle = NULL;
    }
}