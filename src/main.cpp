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
  delay(10*1000);
  traffic_light_stop();
}