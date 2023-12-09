#include <SPIFFS.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pCharacteristic_2 = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool sendingFile = false;

#define SERVICE_UUID          "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_2 "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"
BLE2902 *pBLE2902;
BLE2902 *pBLE2902_2;

File file;
size_t lastBytesSent = 0;
bool waitingForAck = false;

// Global variables for ACK handling and retransmission
volatile bool ackReceived = false;
int retransmissionCount = 0;
const int MAX_RETRANSMISSIONS = 3; // Set a maximum number of retransmissions
String lastAckData;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    sendingFile = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic_2->getValue();
        lastAckData = String(value.c_str());
        waitingForAck = false;
    }
};

bool waitForAck() {
    // Wait for acknowledgment (timeout after 5 seconds)
    unsigned long startTime = millis();
    while (millis() - startTime < 5000) {
        if (lastAckData != NULL) {
            waitingForAck = false;
            return true;
        }
        delay(10);  // Adjust the delay as needed
    }

    // Timeout, no acknowledgment received
    return false;
}

void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  File root = SPIFFS.open("/");
  file = root.openNextFile();

  BLEDevice::init("ESP32");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pCharacteristic_2 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_2,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |                      
                      BLECharacteristic::PROPERTY_NOTIFY
                    ); 
      
  pBLE2902 = new BLE2902();
  pBLE2902->setNotifications(true);
  pCharacteristic->addDescriptor(pBLE2902);

  pBLE2902_2 = new BLE2902();
  pBLE2902_2->setNotifications(true);
  pCharacteristic_2->setCallbacks(new MyCharacteristicCallbacks());
  pCharacteristic_2->addDescriptor(pBLE2902_2);

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    Serial.println("Client connectted.");
    if (sendingFile) {
      // Open the file
      if (!file) {
        Serial.println("Failed to open file");
        return;
      }
      Serial.println("Start");
      // Read and send the file contents
      while (file.available()) {
        if(!waitingForAck) {
          uint8_t buffer[20]; // Read in chunks of 251 bytes
          size_t bytesRead = file.read(buffer, 20);
          lastBytesSent = bytesRead;
  
          pCharacteristic->setValue(buffer, bytesRead);
          pCharacteristic->notify();
          waitingForAck = true;
        }

        // Wait for acknowledgment
        while (!waitForAck()) {
           // Handle retransmission
           retransmissionCount++;
           if (retransmissionCount <= MAX_RETRANSMISSIONS) {
              // Retransmit the same chunk
              Serial.println("Retransmitting chunk...");
              pCharacteristic->setValue(buffer, lastBytesSent);
              pCharacteristic->notify();
              continue;
           } else {
              Serial.println("Max retransmissions reached. Aborting.");
              break;
           }
        }

        // Check the received ACK data before continuing
        if (lastAckData != "1") {
           // ACK indicates an error or other condition, handle accordingly
           Serial.println("Received non-1 ACK. Aborting.");
           break;
        }
        // Reset retransmission count upon successful acknowledgment
        retransmissionCount = 0;
      }

      // Close the file
      file.close();
      sendingFile = false;
      Serial.println("Finish");
    }
  }

  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Give the client time to disconnect
    pServer->startAdvertising(); // Restart advertising
    Serial.println("Waiting for a client connection...");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connection
    oldDeviceConnected = deviceConnected;
  }

  delay(1000);
}
