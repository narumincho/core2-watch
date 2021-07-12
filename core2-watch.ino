#include <M5Core2.h>
#include <WiFi.h>
#include <HTTPClient.h>

namespace core2watch
{

  // 前の秒. 秒がいつ変化するか調べる必要があるため
  uint8_t beforeSeconds;
  // 今回の秒が始まったミリ秒
  unsigned long secondsStartMillis;
  // 表示用のバッファ
  char sprintfBuf[64];
  // WiFi の SSID
  const String ssid = "n";
  // WiFi の パスワード
  const String wifiPassword = "testpass";

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

  // 今のモード
  Mode nowMode = Mode::Time;

  // 前のモード
  Mode beforeMode = Mode::WiFi;

  // WiFi モード内の状態
  WiFiState wiFiState = WiFiState::Init;

  // Data モード 内での状態
  DataState dataState = DataState::None;

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
  void setRtcDateTime(uint16_t year,
                      uint8_t month,
                      uint8_t day,
                      uint8_t hours,
                      uint8_t minutes,
                      uint8_t seconds)
  {
    RTC_DateTypeDef ntpDate;
    ntpDate.Year = year;
    ntpDate.Month = month;
    ntpDate.Date = day;
    M5.Rtc.SetDate(&ntpDate);

    RTC_TimeTypeDef ntpTime;
    ntpTime.Hours = hours;
    ntpTime.Minutes = minutes;
    ntpTime.Seconds = seconds;
    M5.Rtc.SetTime(&ntpTime);
  }

  /* ---------------------------------
            時刻を表示する
--------------------------------- */
  void drawDateTime(uint16_t year,
                    uint8_t month,
                    uint8_t day,
                    uint8_t hours,
                    uint8_t minutes,
                    uint8_t seconds,
                    unsigned long milliSeconds)
  {
    // year
    M5.Lcd.setCursor(20, 36);
    M5.Lcd.setTextSize(2);
    sprintf(sprintfBuf, "%04d-", year);
    M5.Lcd.print(sprintfBuf);

    // month, day
    M5.Lcd.setCursor(108, 30);
    M5.Lcd.setTextSize(3);
    sprintf(sprintfBuf, "%02d-%02d", month, day);
    M5.Lcd.print(sprintfBuf);

    // hours, minutes
    M5.Lcd.setCursor(20, 100);
    M5.Lcd.setTextSize(5);
    sprintf(sprintfBuf, "%02d:%02d", hours, minutes);
    M5.Lcd.print(sprintfBuf);

    // seconds, milliSeconds
    M5.Lcd.setCursor(190, 116);
    M5.Lcd.setTextSize(2);
    sprintf(sprintfBuf, ":%02d'%03ld", seconds, milliSeconds);
    M5.Lcd.print(sprintfBuf);
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
      M5.Lcd.println("loading");
      M5.Lcd.println(ssid.c_str());
      M5.Lcd.println(wifiPassword.c_str());

      WiFi.begin(ssid.c_str(), wifiPassword.c_str());
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
          sprintf(sprintfBuf, "raw: %04d-%02d-%02d %02d:%02d:%02d", ntpDateTime.tm_year, ntpDateTime.tm_mon, ntpDateTime.tm_mday, ntpDateTime.tm_hour, ntpDateTime.tm_min, ntpDateTime.tm_sec);
          M5.Lcd.println(sprintfBuf);

          setRtcDateTime(
              ntpDateTime.tm_year + 1900,
              ntpDateTime.tm_mon + 1,
              ntpDateTime.tm_mday,
              ntpDateTime.tm_hour,
              ntpDateTime.tm_min,
              ntpDateTime.tm_sec);
        }
        else
        {
          M5.Lcd.println("time error");
        }
        wiFiState = WiFiState::Connected;
        return;
      }
      M5.Lcd.setCursor(0, 16 * 4);
      sprintf(sprintfBuf, "status: %02d", status);
      M5.Lcd.print(sprintfBuf);
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
  void updateInTimeMode()
  {
    RTC_DateTypeDef nowDate;
    RTC_TimeTypeDef nowTime;

    M5.Rtc.GetDate(&nowDate);
    M5.Rtc.GetTime(&nowTime);

    if (beforeSeconds != nowTime.Seconds)
    {
      secondsStartMillis = millis();
      beforeSeconds = nowTime.Seconds;
    }

    drawDateTime(nowDate.Year,
                 nowDate.Month,
                 nowDate.Date,
                 nowTime.Hours,
                 nowTime.Minutes,
                 nowTime.Seconds,
                 millis() - secondsStartMillis);
  }

  /* ================================
              Data Mode
================================= */
  void updateInDataMode(TouchPoint_t touchPosition)
  {
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(150, 16);
    M5.Lcd.println("send to Notion");
    M5.Lcd.drawRect(150, 12, 170, 20, GREEN);

    if (
        150 < touchPosition.x && touchPosition.y < 20)
    {
      dataState = DataState::Sending;
      resetModeArea();
      HTTPClient http;
      http.begin("https://narumincho.com/");
      http.addHeader("accept", "text/html");
      http.addHeader("user-agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");

      int httpCode = http.GET();
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
      float temperature = 0.0f;
      M5.IMU.getTempData(&temperature);
      sprintf(sprintfBuf, "IMU temperature: %03.3f degree Celsius", temperature);
      M5.Lcd.println(sprintfBuf);

      float pitch = 0.0f;
      float roll = 0.0f;
      float yaw = 0.0f;
      M5.IMU.getAhrsData(&pitch, &roll, &yaw);
      sprintf(sprintfBuf, "pitch: %03.3f, roll: %03.3f, yaw: %03.3f", pitch, roll, yaw);
      M5.Lcd.println(sprintfBuf);

      sprintf(sprintfBuf, "voltabe: %03.3f", M5.Axp.GetBatVoltage());
      M5.Lcd.println(sprintfBuf);

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
  }

  /* ================================
               loop
================================= */
  void loop()
  {
    // モード切り替え
    TouchPoint_t touchPosition = M5.Touch.getPressPoint();

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
      ::core2watch::updateInTimeMode();
    }
    if (nowMode == Mode::Data)
    {
      ::core2watch::updateInDataMode(touchPosition);
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