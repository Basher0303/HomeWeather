#pragma once
#include <Arduino.h>

void sendHttpRequest(const String &url, float co2, float temperature, float humidity, float pressure);
