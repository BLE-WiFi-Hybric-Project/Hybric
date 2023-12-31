#ifndef WIFIFIX_H
#define WIFIFIX_H

#pragma once
#include "main.h"
#include <lwip/sockets.h>

class WiFiClientFixed : public WiFiClient
{
public:
    void flush() override;
};

#endif // WIFIFIX_H
