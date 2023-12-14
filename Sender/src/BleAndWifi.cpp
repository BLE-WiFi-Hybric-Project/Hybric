#include "BleAndWifi.h"
#include "main.h"

// BLE
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
BLECharacteristic *pCharacteristic_2 = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool sendingFile = false;
BLE2902 *pBLE2902;
BLE2902 *pBLE2902_2;
size_t lastByteSend;

// WIFI
const char *ssid = "ESP32";
const char *password = "88888888";
AsyncWebServer server(80);
SemaphoreHandle_t ackSemaphore;

// ACK
bool ackReceived = false;
int retransmissionCount = 0;
const int MAX_RETRANSMISSIONS = 3; // Set a maximum number of retransmissions
String lastAckData;

// Handing BLE Server Call backs
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

// Handing BLE Characteristic Call backs
class MyCharacteristicCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic_2->getValue();
        lastAckData = String(value.c_str());
    }
};

// ACK use in BLE
bool waitForAck()
{
    unsigned long startTime = millis();
    while (millis() - startTime < 3000)
    {
        if (lastAckData != NULL)
        {
            ackReceived = false;
            return true;
        }
        delay(10); // Adjust the delay as needed
    }

    // Timeout, no acknowledgment received
    return false;
}

void BLE_setup()
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

void BLE_Sending(File fileSending)
{
    if (deviceConnected)
    {
        if (sendingFile)
        {
            // Inform client to open file to save
            pCharacteristic_2->setValue(String("1").c_str());
            pCharacteristic_2->notify();

            uint8_t buffer[20];
            // Read and send the file contents
            while (fileSending.available())
            {
                // Read in chunks of 20 bytes
                if (!ackReceived)
                {
                    size_t bytesRead = fileSending.read(buffer, 20);
                    lastByteSend = bytesRead;
                    pCharacteristic->setValue(buffer, bytesRead);
                    pCharacteristic->notify();
                    ackReceived = true;
                }

                // Wait for acknowledgment
                while (!waitForAck())
                {
                    // Handle retransmission
                    retransmissionCount++;
                    if (retransmissionCount <= MAX_RETRANSMISSIONS)
                    {
                        // Retransmit the same chunk
                        pCharacteristic->setValue(buffer, lastByteSend);
                        pCharacteristic->notify();
                    }
                    else
                    {
                        switchToWiFi = true;
                        // Inform client to switch to Wifi
                        pCharacteristic_2->setValue(String("3").c_str());
                        pCharacteristic_2->notify();
                        break;
                    }
                }
                // Reset retransmission count upon successful acknowledgment
                retransmissionCount = 0;
            }

            // Inform client to close file
            pCharacteristic_2->setValue(String("2").c_str());
            pCharacteristic_2->notify();

            // Close the file
            fileSending.close();
            sendingFile = false;
        }
    }

    if (!deviceConnected && oldDeviceConnected)
    {
        delay(500);                  // Give the client time to disconnect
        pServer->startAdvertising(); // Restart advertising
        oldDeviceConnected = deviceConnected;
    }

    if (deviceConnected && !oldDeviceConnected)
        oldDeviceConnected = deviceConnected;
}

void Wifi_setup(File fileSending)
{
    Serial.begin(115200);

    // Start access point
    WiFi.softAP(ssid, password);

    // Create a binary semaphore
    ackSemaphore = xSemaphoreCreateBinary();

    // Print the IP address
    Serial.println("Access Point IP address: " + WiFi.softAPIP().toString());

    // Handle the GET request for ACK
    server.on("/ack", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    xSemaphoreGive(ackSemaphore); // Release the semaphore
    request->send(200, "text/plain", "200"); });

    // Handle root URL
    server.on("/download", HTTP_GET, [fileSending](AsyncWebServerRequest *request)
              {
    if (fileSending) {
      request->send(SPIFFS, fileSending.name(), "text/plain");
      file.close();
    } else {
      request->send(404, "text/plain", "File not found");
    } });

    // Serve other static files (like HTML, CSS, etc.)
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    // Start server
    server.begin();
}

void WifiSending()
{
    // Wait for the semaphore to be given by the callback
    if (xSemaphoreTake(ackSemaphore, portMAX_DELAY))
        switchToWiFi = false;
}

bool shouldSwitchToWifi(File fileSending)
{
    if (switchToWiFi)
        return true;

    int size = (fileSending.size() / 1024.0, 3);

    if (size > 50)
    {
        switchToWiFi = true;
        return true;
    }
    else
        return false;
}