#include "main.h"
#include "BleAndWifi.h"

File file;
bool FileisHere = true;
bool switchToWiFi = false;

void setup()
{
  Serial.begin(115200);

  if (!SPIFFS.begin(true))
  {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  BLE_setup();
}

void loop()
{
  if (FileisHere)
  {
    file = SPIFFS.open("/example.txt");
    switchToWiFi = shouldSwitchToWifi(file);
  }
  // Serial.println(switchToWiFi);

  switch (switchToWiFi)
  {
  case false:
    if (!isBLEsetup)
    {
      BLE_setup();
      isBLEsetup = true;
      isWIFIsetup = false;
    }

    if (deviceConnected)
      if (file.available())
      {
        if (sendingFile)
        {
          BLE_Sending(file);
          FileisHere = false;
          switchToWiFi = false;
        }
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

    break;
  case true:
    if (!isWIFIsetup)
    {
      Wifi_setup();
      isWIFIsetup = true;
      isBLEsetup = false;
    }

    if (client.connected())
    {
      if (file.available())
      {
        WifiSending(file);
        FileisHere = false;
        switchToWiFi = false;
      }
    }

    break;
  }

  delay(1000);
}