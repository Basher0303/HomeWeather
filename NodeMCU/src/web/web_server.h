#pragma once
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "settings.h"

class WebServer
{
public:
    WebServer(char *ssid, char *password, int port, EEPROMSettings &settings);
    void begin();
    void stop();
    void handleClient();

private:
    char *_ssid;
    char *_password;
    int _port;
    EEPROMSettings &_settings;
    WiFiSettings &_wifiSettings;

    ESP8266WebServer *_server;
    IPAddress _localIp;
    IPAddress _gateway;
    IPAddress _subnet;

    void _handleGetMainPage();
    void _handlePostSettings();
};
