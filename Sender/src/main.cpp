#include "main.h"
#include "BleAndWifi.h"

File file;
File root;
bool switchToWiFi = false;

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
  file = root.openNextFile();
  if (shouldSwitchToWifi(file))
    BLE_Sending(file);
  else
  {
    Wifi_setup(file);
    while (!switchToWiFi) {
        Wifi_Transmission();
    }
    BLE_setup();
  }
}