#include <Arduino.h>
#include <ble_communicator.h>
#include <trafficlight.h>

void ble_data_received(int data)
{
  if (data == 0)
  {
    traffic_light_send_event(TURNOFF_EVENT);
  }
  else if (data == 1)
  {
    traffic_light_send_event(NORMAL_MODE_EVENT);
  }
  else
  {
    traffic_light_send_event(CAUTION_MODE_EVENT);
  }
}

void setup()
{
  ble_comm_init(ble_data_received);
  traffic_light_init();
  traffic_light_start();
}

void loop()
{
}