#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <GyverBME280.h>
#include <ArduinoJson.h>
#include "GyverButton.h"
#include "GyverTimer.h"
#include <EEPROM.h>
#include <SoftwareSerial.h>


template < typename T > void consoleLogLn(T text);
template < typename T > void consoleLog(T text);


struct WiFiSettings {
  char ssid[32];
  char password[32];
};

class LED {
  public:
    LED(byte pin, byte enabledLevel) {
      _pin = pin;
      _enabledLevel = enabledLevel;
      _disabledLevel = _enabledLevel == HIGH ? LOW : HIGH;
      _timer = new GTimer(MS);
      pinMode(pin, OUTPUT);
      disable();
    }
    void enable() {
      _state = true;
      if(!_timer->isEnabled()) {
        digitalWrite(_pin, _enabledLevel);
      }
    }
    void disable() {
      _state = false;
      if(!_timer->isEnabled()) {
        digitalWrite(_pin, _disabledLevel);
      }
    }
    void startBlink(uint16_t prd) {
      if(!_timer->isEnabled()) {
        _blinkState = true;
        _timer->setInterval(prd);
      }
    }
    void stopBlink() {
      _timer->stop();
      digitalWrite(_pin, _state ? _enabledLevel : _disabledLevel);
    }
    bool blink() {
      if(_timer->isReady()) {
        _blinkState = !_blinkState;
        digitalWrite(_pin, _blinkState ? _enabledLevel : _disabledLevel);
      }
    }
  private:
    byte _pin;
    byte _enabledLevel;
    byte _disabledLevel;
    bool _enabled;
    bool _state;
    bool _blinkState;
    GTimer* _timer;
};


class WiFiConnect {
public:
  WiFiConnect(WiFiSettings &wifiSettings, LED &led) : _wifiSettings(wifiSettings), _led(led) {
    _timerConnected = new GTimer(MS);
  }

  void begin() {
    WiFi.begin(_wifiSettings.ssid, _wifiSettings.password);
    _timerConnected->setInterval(500);
  }

  void stop() {
    _timerConnected->stop();
    _led.stopBlink();
  }

  void checkConnect() {
    if(_timerConnected->isReady()) {
      if(WiFi.status() == WL_CONNECTED) {
        _led.stopBlink();
        
        // consoleLogLn("WiFi connected");
        // consoleLogLn("IP address: ");
        // consoleLogLn(WiFi.localIP());
      } else {
        _led.startBlink(500);

        consoleLogLn("WiFi not connected :(");
        consoleLog("SSID = ");
        consoleLogLn(_wifiSettings.ssid);
        consoleLog("Password = ");
        consoleLogLn(_wifiSettings.password);
      }
    }
  }
private:
  WiFiSettings &_wifiSettings;
  LED &_led;
  GTimer* _timerConnected;
};

class WebServer {
public: 
  WebServer(char* ssid, char* password, int port, WiFiSettings &wifiSettings)
    : _ssid(ssid), _password(password), _port(port), _wifiSettings(wifiSettings) {
    _server = new ESP8266WebServer(port);
    _localIp = IPAddress(192,168,1,1);
    _gateway = IPAddress(192,168,1,1);
    _subnet = IPAddress(255,255,255,0);
  }

  void begin() {
    WiFi.softAP(_ssid, _password);
    consoleLogLn("Server start!");
    WiFi.softAPConfig(_localIp, _gateway, _subnet);
    delay(100);
    _server->on("/", std::bind(&WebServer::_handleGetMainPage, this));
    _server->on("/settings", HTTP_POST, std::bind(&WebServer::_handlePostSettings, this));

   _server->begin();
  }

  void stop() {
    WiFi.softAPdisconnect();
    _server->stop();
  }

   void handleClient() {
    _server->handleClient();
  }
private: 
  char* _ssid;
  char* _password;
  int _port;
  WiFiSettings &_wifiSettings;
  
  ESP8266WebServer* _server;
  IPAddress _localIp;
  IPAddress _gateway;
  IPAddress _subnet;

