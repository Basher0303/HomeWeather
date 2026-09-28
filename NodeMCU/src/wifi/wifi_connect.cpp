#include "wifi_connect.h"
#include "utils.h"
#include <ESP8266WiFi.h>

WiFiConnect::WiFiConnect(WiFiSettings &wifiSettings, LED &led) : _wifiSettings(wifiSettings), _led(led)
{
    _timerConnected = new GTimer(MS);
}

void WiFiConnect::begin()
{
    WiFi.begin(_wifiSettings.ssid, _wifiSettings.password);
    _timerConnected->setInterval(500);
}

void WiFiConnect::stop()
{
    _timerConnected->stop();
    _led.stopBlink();
}

void WiFiConnect::checkConnect()
{
    if (_timerConnected->isReady())
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            _led.stopBlink();
        }
        else
        {
            _led.startBlink(500);

            consoleLogLn("WiFi not connected :(");
            consoleLog("SSID = ");
            consoleLogLn(_wifiSettings.ssid);
            consoleLog("Password = ");
            consoleLogLn(_wifiSettings.password);
        }
    }
}
