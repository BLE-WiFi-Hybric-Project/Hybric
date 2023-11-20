#ifndef INC_BLEANDWIFI_H_
#define INC_BLEANDWIFI_H_

#include "main.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>
#include <FS.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

extern unsigned long maxBLEMillis;
extern BLEServer *pServer;
extern BLECharacteristic *pCharacteristic;
extern bool deviceConnected;
extern bool oldDeviceConnected;
extern bool sendingFile;
extern bool isBLEsetup;

extern const char *ssid;
extern const char *password;
extern const char *serverAddress;
extern const int serverPort;
extern WiFiClient client;
extern bool isWIFIsetup;

void BLE_setup();
void BLE_Sending(File fileSending);

void Wifi_setup();
void WifiSending(File fileSending);

bool shouldSwitchToWifi(File file);

#endif
