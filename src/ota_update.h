#pragma once

#include <HTTPClient.h>
#include <Arduino.h>
#include "ui/ui.h"
#include <ArduinoJson.h>
#include "version.h"
#include <HTTPUpdate.h>

#define BUTTON_PIN_0 0
#define BUTTON_PIN_14 14
bool isCancel = false;

void performOTAUpdate(String firmware_url)
{
    if (isCancel)
    {
        lv_obj_clear_flag(ui_Button1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_x(ui_Button2, 60);
        lv_label_set_text(ui_Label5, "Update");
        lv_obj_add_flag(ui_Panel4, LV_OBJ_FLAG_HIDDEN);
        lv_handler();
        return;
    }
    Serial.println("Starting OTA Update...");
    WiFiClientSecure client;
    httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    httpUpdate.onProgress([](int current, int total)
                          {
                            int percent = (current * 100) / total;
                            lv_bar_set_value(ui_Bar2, percent, LV_ANIM_OFF);
                            Serial.printf("Progress: %d%%\n", percent);
                            lv_handler(); });
    httpUpdate.onStart([]()
                       {
                           Serial.println("OTA Update started...");
                           lv_obj_clear_flag(ui_Bar2, LV_OBJ_FLAG_HIDDEN);
                           lv_bar_set_value(ui_Bar2, 0, LV_ANIM_OFF);
                           lv_label_set_text(ui_Label3, "OTA Update started...");
                           lv_obj_add_state(ui_Button1, LV_STATE_DISABLED); // Disable the button
                           lv_obj_add_state(ui_Button2, LV_STATE_DISABLED); // Disable the button
                           lv_handler(); });
    httpUpdate.onEnd([]()
                     {
                         Serial.println("OTA Update finished...");
                         lv_label_set_text(ui_Label3, "OTA Update finished...");
                         lv_obj_clear_flag(ui_Button1, LV_STATE_DISABLED); // Enable the button
                         lv_obj_clear_flag(ui_Button2, LV_STATE_DISABLED); // Enable the button
                         lv_obj_clear_flag(ui_Panel5, LV_OBJ_FLAG_HIDDEN);
                         lv_handler(); });

    Serial.println("Starting update...");
    Serial.printf("Firmware URL: %s\n", firmware_url.c_str());
    client.setTimeout(20000); // 20 seconds
    client.setInsecure();     // ✅ Ignore SSL verification
    httpUpdate.update(client, firmware_url);
    HTTPUpdateResult result = httpUpdate.update(client, firmware_url);
    int start = 0;
    switch (result)
    {
    case HTTP_UPDATE_FAILED:
        Serial.printf("Update Failed! Error: %s\n", httpUpdate.getLastErrorString().c_str());
        lv_label_set_text(ui_Label6, "Update Failed!");
        lv_label_set_text(ui_Label7, "Closing ...");
        lv_handler();
        start = millis();
        while (millis() - start < 3000)
        {
            lv_bar_set_value(ui_Bar3, (millis() - start) / 30, LV_ANIM_OFF);
            lv_handler();
        }
        lv_obj_add_flag(ui_Panel5, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_Panel4, LV_OBJ_FLAG_HIDDEN);
        lv_handler();
        break;
    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("No update needed.");
        lv_label_set_text(ui_Label6, "No update needed");
        lv_handler();
        start = millis();
        while (millis() - start < 3000)
        {
            lv_bar_set_value(ui_Bar3, (millis() - start) / 30, LV_ANIM_OFF);
            lv_handler();
        }
        lv_obj_add_flag(ui_Panel5, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_Panel4, LV_OBJ_FLAG_HIDDEN);
        lv_handler();
        break;
    case HTTP_UPDATE_OK:
    {
        Serial.println("Update successful! Restarting...");
        lv_label_set_text(ui_Label6, "Update Done!");

        for (int i = 0; i < 5; i++)
        {
            lv_bar_set_value(ui_Bar3, 20 * (i + 1), LV_ANIM_OFF);
            lv_handler();
            // avoid using delay in loop

            start = millis();
            while (millis() - start < 1000)
                ;
        }
        ESP.restart();
        break;
    }
    }
}

void checkForUpdate()
{
    HTTPClient http;
    http.begin(FIRMWARE_VERSION_URL);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        String payload = http.getString();
        Serial.println("Version info received:");
        Serial.println(payload);

        // Parse JSON
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error)
        {
            Serial.println("JSON Parsing Failed!");
            lv_obj_clear_flag(ui_Panel4, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(ui_Label3, "Error occured !");
            lv_obj_add_flag(ui_Button1, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_x(ui_Button2, 0);
            lv_label_set_text(ui_Label5, "Ok");
            lv_handler();
            int start = millis();
            while (millis() - start < 1000)
                ;
            return;
        }

        String latest_version = doc["latest_version"];
        String firmware_url = doc["firmware_url"];

        Serial.printf("Current version: %s\n", FIRMWARE_VERSION);
        Serial.printf("Latest version: %s\n", latest_version.c_str());

        // Compare Versions
        if (latest_version != FIRMWARE_VERSION)
        {
            lv_obj_clear_flag(ui_Panel4, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(ui_Label3, "New update found!");
            lv_obj_add_state(ui_Button1, LV_STATE_CHECKED); /// States
            isCancel = true;
            lv_handler();
            while (digitalRead(BUTTON_PIN_0) == HIGH)
            {
                if (digitalRead(BUTTON_PIN_14) == LOW)
                {
                    isCancel = !isCancel;
                    Serial.printf("isCancel: %s\n", isCancel ? "true" : "false");
                    if (isCancel)
                    {
                        lv_obj_add_state(ui_Button1, LV_STATE_CHECKED);   /// States
                        lv_obj_clear_state(ui_Button2, LV_STATE_CHECKED); /// States
                    }
                    else
                    {
                        lv_obj_clear_state(ui_Button1, LV_STATE_CHECKED); /// States
                        lv_obj_add_state(ui_Button2, LV_STATE_CHECKED);   /// States
                    }
                }
            }
            performOTAUpdate(firmware_url);
        }
        else
        {
            lv_obj_clear_flag(ui_Panel4, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(ui_Label3, "Device is up to date");
            lv_obj_add_flag(ui_Button1, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_x(ui_Button2, 0);
            lv_label_set_text(ui_Label5, "Ok");
            lv_handler();

            while (digitalRead(BUTTON_PIN_0) != HIGH)
                ;
            lv_obj_clear_flag(ui_Button1, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_x(ui_Button2, 60);
            lv_label_set_text(ui_Label5, "Update");
            lv_obj_add_flag(ui_Panel4, LV_OBJ_FLAG_HIDDEN);
            lv_handler();
            int start = millis();
            while (millis() - start < 1000)
                ;
        }
    }
    else
    {
        Serial.printf("Failed to check update! HTTP Code: %d\n", httpCode);
        lv_obj_clear_flag(ui_Panel4, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ui_Label3, "Update Check Failed!");
        lv_obj_add_flag(ui_Button1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_x(ui_Button2, 0);
        lv_label_set_text(ui_Label5, "Ok"); // ui_Bar2
        lv_handler();
        while (digitalRead(BUTTON_PIN_0) != HIGH)
            ;
        lv_obj_clear_flag(ui_Button1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_x(ui_Button2, 60);
        lv_label_set_text(ui_Label5, "Update");
        lv_obj_add_flag(ui_Panel4, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ui_Label7, "Restarting ...");
        lv_handler();
    }
    http.end();
}
