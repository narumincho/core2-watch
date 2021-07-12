#include <M5Core2.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_SHT31.h>
#include "secret.hpp"

namespace core2watch
{
  // 前の秒. 秒がいつ変化するか調べる必要があるため
  uint8_t beforeSeconds;
  // 今回の秒が始まったミリ秒
  unsigned long secondsStartMillis;

  const int capacity = JSON_OBJECT_SIZE(256);

  // モード
  enum class Mode
  {
    WiFi,
    Time,
    Data
  };

  // WiFi モード 内での状態
  enum class WiFiState
  {
    Init,
    Connecting,
    Connected,
    Fail
  };

  // Data モード 内での状態
  enum class DataState
  {
    None,
    Sending,
    Send
  };

  // 日時
  struct DateTime
  {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
  };

  // 今のモード
  Mode nowMode = Mode::Time;

  // 前のモード
  Mode beforeMode = Mode::WiFi;

  // WiFi モード内の状態
  WiFiState wiFiState = WiFiState::Init;

  // Data モード 内での状態
  DataState dataState = DataState::None;

  Adafruit_SHT31 sht3x;
  Adafruit_BMP280 bme;

  // モードの表示領域を黒で塗りつぶす
  void resetModeArea()
  {
    M5.Lcd.fillRect(0, 320, 221, 19, BLACK);
  }

  // 下のモード切り替えボタンを表示する
  void drawMenu()
  {
    resetModeArea();
    M5.Lcd.drawFastHLine(0, 220, 319, WHITE);
    M5.Lcd.drawFastVLine(106, 220, 19, WHITE);
    M5.Lcd.drawFastVLine(214, 220, 19, WHITE);

    // WiFi
    M5.Lcd.setCursor(28, 222);
    M5.Lcd.setTextSize(2);
    if (nowMode == Mode::WiFi)
    {
      M5.Lcd.setTextColor(BLACK);
      M5.Lcd.fillRect(0, 221, 106, 20, ORANGE);
    }
    else
    {
      M5.Lcd.setTextColor(WHITE);
    }
    M5.Lcd.print("WiFi");

    // Time
    M5.Lcd.setCursor(134, 222);
    M5.Lcd.setTextSize(2);
    if (nowMode == Mode::Time)
    {
      M5.Lcd.setTextColor(BLACK);
      M5.Lcd.fillRect(107, 221, 106, 20, ORANGE);
    }
    else
    {
      M5.Lcd.setTextColor(WHITE);
    }
    M5.Lcd.print("Time");
    M5.Lcd.setTextColor(WHITE, BLACK);

    // Data
    M5.Lcd.setCursor(240, 222);
    M5.Lcd.setTextSize(2);
    if (nowMode == Mode::Data)
    {
      M5.Lcd.setTextColor(BLACK);
      M5.Lcd.fillRect(213, 221, 106, 20, ORANGE);
    }
    else
    {
      M5.Lcd.setTextColor(WHITE);
    }
    M5.Lcd.print("Data");
    M5.Lcd.setTextColor(WHITE, BLACK);
  }

  /* ---------------------------------
  リアルタイムクロック に 時刻 を設定する
--------------------------------- */
  void setRtcDateTime(DateTime dateTime)
  {
    RTC_DateTypeDef ntpDate;
    ntpDate.Year = dateTime.year;
    ntpDate.Month = dateTime.month;
    ntpDate.Date = dateTime.day;
    M5.Rtc.SetDate(&ntpDate);

    RTC_TimeTypeDef ntpTime;
    ntpTime.Hours = dateTime.hours;
    ntpTime.Minutes = dateTime.minutes;
    ntpTime.Seconds = dateTime.seconds;
    M5.Rtc.SetTime(&ntpTime);
  }
  /* ---------------------------------
  リアルタイムクロック から 時刻 を取得する
--------------------------------- */
  DateTime getRtcDateTime()
  {
    RTC_DateTypeDef nowDate;
    RTC_TimeTypeDef nowTime;

    M5.Rtc.GetDate(&nowDate);
    M5.Rtc.GetTime(&nowTime);
    return {
        nowDate.Year,
        nowDate.Month,
        nowDate.Date,
        nowTime.Hours,
        nowTime.Minutes,
        nowTime.Seconds,
    };
  }

