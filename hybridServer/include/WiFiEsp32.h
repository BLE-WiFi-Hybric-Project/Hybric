#ifndef WiFiEsp32_h
#define WiFiEsp32_h

#include "main.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

extern const char *ssid;
extern const char *password;
extern AsyncWebServer server;
extern bool ACK;

void wifi_setup();
void wifi_loop();
#endif
