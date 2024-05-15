#include <Arduino.h>
#include <stdbool.h>

#include "ble_communicator.h"
#include "common_functions.h"

static TaskHandle_t connectionBlinkHandle = NULL;

static void ble_connectingMode(void);

void ble_comm_init(void)
{
    pinMode(LED_INDICATOR_CONNECTED, OUTPUT);
    pinMode(LED_INDICATOR_ERROR, OUTPUT);

    ble_connectingMode();
}

void ble_comm_deinit(void)
{

}

/* INTERNAL FUNCTIONS */
void ble_connectingMode(void)
{
    start_blink(LED_INDICATOR_CONNECTED, 500, &connectionBlinkHandle);
}
