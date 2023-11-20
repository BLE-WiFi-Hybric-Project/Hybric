#include "BleAndWifi.h"
#include "main.h"

unsigned long maxBLEMillis = 10000;
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool sendingFile = false;
bool isBLEsetup = false;

const char *ssid = "FREE-WIFI";
const char *password = "20012005";
const char *serverAddress = "192.168.1.1";
const int serverPort = 8888;
WiFiClient client;
bool isWIFIsetup = false;

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        sendingFile = true;
    };

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
    }
};

void BLE_setup()
{
    BLEDevice::init("ESP32");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    pCharacteristic->addDescriptor(new BLE2902());

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);

    pService->start();
    pAdvertising->start();

    Serial.println("Waiting for a client connection...");
    isBLEsetup = true;
}

void BLE_Sending(File fileSending)
{
    if (!fileSending)
    {
        Serial.println("Failed to open file");
        return;
    }
    // unsigned long startMillis = millis();

    // Read and send the file contents
    while (fileSending.available())
    {
        uint8_t buffer[251]; // Read in chunks of 251 bytes
        size_t bytesRead = fileSending.read(buffer, 251);
        pCharacteristic->setValue(buffer, bytesRead);
        pCharacteristic->notify();
        delay(3); // Add a small delay to avoid buffer overflow
    }

    Serial.println("Finish Sending");
    // Close the file
    fileSending.close();
}

void Wifi_setup()
{
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting...");
    }
    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());

    // Kết nối đến máy chủ (ESP32 #1)
    if (client.connect(serverAddress, serverPort))
    {
        Serial.println("Connected to server");
        isWIFIsetup = true;
    }
    else
        Serial.println("Connection to server failed");
}

void WifiSending(File fileSending)
{
    if (!fileSending)
    {
        Serial.println("Failed to open file");
        client.stop();
        return;
    }

    while (fileSending.available())
    {
        uint8_t buffer[1460];
        size_t bytesRead = fileSending.read(buffer, 1460);
        client.write(buffer, bytesRead);
    }

    fileSending.close();
    client.stop();
}

bool shouldSwitchToWifi(File file)
{
    return file.size() > 50 * 1024 ? true : false;
}