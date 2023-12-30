#ifndef WiFiEsp32_h
#define WiFiEsp32_h

#include "main.h"
#include <WiFi.h>
#include <HTTPClient.h>

extern const char *ssid;
extern const char *password;
extern const char *serverIP; // SoftAP IP address

void wifi_setup();
void wifi_loop();
#endif
