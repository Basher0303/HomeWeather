#include <ESP8266WiFi.h>
#include <GyverBME280.h>
#include "GyverButton.h"
#include "GyverTimer.h"
#include <EEPROM.h>

#include "settings.h"
#include "led/led.h"
#include "wifi/wifi_connect.h"
#include "web/web_server.h"
#include "sensors/sensor_buffer.h"
#include "sensors/co2.h"
#include "net/http_client.h"
#include "utils.h"

#define BTN_PIN D7 // Пин, к которому подключена кнопка
#define LED_PIN D4 // Встроенный светодиод

#define READ_SENSORS_INTERVAL 2000
#define SEND_REQUEST_INTERVAL 10000

SensorBuffer co2Buffer(SEND_REQUEST_INTERVAL / READ_SENSORS_INTERVAL);
SensorBuffer temperatureBuffer(SEND_REQUEST_INTERVAL / READ_SENSORS_INTERVAL);
SensorBuffer humidityBuffer(SEND_REQUEST_INTERVAL / READ_SENSORS_INTERVAL);
SensorBuffer pressureBuffer(SEND_REQUEST_INTERVAL / READ_SENSORS_INTERVAL);

GTimer timerReadSensors(MS, READ_SENSORS_INTERVAL);
GTimer timeoutSensorsReady(MS);

GButton button(BTN_PIN);
LED led(LED_PIN, LOW);
String urlRequest = "http://api.alex-basher.ru";
GyverBME280 bme;
EEPROMSettings settings;
WiFiSettings &wifiSettings = settings.wifiSettings;
WiFiConnect wifiConnect(wifiSettings, led);
WebServer webServer((char *)"HomeWeather", (char *)"1234567890", 80, settings);
bool isSensorsReady = false;
bool settingsMode = false;
bool debugMode = true;

void buttonControll()
{
	if (button.isHolded())
	{
		settingsMode = !settingsMode;

		if (settingsMode)
		{
			wifiConnect.stop();
			webServer.begin();
			led.enable();
		}
		else
		{
			webServer.stop();
			wifiConnect.begin();
			led.disable();
		}
	}
	if (button.isTriple())
	{
		debugMode = !debugMode;
		consoleLogLn("Button triple");
	}
}

void setup()
{
	Serial.begin(115200);
	co2Begin();

	EEPROM.begin(sizeof(settings));
	EEPROM.get(0, settings);

	if (settings.magic != EEPROMSettings::MAGIC)
	{
		initDefaultSettings(settings);
		EEPROM.put(0, settings);
		EEPROM.commit();
	}

	wifiSettings = settings.wifiSettings;
	urlRequest = String(settings.webSettings.apiUrl);

	pinMode(BTN_PIN, INPUT_PULLUP); // Включаем встроенный pull-up
	button.setTickMode(AUTO);

	wifiConnect.begin();
	timeoutSensorsReady.setTimeout(10 * 1000);
	if (!bme.begin(0x76))
		consoleLogLn("Bme error!");
}

void loop()
{
	buttonControll();
	webServer.handleClient();
	led.blink();
	wifiConnect.checkConnect();

	if (timeoutSensorsReady.isReady())
	{
		isSensorsReady = true;
	}
	if (isSensorsReady && timerReadSensors.isReady())
	{
		int co2 = readCO2();
		if (co2 != -1)
		{
			co2Buffer.push(co2);
		}
		else
		{
			co2Buffer.pushLastValue();
			Serial.println("Ошибка чтения CO2!");
		}

		temperatureBuffer.push(bme.readTemperature());
		humidityBuffer.push(bme.readHumidity());
		pressureBuffer.push(bme.readPressure());

		if (co2Buffer.isFilled())
		{
			Serial.print("CO2: ");
			Serial.println(co2Buffer.getAverage());

			Serial.print("Temperature: ");
			Serial.println(temperatureBuffer.getAverage());

			Serial.print("Humidity: ");
			Serial.println(humidityBuffer.getAverage());

			Serial.print("Pressure: ");
			Serial.println(pressureBuffer.getAverage());

			if (WiFi.status() == WL_CONNECTED)
			{
				Serial.print("Отправка данных: ");
				Serial.println(urlRequest);
				sendHttpRequest(urlRequest, co2Buffer.getAverage(), temperatureBuffer.getAverage(), humidityBuffer.getAverage(), pressureBuffer.getAverage());
			}
			else
			{
				Serial.println("WiFi не подключен, данные не отправлены");
			}

			co2Buffer.clear();
			temperatureBuffer.clear();
			humidityBuffer.clear();
			pressureBuffer.clear();
		}
	}
}