  void _handleGetMainPage() {
    String html = R"rawliteral(
        <!DOCTYPE html>
        <html lang="ru">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <title>Home Weather</title>
            <style>
                body {
                    display: flex;
                    justify-content: center;
                    align-items: center;
                    height: 100vh;
                    margin: 0;
                    background-color: #f0f0f0;
                    font-family: Arial, sans-serif;
                }
                .form-container {
                    background-color: #ffffff;
                    padding: 20px;
                    border-radius: 8px;
                    box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
                    width: 300px;
                }
                h2 {
                    text-align: center;
                    margin-bottom: 20px;
                    color: #333;
                }
                .form-group {
                    margin-bottom: 15px;
                }
                label {
                    display: block;
                    margin-bottom: 5px;
                    color: #555;
                }
                input[type="text"],
                input[type="password"] {
                    width: calc(100% - 20px);
                    padding: 10px;
                    border: 1px solid #ccc;
                    border-radius: 4px;
                    font-size: 14px;
                }
                button {
                    width: 100%;
                    padding: 10px;
                    background-color: #28a745;
                    border: none;
                    border-radius: 4px;
                    color: white;
                    font-size: 16px;
                    cursor: pointer;
                }
                button:hover {
                    background-color: #218838;
                }
            </style>
        </head>
        <body>
            <div class="form-container">
                <h2>Настройки сети</h2>
                <form id="settings-form">
                    <div class="form-group">
                        <label for="ssid">Название</label>
                        <input type="text" id="ssid" name="ssid" value="%SSID%" required>
                    </div>
                    <div class="form-group">
                        <label for="password">Пароль</label>
                        <input type="text" id="password" name="password" value="%PASSWORD%" required>
                    </div>
                    <button type="submit">Сохранить</button>
                </form>
            </div>
            <script>
                document.getElementById('settings-form').addEventListener('submit', function(event) {
                    event.preventDefault();
                    const formData = new FormData(this);
                    fetch('/settings', {
                        method: 'POST',
                        body: formData,
                    })
                    .then(response => {
                        if (response.ok) {
                            alert('Настройки успешно применены');
                        } else {
                            alert('Ошибка при применении настроек(серверная)');
                        }
                    })
                    .catch(error => {
                        console.error('Ошибка:', error);
                        alert('Ошибка при применении настроек(клиентская)');
                    });
                });
            </script>
        </body>
        </html>
    )rawliteral";

    html.replace("%SSID%", _wifiSettings.ssid);
    html.replace("%PASSWORD%", _wifiSettings.password);
    _server->send(200, "text/html", html); // Отправляем HTML-форму
  }

  void _handlePostSettings() {
    if (_server->hasArg("ssid")) { 
      String ssid = _server->arg("ssid");
      String password = _server->arg("password");
      ssid.toCharArray(_wifiSettings.ssid, sizeof(_wifiSettings.ssid));
      password.toCharArray(_wifiSettings.password, sizeof(_wifiSettings.password));
      EEPROM.put(0, _wifiSettings); 
      EEPROM.commit();

      _server->send(200, "text/plain", "OK"); // Отправляем ответ
    } else {
      _server->send(400, "text/plain", "Error");
    }
  }
};

class SensorBuffer {
public: 
  SensorBuffer(int size) {
    _size = size;
    _buffer = new float[size];
  }

  void push(float value) {
    if(!isFilled()) {
      _buffer[_currentIndex] = value;
      _currentIndex += 1;
    }
  }

  void pushLastValue() {
    if(!isFilled()) {
      if(_currentIndex == 0) {
        _buffer[_currentIndex] = 0;
      } else {
        _buffer[_currentIndex] = _buffer[_currentIndex - 1];
      }
      _currentIndex += 1;
    }
  }

  float getAverage() {
    if (_size == 0) {
        return 0;
    }

    float sum = 0;
    for (int i = 0; i < _size; i++) {
        sum += _buffer[i];
    }
    return static_cast<float>(sum) / _size;
  }

  bool isFilled() {
    return _currentIndex == _size;
  }

  void clear() {
    for (int i = 0; i < _size; i++) {
      _buffer[i] = 0;
    }
    _currentIndex = 0;
  }

private:
  int _size;
  int _currentIndex = 0;
  float* _buffer;
};


#define BTN_PIN D7  // Пин, к которому подключена кнопка
#define LED_PIN D4  // Встроенный светодиод

#define READ_SENSORS_INTERVAL  2000
#define SEND_REQUEST_INTERVAL  10000

SensorBuffer co2Buffer(SEND_REQUEST_INTERVAL / READ_SENSORS_INTERVAL);
SensorBuffer temperatureBuffer(SEND_REQUEST_INTERVAL / READ_SENSORS_INTERVAL);
SensorBuffer humidityBuffer(SEND_REQUEST_INTERVAL / READ_SENSORS_INTERVAL);
SensorBuffer pressureBuffer(SEND_REQUEST_INTERVAL / READ_SENSORS_INTERVAL);

SoftwareSerial co2Serial(D5, D6);
GTimer timerReadSensors(MS, READ_SENSORS_INTERVAL);
GTimer timeoutSensorsReady(MS);

