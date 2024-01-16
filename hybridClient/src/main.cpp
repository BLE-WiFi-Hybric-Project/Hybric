#include "main.h"
#include "BleEsp32.h"
#include "WiFiEsp32.h"

File fileSend;
File root;

bool switchToWiFi = false;
bool fileOpenForUpload = true;

int retransmissionCount = 0;
const int MAX_RETRANSMISSIONS = 3;

void shouldSwitchToWifi(File fileSending)
{
  if(!fileSending)
    return;
  
  if (switchToWiFi)
    return;

  int size = fileSending.size();
  Serial.println("File size: " + String(size));

  if (size > 30 * 1024)
    switchToWiFi = true;
  else
    switchToWiFi = false;
  fileBleSend = true;
  fileOpenForUpload = false;
}

void setup()
{
  Serial.begin(115200);

  // pin Mode:
  // pinMode(0, OUTPUT);
  // pinMode(1, OUTPUT);

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
  if(fileOpenForUpload){
    fileSend = root.openNextFile();
    shouldSwitchToWifi(fileSend);
  }
  ble_loop();
  if (switchToWiFi)
  {
    // Inform Server to switch
    while (switchToWiFi)
      ble_loop();

    // Start Wi-Fi
    switchToWiFi = true;
    wifi_setup();
    while (switchToWiFi)
      wifi_loop();

    // Start BLE agian
    ble_setup();
  }

  delay(1000);
}
