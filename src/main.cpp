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
  
}