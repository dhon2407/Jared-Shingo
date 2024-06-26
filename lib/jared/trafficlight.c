#include <Arduino.h>

#include "trafficlight_internal.h"
#include "common_functions.h"

static traffic_state_t g_current_state = UNKNOWN;
static volatile bool traffic_light_running = false;
static TaskHandle_t main_taskHandle;
static task_handle_t *blinkingPedWalkHandle = NULL;
static task_handle_t *cautionModeHandle = NULL;
static task_handle_t *jaredModeHandle = NULL;

static SemaphoreHandle_t eventMutex = NULL;
static volatile process_data_t current_data = { NO_EVENT, 0 };

static state_action_map_t start_actions[] =
{
    { CARS_GO,          start_CARS_GO },
    { CARS_STOPPING,    start_CARS_STOPPING },
    { CARS_STOP,        start_CARS_STOP },
    { PED_GO,           start_PED_GO },
    { PED_STOPPING,     start_PED_STOPPING },
    { PED_STOP,         start_PED_STOP },
    { CAUTION_MODE,     start_CAUTION_MODE },
    { OFF,              lights_all_off },
    { JARED_MODE,       start_JARED_MODE },
    { MANUAL_MODE,      start_MANUAL_MODE },
};

static state_action_map_t end_actions[] =
{
    { PED_STOPPING,     end_PED_STOPPING },
    { CAUTION_MODE,     end_CAUTION_MODE },
    { JARED_MODE,       stop_JARED_MODE },
    { MANUAL_MODE,      stop_MANUAL_MODE },
};

static state_event_map_t state_event_map[] =
{
    { OFF,              CARS_GO,        NORMAL_MODE_EVENT },
    { CAUTION_MODE,     CARS_GO,        NORMAL_MODE_EVENT },
    { JARED_MODE,       CARS_GO,        NORMAL_MODE_EVENT },
    { MANUAL_MODE,      CARS_GO,        NORMAL_MODE_EVENT },

    { CARS_GO,          CAUTION_MODE,   CAUTION_MODE_EVENT },
    { CARS_STOPPING,    CAUTION_MODE,   CAUTION_MODE_EVENT },
    { CARS_STOP,        CAUTION_MODE,   CAUTION_MODE_EVENT },
    { PED_GO,           CAUTION_MODE,   CAUTION_MODE_EVENT },
    { PED_STOPPING,     CAUTION_MODE,   CAUTION_MODE_EVENT },
    { PED_STOP,         CAUTION_MODE,   CAUTION_MODE_EVENT },
    { JARED_MODE,       CAUTION_MODE,   CAUTION_MODE_EVENT },
    { MANUAL_MODE,      CAUTION_MODE,   CAUTION_MODE_EVENT },

    { CARS_GO,          OFF,            TURNOFF_EVENT },
    { CARS_STOPPING,    OFF,            TURNOFF_EVENT },
    { CARS_STOP,        OFF,            TURNOFF_EVENT },
    { PED_GO,           OFF,            TURNOFF_EVENT },
    { PED_STOPPING,     OFF,            TURNOFF_EVENT },
    { PED_STOP,         OFF,            TURNOFF_EVENT },
    { CAUTION_MODE,     OFF,            TURNOFF_EVENT },
    { JARED_MODE,       OFF,            TURNOFF_EVENT },
    { MANUAL_MODE,      OFF,            TURNOFF_EVENT },

    { CARS_GO,          JARED_MODE,        JARED_MODE_EVENT },
    { CARS_STOPPING,    JARED_MODE,        JARED_MODE_EVENT },
    { CARS_STOP,        JARED_MODE,        JARED_MODE_EVENT },
    { PED_GO,           JARED_MODE,        JARED_MODE_EVENT },
    { PED_STOPPING,     JARED_MODE,        JARED_MODE_EVENT },
    { PED_STOP,         JARED_MODE,        JARED_MODE_EVENT },
    { CAUTION_MODE,     JARED_MODE,        JARED_MODE_EVENT },
    { MANUAL_MODE,      JARED_MODE,        JARED_MODE_EVENT },
    { OFF,              JARED_MODE,        JARED_MODE_EVENT },

    { CARS_GO,          MANUAL_MODE,       MANUAL_MODE_EVENT },
    { CARS_STOPPING,    MANUAL_MODE,       MANUAL_MODE_EVENT },
    { CARS_STOP,        MANUAL_MODE,       MANUAL_MODE_EVENT },
    { PED_GO,           MANUAL_MODE,       MANUAL_MODE_EVENT },
    { PED_STOPPING,     MANUAL_MODE,       MANUAL_MODE_EVENT },
    { PED_STOP,         MANUAL_MODE,       MANUAL_MODE_EVENT },
    { CAUTION_MODE,     MANUAL_MODE,       MANUAL_MODE_EVENT },
    { JARED_MODE,       MANUAL_MODE,       MANUAL_MODE_EVENT },
    { OFF,              MANUAL_MODE,       MANUAL_MODE_EVENT },
};

