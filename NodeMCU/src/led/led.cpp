#include "led.h"

LED::LED(byte pin, byte enabledLevel)
{
    _pin = pin;
    _enabledLevel = enabledLevel;
    _disabledLevel = _enabledLevel == HIGH ? LOW : HIGH;
    _timer = new GTimer(MS);
    pinMode(pin, OUTPUT);
    disable();
}

void LED::enable()
{
    _state = true;
    if (!_timer->isEnabled())
    {
        digitalWrite(_pin, _enabledLevel);
    }
}

void LED::disable()
{
    _state = false;
    if (!_timer->isEnabled())
    {
        digitalWrite(_pin, _disabledLevel);
    }
}

void LED::startBlink(uint16_t prd)
{
    if (!_timer->isEnabled())
    {
        _blinkState = true;
        _timer->setInterval(prd);
    }
}

void LED::stopBlink()
{
    _timer->stop();
    digitalWrite(_pin, _state ? _enabledLevel : _disabledLevel);
}

void LED::blink()
{
    if (_timer->isReady())
    {
        _blinkState = !_blinkState;
        digitalWrite(_pin, _blinkState ? _enabledLevel : _disabledLevel);
    }
}
