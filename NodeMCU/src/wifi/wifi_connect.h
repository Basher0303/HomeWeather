#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "led/led.h"
#include <GyverTimer.h>
#include "settings.h"

class WiFiConnect
{
public:
    WiFiConnect(WiFiSettings &wifiSettings, LED &led);
    void begin();
    void stop();
    void checkConnect();

private:
    WiFiSettings &_wifiSettings;
    LED &_led;
    GTimer *_timerConnected;
};
