#pragma once

class SensorBuffer
{
public:
    SensorBuffer(int size);
    void push(float value);
    void pushLastValue();
    float getAverage();
    bool isFilled();
    void clear();

private:
    int _size;
    int _currentIndex = 0;
    float *_buffer;
};
