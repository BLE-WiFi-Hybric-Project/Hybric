#include "main.h"
#include "BleAndWifi.h"

File file;
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
  if (!switchToWiFi)
    BLE_Reciever();
  else
  {
    Wifi_setup();
    Wifi_Reciever();
    BLE_setup();
  }

  delay(1000);
}