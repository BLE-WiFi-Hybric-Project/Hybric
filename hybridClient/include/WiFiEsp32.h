#ifndef WiFiEsp32_h
#define WiFiEsp32_h

#include "main.h"
#include <WiFi.h>
#include <WiFiClient.h>

extern const char *ssid;
extern const char *password;
extern const char *serverIP; // SoftAP IP address
extern const int serverPort;

extern const int bufferSize; // Adjust the buffer size as needed
extern bool waitingForAck;
extern size_t lastAckData;

bool ACK(WiFiClient &client);
void wifi_setup();
void wifi_loop();
#endif
