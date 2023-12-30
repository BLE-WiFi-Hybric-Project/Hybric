#ifndef BleEsp32_h
#define BleEsp32_h

#include "main.h"
#include <BLEDevice.h>

// Define UUIDs:
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
static BLEUUID charUUID_2("1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e");

// Some variables to keep track of the device connection
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;

// Use for sending file
extern const int chunkSize;
extern int lastByteSent;
extern bool ackReceived;
extern bool signalSwitch;

// Define pointer for the BLE connection
static BLEAdvertisedDevice *myDevice;
extern BLERemoteCharacteristic *pRemoteChar;
extern BLERemoteCharacteristic *pRemoteChar_2;

void informServer(String txValue);
bool waitForAck();
void readAndSendFileChunk();
bool connectCharacteristic(BLERemoteService *pRemoteService, BLERemoteCharacteristic *BLERemoteChar);
bool connectToServer();
void ble_setup();
void ble_loop();

#endif