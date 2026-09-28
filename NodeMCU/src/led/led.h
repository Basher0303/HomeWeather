#pragma once
#include <Arduino.h>
#include <GyverTimer.h>

class LED
{
public:
    LED(byte pin, byte enabledLevel);
    void enable();
    void disable();
    void startBlink(uint16_t prd);
    void stopBlink();
    void blink();

private:
    byte _pin;
    byte _enabledLevel;
    byte _disabledLevel;
    bool _enabled;
    bool _state;
    bool _blinkState;
    GTimer *_timer;
};
