#ifndef WiFiEsp32_h
#define WiFiEsp32_h

#include "main.h"
#include <WiFi.h>
#include <WiFiServer.h>

extern const char *ssid;
extern const char *password;
extern WiFiServer server;
extern const int bufferSize; // Adjust the buffer size as needed

void wifi_setup();
void wifi_loop();
#endif
