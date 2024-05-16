#ifndef __TRAFFICLIGHT_H__
#define __TRAFFICLIGHT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NO_EVENT,
    NORMAL_MODE_EVENT,
    CAUTION_MODE_EVENT,
    TURNOFF_EVENT,
    JARED_MODE_EVENT,
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