static state_duration_map_t state_duration_map[] =
{
    { CARS_GO,          CARS_STOPPING,  5000 },
    { CARS_STOPPING,    CARS_STOP,      2000 },
    { CARS_STOP,        PED_GO,         1000 },
    { PED_GO,           PED_STOPPING,   5000 },
    { PED_STOPPING,     PED_STOP,       3000 },
    { PED_STOP,         CARS_GO,        2000 },
};


void traffic_light_init(void)
{
    pinMode(LED_CAR_GREEN, OUTPUT);
    pinMode(LED_CAR_RED, OUTPUT);
    pinMode(LED_CAR_YELLOW, OUTPUT);
    pinMode(LED_PED_GREEN, OUTPUT);
    pinMode(LED_PED_RED, OUTPUT);

    MUTEX_INIT(eventMutex);
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

void traffic_light_send_event(traffic_event_t event)
{
    MUTEX_LOCK(eventMutex);
    current_data.event = event;
    MUTEX_UNLOCK(eventMutex);
}

void traffic_light_send_manual_data(int data)
{
    if (g_current_state != MANUAL_MODE) return;

    changeLights(data);
}

/*======================= STATIC FUNCTIONS =======================*/
void main_traffic_light_loop(void *params)
{
    (void)params;
    traffic_state_t next_state = UNKNOWN;
    unsigned long startTime = 0U;
    unsigned long currentTime = 0U;

    /* Start at CARS_GO state */
    startTime = millis();
    g_current_state = changeState(OFF, UNKNOWN);

    traffic_light_running = true;
    while (traffic_light_running == true)
    {
        MUTEX_LOCK(eventMutex);
        currentTime = millis();
        current_data.timeLapsed = currentTime - startTime;
        next_state = process_state(g_current_state, current_data);
        if (next_state != g_current_state)
        {
            startTime = millis();
            g_current_state = changeState(next_state, g_current_state);
        }
        //Event is consumed
        current_data.event = NO_EVENT;
        MUTEX_UNLOCK(eventMutex);

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

traffic_state_t changeState(traffic_state_t targetState, traffic_state_t currentState)
{
    size_t startActionsSize = sizeof(start_actions) / sizeof(start_actions[0]);
    size_t endActionsSize = sizeof(end_actions) / sizeof(end_actions[0]);
    size_t index = 0U;

    for (index = 0; index < endActionsSize; index++)
    {
        if (end_actions[index].state == currentState)
        {
            if (end_actions[index].action)
            {
                end_actions[index].action();
            }
            break;
        }
    }

    for (index = 0; index < startActionsSize; index++)
    {
        if (start_actions[index].state == targetState)
        {
            if (start_actions[index].action)
            {
                start_actions[index].action();
            }
            break;
        }
    }

    return targetState;
}

traffic_state_t process_state(traffic_state_t current_state, process_data_t data)
{
    size_t duration_map_size = sizeof(state_duration_map) / sizeof(state_duration_map[0]);
    size_t events_map_size = sizeof(state_event_map) / sizeof(state_event_map[0]);
    size_t index = 0U;

    if (data.event != NO_EVENT)
    {
        for (index = 0; index < events_map_size; index++)
        {
            if (state_event_map[index].current_state == current_state)
            {
                if (state_event_map[index].event == data.event)
                {
                    return state_event_map[index].next_state;
                }
            }
        }
    }

    for (index = 0; index < duration_map_size; index++)
    {
        if (state_duration_map[index].current_state == current_state)
        {
            if (state_duration_map[index].state_duration_millis >= data.timeLapsed)
            {
                return current_state;
            }

            return state_duration_map[index].next_state;
        }
    }

    return current_state;
}

void changeLights(int data)
{
// const RED_LIGHT     = 1;  // 00000001 (1)
// const YELLOW_LIGHT  = 2;  // 00000010 (2)
// const GREEN_LIGHT   = 4;  // 00000100 (4)
// const WALK_LIGHT    = 8;  // 00000100 (8)
// const STOP_LIGHT    = 16; // 00001000 (16)

    setLight(LED_CAR_RED, data & 0x01);
    setLight(LED_CAR_YELLOW, data & 0x02);
    setLight(LED_CAR_GREEN, data & 0x04);
    setLight(LED_PED_RED, data & 0x08);
    setLight(LED_PED_GREEN, data & 0x10);

    return;
}

void setLight(uint8_t lightCode, bool isOn)
{
    if (isOn) ON(lightCode);
    else OFF(lightCode);
}

void lights_all_off(void)
{
    OFF(LED_CAR_RED);
    OFF(LED_CAR_YELLOW);
    OFF(LED_CAR_GREEN);
    OFF(LED_PED_RED);
    OFF(LED_PED_GREEN);

    return;
}

void lightOn_only(uint8_t lightCode)
{
    lights_all_off();
    ON(lightCode);
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

    start_blink(LED_PED_GREEN, 300, &blinkingPedWalkHandle);
}
void end_PED_STOPPING(void)
{
    OFF(LED_PED_GREEN);
    stop_blink(blinkingPedWalkHandle);
}

void start_PED_STOP(void)
{
    OFF(LED_CAR_GREEN);
    OFF(LED_CAR_YELLOW);
    ON(LED_CAR_RED);

    ON(LED_PED_RED);
    OFF(LED_PED_GREEN);
}

void start_CAUTION_MODE(void)
{
    lights_all_off();
    start_interval_action(
        cautionLightsON,
        lights_all_off,
        400,
        &cautionModeHandle);
}
void end_CAUTION_MODE(void)
{
    stop_interval_action(cautionModeHandle);
}

void cautionLightsON(void)
{
    ON(LED_CAR_YELLOW);
    ON(LED_PED_RED);
}

void start_JARED_MODE(void)
{
    lights_all_off();
    start_interval_action(
        jared1,
        jared2,
        100,
        &jaredModeHandle);
}
void stop_JARED_MODE(void)
{
    stop_interval_action(jaredModeHandle);
    lights_all_off();
}

void jared1(void)
{
    lightOn_only(LED_CAR_GREEN);
    DELAY(100);
    lightOn_only(LED_CAR_YELLOW);
    DELAY(100);
    lightOn_only(LED_CAR_RED);
    DELAY(100);
    lightOn_only(LED_PED_RED);
}

void jared2(void)
{
    lightOn_only(LED_PED_GREEN);
    DELAY(100);
    lightOn_only(LED_CAR_RED);
    DELAY(100);
    lightOn_only(LED_CAR_YELLOW);
}

void start_MANUAL_MODE(void)
{
    lights_all_off();
}
void stop_MANUAL_MODE(void)
{
    lights_all_off();
}