  /* ---------------------------------
            時刻を表示する
--------------------------------- */
  void drawDateTime(DateTime dateTime, unsigned long milliSeconds)
  {
    // year
    M5.Lcd.setCursor(20, 36);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("%04d-", dateTime.year);

    // month, day
    M5.Lcd.setCursor(108, 30);
    M5.Lcd.setTextSize(3);
    M5.Lcd.printf("%02d-%02d", dateTime.month, dateTime.day);

    // hours, minutes
    M5.Lcd.setCursor(20, 100);
    M5.Lcd.setTextSize(5);
    M5.Lcd.printf("%02d:%02d", dateTime.hours, dateTime.minutes);

    // seconds, milliSeconds
    M5.Lcd.setCursor(190, 116);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf(":%02d'%03ld", dateTime.seconds, milliSeconds);
  }

  /* ================================
            WiFi Mode
================================= */
  void updateInWifiMode()
  {
    M5.Lcd.setTextSize(2);
    if (wiFiState == WiFiState::Init)
    {
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.printf("conneting %s\n", secret::wifiSsid.c_str());

      WiFi.begin(secret::wifiSsid.c_str(), secret::wifiPassword.c_str());
      wiFiState = WiFiState::Connecting;
      return;
    }
    if (wiFiState == WiFiState::Connecting)
    {
      wl_status_t status = WiFi.status();
      if (status == wl_status_t::WL_CONNECT_FAILED)
      {
        M5.Lcd.fillRect(0, 0, 320, 200, BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextSize(2);
        M5.Lcd.println("faild");
        wiFiState = WiFiState::Fail;
        return;
      }
      if (status == wl_status_t::WL_CONNECTED)
      {
        configTime(60 * 60 * 9, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp");
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextSize(2);
        M5.Lcd.fillRect(0, 0, 320, 200, BLACK);
        M5.Lcd.println("ok");
        // NTP サーバーから取得した時刻
        tm ntpDateTime;

        if (getLocalTime(&ntpDateTime))
        {
          M5.Lcd.println("set time from NTP");
          M5.Lcd.printf("raw: %04d-%02d-%02d %02d:%02d:%02d\n", ntpDateTime.tm_year, ntpDateTime.tm_mon, ntpDateTime.tm_mday, ntpDateTime.tm_hour, ntpDateTime.tm_min, ntpDateTime.tm_sec);

          setRtcDateTime(
              {(uint16_t)(ntpDateTime.tm_year + 1900),
               (uint8_t)(ntpDateTime.tm_mon + 1),
               (uint8_t)ntpDateTime.tm_mday,
               (uint8_t)ntpDateTime.tm_hour,
               (uint8_t)ntpDateTime.tm_min,
               (uint8_t)ntpDateTime.tm_sec});
        }
        else
        {
          M5.Lcd.println("time error");
        }
        wiFiState = WiFiState::Connected;
        return;
      }
      M5.Lcd.setCursor(0, 16 * 4);
      M5.Lcd.printf("status: %02d", status);
    }
  }
  /* ---------------------------------
            モード切り替え
--------------------------------- */
  void checkModeChange(TouchPoint_t touchPosition)
  {
    if (220 < touchPosition.y)
    {
      if (0 < touchPosition.x && touchPosition.x < 106)
      {
        nowMode = Mode::WiFi;
        return;
      }
      if (touchPosition.x < 214)
      {
        nowMode = Mode::Time;
        return;
      }
      nowMode = Mode::Data;
    }
  }

  /* ================================
              Time Mode
================================= */
  void updateInTimeMode(DateTime dateTime)
  {
    if (beforeSeconds != dateTime.seconds)
    {
      secondsStartMillis = millis();
      beforeSeconds = dateTime.seconds;
    }

    drawDateTime(dateTime, millis() - secondsStartMillis);
  }

  /* ================================
              Data Mode
================================= */
  void updateInDataMode(TouchPoint_t touchPosition, DateTime dateTime)
  {
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(150, 16);
    M5.Lcd.println("send to Notion");
    M5.Lcd.drawRect(150, 12, 170, 20, GREEN);

    float imuTemperature = 0.0f;
    M5.IMU.getTempData(&imuTemperature);
    float pitch = 0.0f;
    float roll = 0.0f;
    float yaw = 0.0f;
    M5.IMU.getAhrsData(&pitch, &roll, &yaw);

    float voltage = M5.Axp.GetBatVoltage();

    float env2Temperature = sht3x.readTemperature();
    float humidity = sht3x.readHumidity();
    float pressure = bme.readPressure();

    if (
        (150 < touchPosition.x && touchPosition.y < 20) || (dateTime.seconds == 0))
    {
      dataState = DataState::Sending;
      resetModeArea();
      HTTPClient http;
      http.begin("https://api.notion.com/v1/pages");
      http.addHeader("authorization", "Bearer " + secret::notionIntegrationsSecret);

      http.addHeader("Content-Type", "application/json");
      http.addHeader("Notion-Version", "2021-05-13");

      StaticJsonDocument<512> doc;

      char dateBuffer[16];
      sprintf(dateBuffer, "%04d-%02d-%02d", dateTime.year, dateTime.month, dateTime.day);

      doc["parent"]["database_id"] = secret::notionDatabaseId;
      doc["properties"]["Name"]["title"][0]["text"]["content"] = String(dateBuffer) + " の日記";
      // doc["properties"]["送信日時"]["type"] = "date";

      // doc["properties"]["送信日時"]["start"] = String(dateBuffer);
      doc["properties"]["気温"]["type"] = "number";
      doc["properties"]["気温"]["number"] = env2Temperature;
      doc["properties"]["湿度"]["type"] = "number";
      doc["properties"]["湿度"]["number"] = humidity;
      doc["properties"]["気圧"]["type"] = "number";
      doc["properties"]["気圧"]["number"] = pressure;
      doc["children"][0]["object"] = "block";
      doc["children"][0]["type"] = "heading_2";
      doc["children"][0]["heading_2"]["text"][0]["type"] = "text";
      doc["children"][0]["heading_2"]["text"][0]["text"]["content"] = 30 < env2Temperature ? "暑いね" : "暑くないね";

      String output;
      serializeJson(doc, output);

      int httpCode = http.POST(output);
      String payload = http.getString();

      // httpCode will be negative on error
      if (httpCode > 0)
      {
        // HTTP header has been send and Server response header has been handled
        M5.Lcd.printf("[HTTP] GET... code: %d\n", httpCode);
        M5.Lcd.println(payload);
      }
      else
      {
        M5.Lcd.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        M5.Lcd.println(payload);
      }
      return;
    }

    if (dataState == DataState::None)
    {
      M5.Lcd.setCursor(0, 116);
      M5.Lcd.setTextSize(1);

      M5.Lcd.printf("IMU temperature: %03.3f degree Celsius\n", imuTemperature);

      M5.Lcd.printf("pitch: %03.3f, roll: %03.3f, yaw: %03.3f\n", pitch, roll, yaw);

      M5.Lcd.printf("voltabe: %03.3f\n", voltage);

      M5.Lcd.printf("env2: %03.3f, %03.3f, %03.3f\n", env2Temperature, humidity, pressure);

      M5.Axp.SetLed(0 < pitch);
    }
  }

  /* ================================
               setup
================================= */
  void setup()
  {
    M5.begin();
    M5.IMU.Init();
    WiFi.mode(WIFI_STA); //STAモード（子機）として使用

    // センサーの初期化
    while (!bme.begin(0x76))
    {
      Serial.println("BMP280 init fail");
    }
    while (!sht3x.begin(0x44))
    {
      Serial.println("SHT3x init fail");
    }
  }

  /* ================================
               loop
================================= */
  void loop()
  {
    // モード切り替え
    TouchPoint_t touchPosition = M5.Touch.getPressPoint();
    DateTime dateTime = getRtcDateTime();

    if (beforeMode != nowMode)
    {
      M5.Lcd.fillScreen(BLACK);
      beforeMode = nowMode;
      ::core2watch::drawMenu();
      if (nowMode == Mode::WiFi)
      {
        wiFiState = WiFiState::Init;
      }
      if (nowMode == Mode::Data)
      {
        dataState = DataState::None;
      }
    }
    if (nowMode == Mode::WiFi)
    {
      ::core2watch::updateInWifiMode();
    }
    if (nowMode == Mode::Time)
    {
      ::core2watch::updateInTimeMode(dateTime);
    }
    if (nowMode == Mode::Data)
    {
      ::core2watch::updateInDataMode(touchPosition, dateTime);
    }
    checkModeChange(touchPosition);
  }
}

/* ================================
               setup
================================= */
void setup()
{
  core2watch::setup();
}

/* ================================
               loop
================================= */
void loop()
{
  core2watch::loop();
}