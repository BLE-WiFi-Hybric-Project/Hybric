#include "main.h"
#include "BleAndWifi.h"

// Define pointer for the BLE connection
BLERemoteCharacteristic *pRemoteChar;
BLERemoteCharacteristic *pRemoteChar_2;

// ACK
boolean sendAck = false;
boolean openFile = false;
boolean closeFile = false;

// Wifi Setup
const char *ssid = "ESP32";
const char *password = "88888888";
const char *serverIP = "192.168.4.1";

class MyClientCallback : public BLEClientCallbacks
{
    void onConnect(BLEClient *pclient) {}

    void onDisconnect(BLEClient *pclient)
    {
        connected = false;
        Serial.println("onDisconnect");
    }
};

void sendAckNow()
{
    String txValue = "1";
    pRemoteChar_2->writeValue(txValue.c_str(), txValue.length());
}

// Callback function for Notify function
static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                           uint8_t *pData,
                           size_t length,
                           bool isNotify)
{
    if (pBLERemoteCharacteristic->getUUID().toString() == charUUID.toString())
    {
        file.write(pData, length);
        sendAck = true;
    }
    else if (pBLERemoteCharacteristic->getUUID().toString() == charUUID_2.toString())
    {
        uint32_t counter = pData[0];
        for (int i = 1; i < length; i++)
            counter = counter | (pData[i] << i * 8);

        if (counter == 49)
            openFile = true;
        else if (counter == 50)
            closeFile = true;
        else
            switchToWiFi = true;
    }
}

// Function to chech Characteristic
bool connectCharacteristic(BLERemoteService *pRemoteService, BLERemoteCharacteristic *l_BLERemoteChar)
{
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    if (l_BLERemoteChar == nullptr)
    {
        Serial.print("Failed to find one of the characteristics");
        Serial.print(l_BLERemoteChar->getUUID().toString().c_str());
        return false;
    }
    Serial.println(" - Found characteristic: " + String(l_BLERemoteChar->getUUID().toString().c_str()));

    if (l_BLERemoteChar->canNotify())
        l_BLERemoteChar->registerForNotify(notifyCallback);

    return true;
}

// Function that is run whenever the server is connected
bool connectToServer()
{
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());

    BLEClient *pClient = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr)
    {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(serviceUUID.toString().c_str());
        pClient->disconnect();
        return false;
    }
    Serial.println(" - Found our service");

    connected = true;
    pRemoteChar = pRemoteService->getCharacteristic(charUUID);
    pRemoteChar_2 = pRemoteService->getCharacteristic(charUUID_2);
    if (connectCharacteristic(pRemoteService, pRemoteChar) == false)
        connected = false;
    else if (connectCharacteristic(pRemoteService, pRemoteChar_2) == false)
        connected = false;

    if (connected == false)
    {
        pClient->disconnect();
        Serial.println("At least one characteristic UUID not found");
        return false;
    }
    return true;
}

// Scan for BLE servers and find the first one that advertises the service we are looking for.
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    // Called for each advertising BLE server.
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        Serial.print("BLE Advertised Device found: ");
        Serial.println(advertisedDevice.toString().c_str());

        // We have found a device, let us now see if it contains the service we are looking for.
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
        {
            BLEDevice::getScan()->stop();
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
            doScan = true;
        } // Found our server
    }     // onResult
};        // MyAdvertisedDeviceCallbacks

void BLE_setup()
{
    BLEDevice::init("BLE client");
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
}

void BLE_Reciever()
{
    if (doConnect == true)
    {
        if (connectToServer())
        {
            Serial.println("Connected to the BLE Server.");
            connected = true;
        }
        else
        {
            Serial.println("Failed to connect to the server.");
            connected = false;
        }
        doConnect = false;
    }

    if (connected)
    {
        // Do something when connected
        if (sendAck)
        {
            sendAckNow();
            sendAck = false;
            delay(1);
        }

        // Open file
        if (openFile)
        {
            file = SPIFFS.open("/recieved_data.txt", FILE_WRITE);
            openFile = false;
        }

        // CLose File
        if (closeFile)
        {
            file.close();
            closeFile = false;
        }

        // Switch to Wifi
        if (switchToWiFi)
        {
            if (file != NULL)
                file.close();
            return;
        }
    }
    else if (doScan)
        BLEDevice::getScan()->start(0);
}

void Wifi_setup()
{
    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print("...");
    }
    Serial.println("\nConnected to WiFi");
}

void Wifi_Reciever()
{
    // Send a GET request to the server to download the file
    HTTPClient http;
    String url = "http://" + String(serverIP) + "/download";
    String url2 = "http://" + String(serverIP) + "/ack";

    Serial.println("Sending GET request to: " + url);

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0)
    {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK)
        {
            // Save received data to a file on the client
            file = SPIFFS.open("/recieved_data.txt", FILE_WRITE);
            if (file)
                http.writeToStream(&file);
            else
                Serial.println("Error opening file for writing");

            // Send ACK to Server
            http.begin(url2);
            int httpCode2 = http.GET();
            if (httpCode2 > 0)
                Serial.println("Send ACK");

            switchToWiFi = false;
        }
        else
            Serial.printf("Failed to download file, error: %s\n", http.errorToString(httpCode).c_str());
    }
    else
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
}
