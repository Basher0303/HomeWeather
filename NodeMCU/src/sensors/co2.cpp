#include "co2.h"
#include <SoftwareSerial.h>
#include <Arduino.h>

static SoftwareSerial co2Serial(D5, D6);

void co2Begin()
{
    co2Serial.begin(9600);
    // Уменьшаем таймаут чтения и очищаем входной буфер на старте
    co2Serial.setTimeout(200);
    while (co2Serial.available())
        co2Serial.read();
}

int readCO2()
{
    byte requestData[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
    unsigned char responseData[9];

    // Попробуем выполнить несколько попыток чтения, если устройство ещё не готово
    const int maxAttempts = 3;
    for (int attempt = 0; attempt < maxAttempts; attempt++)
    {
        // Очистим входной буфер перед запросом
        while (co2Serial.available())
            co2Serial.read();

        co2Serial.write(requestData, 9);
        memset(responseData, 0, 9);
        delay(100);

        size_t read = co2Serial.readBytes(responseData, 9);
        if (read != 9)
        {
            Serial.println("CO2: incomplete response");
            delay(100);
            continue;
        }

        byte checksum = 0;
        for (int i = 1; i < 8; i++)
        {
            checksum += responseData[i];
        }
        checksum = 255 - checksum + 1;

        if (responseData[8] == checksum)
        {
            unsigned int HLconcentration = (unsigned int)responseData[2];
            unsigned int LLconcentration = (unsigned int)responseData[3];
            unsigned int co2 = (256 * HLconcentration) + LLconcentration;
            return co2;
        }
        else
        {
            Serial.println("Контрольная сумма не совпала!");
            delay(150);
            // повторяем попытку
        }
    }

    // Все попытки неуспешны
    return -1;
}
