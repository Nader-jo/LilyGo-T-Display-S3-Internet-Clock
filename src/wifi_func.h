#pragma once

#include <WiFiManager.h>
#include "lvgl.h"


void connect()
{
    WiFiManager wifiManager;
    wifiManager.setConfigPortalTimeout(5000);
    wifiManager.setCustomHeadElement("<style>form[action='/info'],form[action='/exit'],form[action='/update']{display:none}</style>");
    wifiManager.setTitle("Internet Clock Wifi Setup");
    if (!wifiManager.autoConnect("Internet Clock Wifi", ""))
    {
        delay(3000);
        ESP.restart();
    }
}
