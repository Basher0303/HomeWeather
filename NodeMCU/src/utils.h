#pragma once
#include <Arduino.h>

extern bool debugMode;

template <typename T>
void consoleLogLn(T text)
{
    if (debugMode)
    {
        Serial.println(text);
    }
}

template <typename T>
void consoleLog(T text)
{
    if (debugMode)
    {
        Serial.print(text);
    }
}
