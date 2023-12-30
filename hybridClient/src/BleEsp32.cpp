#include "BleEsp32.h"

// Use for sending file
const int chunkSize = 500;
int lastByteSent;
bool ackReceived = false;

// Define pointer for the BLE connection
BLERemoteCharacteristic *pRemoteChar;
BLERemoteCharacteristic *pRemoteChar_2;

class MyClientCallback : public BLEClientCallbacks
{
    void onConnect(BLEClient *pclient) {}

    void onDisconnect(BLEClient *pclient)
    {
        connected = false;
        Serial.println("onDisconnect");
    }
};

// Funtion to inform Server
void informServer(String txValue)
{
    pRemoteChar_2->writeValue(txValue.c_str(), txValue.length());
}

// Funtion to wait for ACK
bool waitForAck()
{
    unsigned long startTime = millis();
    while (millis() - startTime < 3000)
    {
        if (ackReceived)
        {
            ackReceived = false;
            return true;
        }
        delay(1); // Adjust the delay as needed
    }

    // Timeout, no acknowledgment received
    return false;
}

// Function to open the file for reading
void openFileForReading()
{
    fileSend = root.openNextFile();
    if (!fileSend)
        Serial.println("Failed to open file for reading");
    else
        Serial.println("File opened for reading");
}

void readAndSendFileChunk()
{
    while (fileSend.available())
    {
        char data[chunkSize];
        informServer("open");

        if (!ackReceived)
        {
            int bytesRead = fileSend.readBytes(data, chunkSize);
            lastByteSent = bytesRead;
            pRemoteChar->writeValue(data, bytesRead);
        }

        while (!waitForAck())
        {
            retransmissionCount++;
            if (retransmissionCount <= MAX_RETRANSMISSIONS)
            {
                // Retransmit the same chunk
                Serial.println("Retransmitting chunk...");
                pRemoteChar->writeValue(data, lastByteSent);
            }
            else
            {
                Serial.println("Max retransmissions reached. Aborting.");
                informServer("switch");
                fileSend.close();
                switchToWiFi = true;
                return;
            }
        }

        retransmissionCount = 0;
    }

    // File reading is complete, close the file
    informServer("close");
    fileSend.close();
    Serial.println("File reading complete");
}

// Callback function for Notify function
static void notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                           uint8_t *pData,
                           size_t length,
                           bool isNotify)
{
    if (pBLERemoteCharacteristic->getUUID().toString() == charUUID.toString())
    {
        uint32_t counter = pData[0];
        for (int i = 1; i < length; i++)
            counter = counter | (pData[i] << i * 8);

        // Serial.println(counter);
        if (counter == 49)
            ackReceived = true;
    }
}

// Function to chech Characteristic
bool connectCharacteristic(BLERemoteService *pRemoteService, BLERemoteCharacteristic *BLERemoteChar)
{
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    if (BLERemoteChar == nullptr)
    {
        Serial.print("Failed to find one of the characteristics");
        Serial.print(BLERemoteChar->getUUID().toString().c_str());
        return false;
    }
    Serial.println(" - Found characteristic: " + String(BLERemoteChar->getUUID().toString().c_str()));

    if (BLERemoteChar->canNotify())
        BLERemoteChar->registerForNotify(notifyCallback);

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

void ble_setup()
{
    Serial.begin(115200);
    Serial.println("Starting Arduino BLE Client application...");
    BLEDevice::init("BLE client");
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
    Serial.println("Done Setup");
}

void ble_loop()
{
    if (doConnect == true)
    {
        if (connectToServer())
        {
            digitalWrite(0, HIGH);
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
        if (switchToWiFi)
            return;

        openFileForReading();

        if (fileSend)
        {
            digitalWrite(1, HIGH);
            readAndSendFileChunk();
            digitalWrite(1, HIGH);
        }
    }
    else if (doScan)
        BLEDevice::getScan()->start(0);

    delay(1000);
}