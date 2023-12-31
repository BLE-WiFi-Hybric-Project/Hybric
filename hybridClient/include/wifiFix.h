#ifndef WIFIFIX_H
#define WIFIFIX_H

#pragma once
#include "WifiEsp32.h"
#include <lwip/sockets.h>

class WiFiClientFixed : public WiFiClient
{
public:
    void flush() override;
};

#endif // WIFIFIX_H
