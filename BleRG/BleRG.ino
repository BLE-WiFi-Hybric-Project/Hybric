#include "BLEDevice.h"

// Define UUIDs:
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
static BLEUUID charUUID_2("1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e");

// Some variables to keep track of the device connection
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
boolean sendAck = false;

// Define pointer for the BLE connection
static BLEAdvertisedDevice* myDevice;
BLERemoteCharacteristic* pRemoteChar;
BLERemoteCharacteristic* pRemoteChar_2;

const int WINDOW_SIZE = 3;
int expectedSeqNum = 0;

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {}

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

void sendAckNow() {
  String txValue = "1";
  pRemoteChar_2->writeValue(txValue.c_str(), txValue.length());
}

// Callback function for Notify function
static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                            uint8_t* pData,
                            size_t length,
                            bool isNotify) {
  if(pBLERemoteCharacteristic->getUUID().toString() == charUUID.toString()) {

    // convert received bytes to integer
    uint32_t counter = pData[0];
    for(int i = 1; i<length; i++) {
      counter = counter | (pData[i] << i*8);
    }

    // print to Serial
    Serial.print("Characteristic 1 (Notify) from server: ");
    Serial.println(counter );
    
    sendAck = true;  
  }
}

// Function to chech Characteristic
bool connectCharacteristic(BLERemoteService* pRemoteService, BLERemoteCharacteristic* l_BLERemoteChar) {
  // Obtain a reference to the characteristic in the service of the remote BLE server.
  if (l_BLERemoteChar == nullptr) {
    Serial.print("Failed to find one of the characteristics");
    Serial.print(l_BLERemoteChar->getUUID().toString().c_str());
    return false;
  }
  Serial.println(" - Found characteristic: " + String(l_BLERemoteChar->getUUID().toString().c_str()));

  if(l_BLERemoteChar->canNotify())
    l_BLERemoteChar->registerForNotify(notifyCallback);

  return true;
}

// Function that is run whenever the server is connected
bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());
  
  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  connected = true;
  pRemoteChar = pRemoteService->getCharacteristic(charUUID);
  pRemoteChar_2 = pRemoteService->getCharacteristic(charUUID_2);
  if(connectCharacteristic(pRemoteService, pRemoteChar) == false)
    connected = false;
  else if(connectCharacteristic(pRemoteService, pRemoteChar_2) == false)
    connected = false;

  if(connected == false) {
    pClient-> disconnect();
    Serial.println("At least one characteristic UUID not found");
    return false;
  }
  return true;
}

// Scan for BLE servers and find the first one that advertises the service we are looking for.
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  //Called for each advertising BLE server.
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());
  
    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
  
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
  
    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("BLE client");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  Serial.println("Done Setup");
}

void loop() {
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("Connected to the BLE Server.");
      connected = true;
    } else {
      Serial.println("Failed to connect to the server.");
      connected = false;
    }
    doConnect = false;
  }

  if (connected) {
    // Do something when connected
    if(sendAck){
      sendAckNow();
      sendAck = false;
    }
  } else if (doScan) {
    BLEDevice::getScan()->start(0);
  }

  delay(1000);
}
