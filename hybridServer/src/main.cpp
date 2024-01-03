#include "main.h"
#include "BleEsp32.h"
#include "WiFiEsp32.h"

File receivedFile;
File root;
bool switchToWiFi = false;

void setup()
{
  Serial.begin(115200);

  // Initialize SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  ble_setup();
}

void loop()
{
  if (switchToWiFi)
  {
    wifi_setup();
    while (switchToWiFi)
      wifi_loop();
    ble_setup();
  }
  ble_loop();
}
