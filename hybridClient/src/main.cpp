#include "main.h"
#include "BleEsp32.h"
#include "WiFiEsp32.h"

File fileSend;
File root;
bool switchToWiFi = false;
int retransmissionCount = 0;
const int MAX_RETRANSMISSIONS = 3;

void shouldSwitchToWifi(File fileSending)
{
  if (switchToWiFi)
    return;

  int size = fileSending.size();
  Serial.println("File size: " + String(size));

  if (size > 30 * 1024)
    switchToWiFi = true;
  else
    switchToWiFi = false;
}

void setup()
{
  Serial.begin(115200);

  // pin Mode:
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);

  // put your setup code here, to run once:
  if (!SPIFFS.begin(true))
  {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  root = SPIFFS.open("/");
  ble_setup();
}

void loop()
{
  fileSend = root.openNextFile();
  if (fileSend)
    shouldSwitchToWifi(fileSend);

  if (!switchToWiFi)
    ble_loop();
  else
  {
    ble_loop();
    wifi_setup();
    while (switchToWiFi)
      wifi_loop();
    ble_setup();
  }
}