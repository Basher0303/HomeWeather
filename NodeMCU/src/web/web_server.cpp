#include "web_server.h"
#include <ESP8266WiFi.h>
#include "utils.h"
#include "generated/index_html.h"

WebServer::WebServer(char *ssid, char *password, int port, EEPROMSettings &settings)
    : _ssid(ssid), _password(password), _port(port), _settings(settings), _wifiSettings(_settings.wifiSettings)
{
    _server = new ESP8266WebServer(port);
    _localIp = IPAddress(192, 168, 1, 1);
    _gateway = IPAddress(192, 168, 1, 1);
    _subnet = IPAddress(255, 255, 255, 0);
}

void WebServer::begin()
{
    WiFi.softAP(_ssid, _password);
    consoleLogLn("Server start!");
    WiFi.softAPConfig(_localIp, _gateway, _subnet);
    delay(100);
    _server->on("/", std::bind(&WebServer::_handleGetMainPage, this));
    _server->on("/settings", HTTP_POST, std::bind(&WebServer::_handlePostSettings, this));

    _server->begin();
}

void WebServer::stop()
{
    WiFi.softAPdisconnect();
    _server->stop();
}

void WebServer::handleClient()
{
    _server->handleClient();
}

void WebServer::_handleGetMainPage()
{
    String html = generated::INDEX_HTML;
    html.replace("%SSID%", _settings.wifiSettings.ssid);
    html.replace("%PASSWORD%", _settings.wifiSettings.password);
    html.replace("%API-URL%", _settings.webSettings.apiUrl);
    html.replace("%API-KEY%", _settings.webSettings.apiKey);
    _server->send(200, "text/html", html);
}

void WebServer::_handlePostSettings()
{
    if (_server->hasArg("ssid") && _server->hasArg("password") && _server->hasArg("api-url") && _server->hasArg("api-key"))
    {
        String ssid = _server->arg("ssid");
        String password = _server->arg("password");
        String apiUrl = _server->arg("api-url");
        String apiKey = _server->arg("api-key");

        ssid.toCharArray(_settings.wifiSettings.ssid, sizeof(_settings.wifiSettings.ssid));
        password.toCharArray(_settings.wifiSettings.password, sizeof(_settings.wifiSettings.password));
        apiUrl.toCharArray(_settings.webSettings.apiUrl, sizeof(_settings.webSettings.apiUrl));
        apiKey.toCharArray(_settings.webSettings.apiKey, sizeof(_settings.webSettings.apiKey));

        EEPROM.put(0, _settings);
        EEPROM.commit();

        _server->send(200, "text/plain", "OK");
    }
    else
    {
        _server->send(400, "text/plain", "Error");
    }
}
