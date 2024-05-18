#include <Arduino.h>
#include <ble_communicator.h>
#include <trafficlight.h>
#include <stdbool.h>

#define ACTION_TURNOFF 0
#define ACTION_NORMALMODE 1
#define ACTION_JAREDMODE 2
#define ACTION_WARNINGMODE 3
#define ACTION_MANUALON 4
#define ACTION_MANUALOFF 17


typedef struct {
  const int data;
  const traffic_event_t event;
} event_data_pair_t;

static const event_data_pair_t event_data_list[] =
{
  { ACTION_TURNOFF,     TURNOFF_EVENT },
  { ACTION_NORMALMODE,  NORMAL_MODE_EVENT },
  { ACTION_JAREDMODE,   JARED_MODE_EVENT },
  { ACTION_WARNINGMODE, CAUTION_MODE_EVENT },
  { ACTION_MANUALON,    MANUAL_MODE_EVENT },
};

static const size_t size_tevent_data_list_size = sizeof(event_data_list) / sizeof(event_data_list[0]);

static bool manual_mode = false;


void ble_data_received(int data)
{
  size_t index = 0;

  if (manual_mode == true)
  {
    if (data == ACTION_MANUALOFF)
    {
      traffic_light_send_event(NORMAL_MODE_EVENT);
      manual_mode = false;
      return;
    }

    traffic_light_send_manual_data(data);
    return;
  }

  for (index = 0; index < size_tevent_data_list_size; index++)
  {
    if (data == event_data_list[index].data)
    {
      traffic_light_send_event(event_data_list[index].event);
      manual_mode = event_data_list[index].event == MANUAL_MODE_EVENT;
      break;
    }
  }

  return;
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