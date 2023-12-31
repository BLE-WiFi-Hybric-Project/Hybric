#ifndef INC_DATA_H_
#define INC_DATA_H_

#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <HTTPClient.h>

extern const char *ssid;
extern const char *password;
extern const char *serverAddress; // Change this to the IP address of your ESP32 server
extern const size_t chunkSize;    // 1KB
extern int retransmissionCount;
extern const int MAX_RETRANSMISSIONS;

extern WiFiClient *wifi;

extern File root;
extern File file;

#endif