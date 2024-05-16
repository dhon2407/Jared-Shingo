#ifndef __TRAFFICLIGHT_INTERNAL_H__
#define __TRAFFICLIGHT_INTERNAL_H__

#include "trafficlight.h"

#define LED_CAR_RED D8
#define LED_CAR_YELLOW D9
#define LED_CAR_GREEN D10
#define LED_PED_RED D5
#define LED_PED_GREEN D4

#define ON(light)   digitalWrite(light, HIGH)
#define OFF(light)  digitalWrite(light, LOW)
#define MAIN_LOOP_TICK (10  / portTICK_PERIOD_MS)
#define DELAY(ms) vTaskDelay(ms / portTICK_PERIOD_MS)

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CARS_GO,
    CARS_STOPPING,
    CARS_STOP,
    PED_GO,
    PED_STOPPING,
    PED_STOP,
    OFF,
    JARED_MODE,
    CAUTION_MODE,
    UNKNOWN,
} traffic_state_t;

typedef void (*on_start_state_callback)(void);

typedef struct
{
    traffic_state_t state;
    on_start_state_callback action;
} state_action_map_t;

typedef struct
{
    traffic_state_t current_state;
    traffic_state_t next_state;
    unsigned long state_duration_millis;
} state_duration_map_t;

typedef struct
{
    traffic_state_t current_state;
    traffic_state_t next_state;
    traffic_event_t event;
} state_event_map_t;

typedef struct
{
    traffic_event_t event;
    unsigned long timeLapsed;
} process_data_t;

/*======================= STATIC FUNCTIONS =======================*/
static void main_traffic_light_loop(void *params);
static traffic_state_t changeState(traffic_state_t targetState, traffic_state_t currentState);
static traffic_state_t process_state(traffic_state_t current_state, process_data_t data);

static void start_CARS_GO(void);
static void start_CARS_STOPPING(void);
static void start_CARS_STOP(void);
static void start_PED_GO(void);
static void start_PED_STOPPING(void);
static void start_PED_STOP(void);
static void start_CAUTION_MODE(void);

static void end_PED_STOPPING(void);
static void end_CAUTION_MODE(void);

static void cautionLightsON(void);
static void lights_all_off(void);
static void lightOn_only(uint8_t);

static void start_JARED_MODE(void);
static void stop_JARED_MODE(void);
static void jared1(void);
static void jared2(void);


#ifdef __cplusplus
}
#endif

#endif /* __TRAFFICLIGHT_INTERNAL_H__ */