#ifndef INC_DATA_H_
#define INC_DATA_H_

#include <Arduino.h>
#include <SPIFFS.h>

extern File fileSend;
extern File root;
extern bool switchToWiFi;
extern bool fileOpenForUpload;
extern int retransmissionCount;
extern const int MAX_RETRANSMISSIONS;

void shouldSwitchToWifi(File fileSending);

#endif
