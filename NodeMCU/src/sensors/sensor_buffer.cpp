#include "sensor_buffer.h"
#include <Arduino.h>

SensorBuffer::SensorBuffer(int size)
{
    _size = size;
    _buffer = new float[size];
}

void SensorBuffer::push(float value)
{
    if (!isFilled())
    {
        _buffer[_currentIndex] = value;
        _currentIndex += 1;
    }
}

void SensorBuffer::pushLastValue()
{
    if (!isFilled())
    {
        if (_currentIndex == 0)
        {
            _buffer[_currentIndex] = 0;
        }
        else
        {
            _buffer[_currentIndex] = _buffer[_currentIndex - 1];
        }
        _currentIndex += 1;
    }
}

float SensorBuffer::getAverage()
{
    if (_size == 0)
    {
        return 0;
    }

    float sum = 0;
    for (int i = 0; i < _size; i++)
    {
        sum += _buffer[i];
    }
    return static_cast<float>(sum) / _size;
}

bool SensorBuffer::isFilled()
{
    return _currentIndex == _size;
}

void SensorBuffer::clear()
{
    for (int i = 0; i < _size; i++)
    {
        _buffer[i] = 0;
    }
    _currentIndex = 0;
}
