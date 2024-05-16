#include <Arduino.h>
#include <ble_communicator.h>
#include <trafficlight.h>

void setup()
{
  ble_comm_init();
  traffic_light_init();

  traffic_light_start();
}

void loop()
{
  delay(30*1000);
  traffic_light_send_event(CAUTION_MODE_EVENT);
  delay(30*1000);
  traffic_light_send_event(NORMAL_MODE_EVENT);
}