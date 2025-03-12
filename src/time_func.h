#pragma once

#include <Arduino.h>
#include "ui/ui.h"
#include <ESP32Time.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

ESP32Time rtc(0);
const char *ntpServer = "pool.ntp.org";
const char *locationServer = "http://ip-api.com/json?fields=status,city,offset,query";

String getData(String url)
{
    HTTPClient http;
    String payload = "";
    Serial.println(url);
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0)
    {
        payload = http.getString();
    }
    else
    {
        Serial.print("GET request failed: ");
        Serial.println(httpResponseCode);
    }
    http.end();
    Serial.println(payload);
    return payload;
}

void setTime(int tz)
{
    configTime(tz, 0, ntpServer);
    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
        rtc.setTimeStruct(timeinfo);
    }
}

void time_init()
{
    JsonDocument locData;
    deserializeJson(locData, getData(locationServer));
    int timeZone =  locData["offset"] | 0;
    setTime(timeZone);
}

void update_time()
{
    // Update time label
    lv_label_set_text(ui_Label2, rtc.getTime().c_str());
    lv_bar_set_value(ui_Bar1, rtc.getSecond() * 100 / 60, LV_ANIM_OFF);

    // Update date label
    lv_label_set_text(ui_Label1, rtc.getDate().c_str());
}