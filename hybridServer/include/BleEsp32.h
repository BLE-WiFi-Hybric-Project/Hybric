#ifndef BleEsp32_h
#define BleEsp32_h

#include "main.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Global variables
extern BLEServer *pServer;
extern BLECharacteristic *pCharacteristic;
extern BLECharacteristic *pCharacteristic_2;
extern BLE2902 *pBLE2902;
extern BLE2902 *pBLE2902_2;

// Some variables to keep track of the device connection
extern bool deviceConnected;
extern bool oldDeviceConnected;

// Define UUIDs:
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_2 "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"

void processReceivedData(const char *data, int length);
void ble_setup();
void ble_loop();

#endif