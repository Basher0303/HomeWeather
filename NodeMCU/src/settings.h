#pragma once
#include <Arduino.h>
#include <string.h>

struct WiFiSettings
{
    char ssid[32];
    char password[32];
};

struct WebSettings
{
    char apiUrl[128];
    char apiKey[128];
};

struct EEPROMSettings
{
    static constexpr uint32_t MAGIC = 0x484D5754UL; // HMWT

    uint32_t magic;
    WiFiSettings wifiSettings;
    WebSettings webSettings;
};

inline void initDefaultSettings(EEPROMSettings &settings)
{
    settings.magic = EEPROMSettings::MAGIC;
    memset(settings.wifiSettings.ssid, 0, sizeof(settings.wifiSettings.ssid));
    memset(settings.wifiSettings.password, 0, sizeof(settings.wifiSettings.password));
    memset(settings.webSettings.apiUrl, 0, sizeof(settings.webSettings.apiUrl));
    memset(settings.webSettings.apiKey, 0, sizeof(settings.webSettings.apiKey));

    strcpy(settings.wifiSettings.ssid, "");
    strcpy(settings.wifiSettings.password, "");
    strcpy(settings.webSettings.apiUrl, "");
    strcpy(settings.webSettings.apiKey, "");
}
