#include "main.h"
#include "BleAndWifi.h"

File file;
File root;
bool switchToWiFi = false;
bool failTranmission = false;

void setup()
{
  Serial.begin(115200);

  if (!SPIFFS.begin(true))
  {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  root = SPIFFS.open("/");

  BLE_setup();
}

void loop()
{
  if (!failTranmission)
    file = root.openNextFile();

  if (file.available())
  {
    Serial.println(file.name());
    if (!shouldSwitchToWifi(file))
    {
      while (sendingFile)
        BLE_Sending(file);
    }
    else
    {
      informCilent(pCharacteristic_2, "3");
      Wifi_setup(file);
      while (switchToWiFi)
        WifiSending();
      BLE_setup();
    }
  }
}