GButton button(BTN_PIN);
LED led(LED_PIN, LOW);
String urlRequest =  "http://api.alex-basher.ru";
GyverBME280 bme;
WiFiSettings wifiSettings;
WiFiConnect wifiConnect(wifiSettings, led);
WebServer webServer("HomeWeather", "1234567890", 80, wifiSettings);
bool isSensorsReady = false;
bool settingsMode = false;
bool debugMode = true;


void setup() {
  Serial.begin(115200);
  co2Serial.begin(9600);

  EEPROM.begin(sizeof(wifiSettings.ssid) + sizeof(wifiSettings.password));
  EEPROM.get(0, wifiSettings);

  pinMode(BTN_PIN, INPUT_PULLUP);  // Включаем встроенный pull-up
  button.setTickMode(AUTO);

  wifiConnect.begin();
  timeoutSensorsReady.setTimeout(10 * 1000);
  if (!bme.begin(0x76)) consoleLogLn("Bme error!");
}

void loop() {
  buttonControll();
  webServer.handleClient();
  led.blink();
  wifiConnect.checkConnect();

  if(timeoutSensorsReady.isReady()) {
    isSensorsReady = true;
  } 
  if(isSensorsReady && timerReadSensors.isReady()) {
    int co2 = readCO2();
    if (co2 != -1) {
      co2Buffer.push(co2);
    } else {
      co2Buffer.pushLastValue();
      Serial.println("Ошибка чтения CO2!");
    }

    temperatureBuffer.push(bme.readTemperature());
    humidityBuffer.push(bme.readHumidity());
    pressureBuffer.push(bme.readPressure());

    if(co2Buffer.isFilled()) {
      Serial.print("CO2: ");
      Serial.println(co2Buffer.getAverage());

      Serial.print("Temperature: ");
      Serial.println(temperatureBuffer.getAverage());

      Serial.print("Humidity: ");
      Serial.println(humidityBuffer.getAverage());

      Serial.print("Pressure: ");
      Serial.println(pressureBuffer.getAverage());

      sendHttpRequest();

      co2Buffer.clear();
      temperatureBuffer.clear();
      humidityBuffer.clear();
      pressureBuffer.clear();
    }
  }
}

int readCO2() {
  byte requestData[9] = { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 }; // Команда для чтения CO2
  unsigned char responseData[9]; // Буфер для получения данных

  // Отправляем команду датчику
  co2Serial.write(requestData, 9);
  memset(responseData, 0, 9);
  delay(50);

  co2Serial.readBytes(responseData, 9);
  // Проверка контрольной суммы
  byte checksum = 0;
  for (int i = 1; i < 8; i++) {
    checksum += responseData[i];
  }
  checksum = 255 - checksum + 1;

  if (responseData[8] == checksum) {
    unsigned int HLconcentration = (unsigned int) responseData[2]; 
    unsigned int LLconcentration = (unsigned int) responseData[3]; 
    unsigned int co2 = (256 * HLconcentration) + LLconcentration;
    return co2;
  } else {
    Serial.println("Контрольная сумма не совпала!");
    return -1; // Ошибка контрольной суммы
  }
}

void buttonControll() {
  if (button.isHolded()) {
    settingsMode = !settingsMode;


    if(settingsMode) {
      wifiConnect.stop();
      webServer.begin();
      led.enable();
    } else {
      webServer.stop();
      wifiConnect.begin();
      led.disable();
    }
  }
  if (button.isTriple()) {
    debugMode = !debugMode;
    consoleLogLn("Button triple");
  }
}

void sendHttpRequest() {
  String jsonData;
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["co2"] = co2Buffer.getAverage();
  jsonDoc["temperature"] = temperatureBuffer.getAverage();
  jsonDoc["humidity"] = humidityBuffer.getAverage();
  jsonDoc["pressure"] = pressureBuffer.getAverage();
  serializeJson(jsonDoc, jsonData);
  
  // WiFiClientSecure client;
  // Ignore SSL certificate validation
  // client.setInsecure();

  WiFiClient client;
  
  //create an HTTPClient instance
  HTTPClient https;

  https.begin(client, urlRequest + "/set");      
  https.addHeader("Content-Type", "application/json"); 

  int httpCode = https.POST(jsonData);
  String payload = https.getString();

  Serial.println(httpCode);   //Распечатать код возврата HTTP
  Serial.println(payload);    //Полезная нагрузка для ответа на запрос печати

  https.end();  //Закрыть соединение
}

template < typename T > void consoleLogLn(T text) {
  if(debugMode) {
    Serial.println(text);
  }
}

template < typename T > void consoleLog(T text) {
  if(debugMode) {
    Serial.print(text);
  }
}
