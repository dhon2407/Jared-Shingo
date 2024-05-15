#include <Arduino.h>

#include "trafficlight.h"

#define ON(light)   digitalWrite(light, HIGH)
#define OFF(light)  digitalWrite(light, LOW)
#define MAIN_LOOP_TICK (10  / portTICK_PERIOD_MS)

typedef enum {
    CARS_GO,
    CARS_STOPPING,
    CARS_STOP,
    PED_GO,
    PED_STOPPING,
    PED_STOP,
    UNKNOWN,
} traffic_state_t;

typedef void (*on_start_state_callback)(void);

typedef struct
{
    traffic_state_t state;
    on_start_state_callback start_action;
} start_state_map_t;

typedef struct
{
    traffic_state_t current_state;
    traffic_state_t next_state;
    unsigned long state_duration_millis;
} state_duration_map_t;


static traffic_state_t g_current_state = UNKNOWN;
static volatile bool traffic_light_running = false;
static TaskHandle_t main_taskHandle;

static void start_CARS_GO(void);
static void start_CARS_STOPPING(void);
static void start_CARS_STOP(void);
static void start_PED_GO(void);
static void start_PED_STOPPING(void);
static void start_PED_STOP(void);

static start_state_map_t start_actions[] =
{
    { CARS_GO, start_CARS_GO },
    { CARS_STOPPING, start_CARS_STOPPING },
    { CARS_STOP, start_CARS_STOP },
    { PED_GO, start_PED_GO },
    { PED_STOPPING, start_PED_STOPPING },
    { PED_STOP, start_PED_STOP },
};

static state_duration_map_t state_duration_map[] =
{
    { CARS_GO, CARS_STOPPING, 5000 },
    { CARS_STOPPING, CARS_STOP, 2000 },
    { CARS_STOP, PED_GO, 1000 },
    { PED_GO, PED_STOPPING, 5000 },
    { PED_STOPPING, PED_STOP, 3000 },
    { PED_STOP, CARS_GO, 2000 },
};


/* STATIC FUNCTIONS */
static void main_traffic_light_loop(void *params);
static traffic_state_t changeState(traffic_state_t current_state);
static traffic_state_t process_state(traffic_state_t current_state, unsigned long timeLapse);
static void lights_all_off(void);

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
    traffic_state_t next_state = UNKNOWN;
    unsigned long startTime = 0U;
    unsigned long currentTime = 0U;
    unsigned long timeLapsed = 0U;

    /* Start at CARS_GO state */
    startTime = millis();
    g_current_state = changeState(CARS_GO);

    traffic_light_running = true;
    while (traffic_light_running == true)
    {
        currentTime = millis();
        timeLapsed = currentTime - startTime;
        next_state = process_state(g_current_state, timeLapsed);
        if (next_state != g_current_state)
        {
            startTime = millis();
            g_current_state = changeState(next_state);
        }

        vTaskDelay(MAIN_LOOP_TICK);
    }

    if (main_taskHandle != NULL)
    {
        lights_all_off();

        //Should be last as this function will be terminated
        vTaskDelete(main_taskHandle);
        main_taskHandle = NULL;
    }
}

traffic_state_t changeState(traffic_state_t targetState)
{
    size_t actionsSize = sizeof(start_actions) / sizeof(start_actions[0]);
    size_t index = 0U;

    for (index = 0; index < actionsSize; index++)
    {
        if (start_actions[index].state == targetState)
        {
            if (start_actions[index].start_action)
            {
                start_actions[index].start_action();
            }
            return targetState;
        }
    }

    assert(0);
}

traffic_state_t process_state(traffic_state_t current_state, unsigned long timeLapse)
{
    size_t map_size = sizeof(state_duration_map) / sizeof(state_duration_map[0]);
    size_t index = 0U;

    for (index = 0; index < map_size; index++)
    {
        if (state_duration_map[index].current_state == current_state)
        {
            if (state_duration_map[index].state_duration_millis >= timeLapse)
            {
                return current_state;
            }

            return state_duration_map[index].next_state;
        }
    }

    assert(0);
}

void lights_all_off(void)
{
    OFF(LED_CAR_GREEN);
    OFF(LED_CAR_YELLOW);
    OFF(LED_CAR_RED);
    OFF(LED_PED_RED);
    OFF(LED_PED_GREEN);

    return;
}

void start_CARS_GO(void)
{
    ON(LED_CAR_GREEN);
    OFF(LED_CAR_YELLOW);
    OFF(LED_CAR_RED);
    ON(LED_PED_RED);
    OFF(LED_PED_GREEN);
}

void start_CARS_STOPPING(void)
{
    OFF(LED_CAR_GREEN);
    ON(LED_CAR_YELLOW);
    OFF(LED_CAR_RED);
    ON(LED_PED_RED);
    OFF(LED_PED_GREEN);
}
void start_CARS_STOP(void)
{
    OFF(LED_CAR_GREEN);
    OFF(LED_CAR_YELLOW);
    ON(LED_CAR_RED);
    ON(LED_PED_RED);
    OFF(LED_PED_GREEN);
}
void start_PED_GO(void)
{
    OFF(LED_CAR_GREEN);
    OFF(LED_CAR_YELLOW);
    ON(LED_CAR_RED);
    OFF(LED_PED_RED);
    ON(LED_PED_GREEN);
}
void start_PED_STOPPING(void)
{
    OFF(LED_CAR_GREEN);
    OFF(LED_CAR_YELLOW);
    ON(LED_CAR_RED);
    OFF(LED_PED_RED);
    ON(LED_PED_GREEN);
}
void start_PED_STOP(void)
{
    OFF(LED_CAR_GREEN);
    OFF(LED_CAR_YELLOW);
    ON(LED_CAR_RED);
    ON(LED_PED_RED);
    OFF(LED_PED_GREEN);
}