#ifndef __TRAFFICLIGHT_H__
#define __TRAFFICLIGHT_H__

#define LED_CAR_RED D8
#define LED_CAR_YELLOW D9
#define LED_CAR_GREEN D10
#define LED_PED_RED D5
#define LED_PED_GREEN D4

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NO_EVENT,
    NORMAL_MODE_EVENT,
    CAUTION_MODE_EVENT,
    TURNOFF_EVENT,
    UNKNOWN_EVENT,
} traffic_event_t;

void traffic_light_init(void);
void traffic_light_start(void);
void traffic_light_stop(void);
void traffic_light_send_event(traffic_event_t event);


#ifdef __cplusplus
}
#endif

#endif /* __TRAFFICLIGHT_H__ */