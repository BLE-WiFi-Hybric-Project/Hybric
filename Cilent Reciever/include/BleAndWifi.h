#ifndef INC_BLEANDWIFI_H_
#define INC_BLEANDWIFI_H_

#include "main.h"
#include <BLEDevice.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Define UUIDs:
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
static BLEUUID charUUID_2("1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e");

// Some variables to keep track of the device connection
static boolean doConnect;
static boolean connected;
static boolean doScan;

// Define pointer for the BLE connection
static BLEAdvertisedDevice *myDevice;
extern BLERemoteCharacteristic *pRemoteChar;
extern BLERemoteCharacteristic *pRemoteChar_2;

// WIFI
extern const char *ssid;
extern const char *password;
extern const char *serverIP;

// ACK
extern boolean sendAck;
extern boolean openFile;
extern boolean closeFile;

// BLE funtion
void sendAckNow();
bool connectCharacteristic(BLERemoteService *pRemoteService, BLERemoteCharacteristic *l_BLERemoteChar);
bool connectToServer();
void BLE_setup();
void BLE_Reciever();

// Wifi funtion
void Wifi_setup();
void Wifi_Reciever();

#endif
