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

void traffic_light_init(void);
void traffic_light_start(void);
void traffic_light_stop(void);


#ifdef __cplusplus
}
#endif

#endif /* __TRAFFICLIGHT_H__ */