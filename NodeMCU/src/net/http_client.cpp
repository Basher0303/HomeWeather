#include "http_client.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>

void sendHttpRequest(const String &url, float co2, float temperature, float humidity, float pressure)
{
    String jsonData;
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["co2"] = co2;
    jsonDoc["temperature"] = temperature;
    jsonDoc["humidity"] = humidity;
    jsonDoc["pressure"] = pressure;
    serializeJson(jsonDoc, jsonData);

    WiFiClient client;
    HTTPClient https;

    https.begin(client, url + "/set");
    https.addHeader("Content-Type", "application/json");

    int httpCode = https.POST(jsonData);
    String payload = https.getString();

    Serial.print("Статус ответа: ");
    Serial.println(httpCode);
    // Serial.print("Тело ответа: ");
    // Serial.println(payload);

    https.end();
}
