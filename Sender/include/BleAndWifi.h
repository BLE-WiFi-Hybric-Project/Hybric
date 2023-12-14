#ifndef INC_BLEANDWIFI_H_
#define INC_BLEANDWIFI_H_

#include "main.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_2 "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"

// BLE
extern BLEServer *pServer;
extern BLECharacteristic *pCharacteristic;
extern BLECharacteristic *pCharacteristic_2;
extern bool deviceConnected;
extern bool oldDeviceConnected;
extern bool sendingFile;
extern BLE2902 *pBLE2902;
extern BLE2902 *pBLE2902_2;
extern size_t lastByteSend;

// WIFI
extern const char *ssid;
extern const char *password;
extern AsyncWebServer server;
extern SemaphoreHandle_t ackSemaphore;

// ACK
extern bool ackReceived;
extern int retransmissionCount;
extern const int MAX_RETRANSMISSIONS; // Set a maximum number of retransmissions
extern String lastAckData;

void BLE_setup();
void BLE_Sending(File fileSending);

void Wifi_setup(File fileSending);
void WifiSending();

bool shouldSwitchToWifi(File fileSending);

#endif
