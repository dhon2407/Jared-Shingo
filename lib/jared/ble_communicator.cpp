#include <stdbool.h>
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "ble_communicator.h"
#include "common_functions.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

static task_handle_t *connectionBlinkHandle = NULL;

static TaskHandle_t main_loop_task_handle = NULL;
static volatile bool main_loop_active = false;

static BLEServer* pServer = NULL;
static BLECharacteristic* pCharacteristic = NULL;
static volatile bool deviceConnected = false;
static bool oldDeviceConnected = false;

static void ble_connectingMode(bool connected);
static void main_ble_loop(void *params);

class MyServerCallbacks: public BLEServerCallbacks
{
    void onConnect(BLEServer* pServer)
    {
        deviceConnected = true;
        ble_connectingMode(deviceConnected);
    };

    void onDisconnect(BLEServer* pServer)
    {
        deviceConnected = false;
        ble_connectingMode(deviceConnected);
    }

};

class MyCallbacks: public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0)
      {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);

        Serial.println();
        Serial.println("*********");
      }
    }
};


void ble_comm_init(void)
{
    pinMode(LED_INDICATOR_CONNECTED, OUTPUT);
    pinMode(LED_INDICATOR_ERROR, OUTPUT);

    BLEDevice::init("Jared Shingou");
    Serial.begin(115200);

    // Create the BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *pService = pServer->createService(SERVICE_UUID);

    // Create a BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID,
                        BLECharacteristic::PROPERTY_READ   |
                        BLECharacteristic::PROPERTY_NOTIFY |
                        BLECharacteristic::PROPERTY_WRITE
                        );

    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->setValue("Initial value..");

    // Start the service
    pService->start();

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();

    ble_connectingMode(deviceConnected);

    xTaskCreate(main_ble_loop, "main_ble_loop", 1000, NULL, 1, &main_loop_task_handle);
}

void ble_comm_deinit(void)
{

}

/* INTERNAL FUNCTIONS */
void ble_connectingMode(bool connected)
{
    if (connected == true)
    {
      stop_blink(connectionBlinkHandle);
      ON(LED_INDICATOR_CONNECTED);
    }
    else
    {
      start_blink(LED_INDICATOR_CONNECTED, 250, &connectionBlinkHandle);
    }
}

static void main_ble_loop(void *params)
{
    main_loop_active = true;
    while (main_loop_active == true)
    {
      // disconnecting
      if (!deviceConnected && oldDeviceConnected)
      {
          vTaskDelay(1000 / portTICK_PERIOD_MS); // give the bluetooth stack the chance to get things ready
          pServer->startAdvertising(); // restart advertising
          oldDeviceConnected = deviceConnected;
      }
      // connecting
      if (deviceConnected && !oldDeviceConnected)
      {
          // do stuff here on connecting
          oldDeviceConnected = deviceConnected;
      }

      vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

