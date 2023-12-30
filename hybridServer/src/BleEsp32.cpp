#include "BleEsp32.h"

// Global variables
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
BLECharacteristic *pCharacteristic_2 = NULL;
BLE2902 *pBLE2902;
BLE2902 *pBLE2902_2;

// Some variables to keep track of the device connection
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Global variables for ACK
bool sendACK = false;
bool signalSwitch = false;

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
    }

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
};

// Function to process received data
void processReceivedData(const char *data, int length)
{
    // Process the received data (e.g., save to a file)
    receivedFile.write(reinterpret_cast<const uint8_t *>(data), length);
    Serial.println(receivedFile.size());
}

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string uuid = pCharacteristic->getUUID().toString();
        if (uuid == CHARACTERISTIC_UUID_2)
        {
            String info = String(pCharacteristic_2->getValue().c_str());
            if (info == "open")
                receivedFile = SPIFFS.open("/example1.txt", FILE_APPEND);
            else if (info == "close")
            {
                Serial.println(receivedFile.size());
                receivedFile.close();
                SPIFFS.remove("/example1.txt");
            }
            else if (info == "switch")
            {
                receivedFile.close();
                switchToWiFi = true;
                signalSwitch = true;
            }
        }
        else if (uuid == CHARACTERISTIC_UUID)
        {
            sendACK = true;
            std::string value = pCharacteristic->getValue();
            processReceivedData(value.c_str(), value.length());
        }
    }
};

void ble_setup()
{
    BLEDevice::init("ESP32");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_WRITE);

    pCharacteristic_2 = pService->createCharacteristic(
        CHARACTERISTIC_UUID_2,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY);

    pBLE2902 = new BLE2902();
    pBLE2902->setNotifications(true);
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
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
    pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
    Serial.println("Waiting a client connection to notify...");
}

void ble_loop()
{
    if (deviceConnected)
    {
        if(signalSwitch){
            String Si = "2";
            pCharacteristic->setValue(Si.c_str());
            pCharacteristic->notify();
            signalSwitch = false;
        }
        
        if (sendACK)
        {
            // Serial.println("Send ACK");
            String ACK = "1";
            pCharacteristic->setValue(ACK.c_str());
            pCharacteristic->notify();
            sendACK = false;
        }
        
        if (switchToWiFi)
            return;
    }

    if (!deviceConnected && oldDeviceConnected)
    {
        delay(500);                  // Give the client time to disconnect
        pServer->startAdvertising(); // Restart advertising
        Serial.println("Waiting for a client connection...");
        oldDeviceConnected = deviceConnected;
    }

    if (deviceConnected && !oldDeviceConnected)
    {
        // do stuff here on connection
        oldDeviceConnected = deviceConnected;
    }

    // delay(1000);
}
