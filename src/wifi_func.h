#pragma once

#include <WiFiManager.h>
#include "TFT_eSPI.h"
#include "lvgl.h"
#include "qrcode.h"

void connect()
{
    TFT_eSPI tft = TFT_eSPI();
    QRcode qrcode(&tft);
    String ssid = "Internet Clock Wifi"; // Network SSID. Required.
    String security = "nopass";          // "WEP", "WPA", "WPA2", "WPA3" or "nopass" for open
    String password = "";                // Password, ignored if security is "nopass"

    WiFiManager wifiManager;
    wifiManager.setConfigPortalTimeout(5000);
    wifiManager.setCustomHeadElement("<style>form[action='/info'],form[action='/exit'],form[action='/update']{display:none}</style>");

    tft.init();
    tft.setRotation(3);
    qrcode.init();
    qrcode.create("WIFI:S:" + ssid + ";T:" + security + ";P:" + password + ";;");

    wifiManager.setTitle(ssid);
    if (!wifiManager.autoConnect("Internet Clock Wifi", ""))
    {
        delay(3000);
        ESP.restart();
    }
    tft.fillScreen(TFT_BLACK);
}
