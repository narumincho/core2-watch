#include <M5Core2.h>
#include <WiFi.h>

// 今の時刻を取得した後に保存しておく一時変数
RTC_TimeTypeDef nowTime;
// 今の日付を取得した後に保存しておく一時変数
RTC_DateTypeDef nowDate;

// 前の秒. 秒がいつ変化するか調べる必要があるため
uint8_t beforeSeconds;
unsigned long secondsStartMillis;
char sprintfBuf[64];

enum class Mode
{
  WiFi,
  Time,
  Data
};

enum class WiFiState
{
  Init,
  Loading,
  Loaded
};

// 今のモード
Mode nowMode = Mode::Time;

// 前のモード
Mode beforeMode = Mode::WiFi;

// WiFi モード内の状態
WiFiState wiFiState = WiFiState::Init;

void setup()
{
  M5.begin();
  M5.IMU.Init();
  WiFi.mode(WIFI_STA); //STAモード（子機）として使用
  WiFi.disconnect();   //Wi-Fi切断
}

void loop()
{
  if (beforeMode != nowMode)
  {
    M5.Lcd.fillScreen(BLACK);
    beforeMode = nowMode;
    drawMenu();
    if (nowMode == Mode::WiFi)
    {
      wiFiState = WiFiState::Init;
    }
  }
  if (nowMode == Mode::WiFi)
  {
    updateInWifiMode();
  }
  if (nowMode == Mode::Time)
  {
    updateInTimeMode();
  }
  if (nowMode == Mode::Data)
  {
    updateInDataMode();
  }

  // モード切り替え
  TouchPoint_t pos = M5.Touch.getPressPoint();
  if (220 < pos.y)
  {
    if (0 < pos.x && pos.x < 106)
    {
      nowMode = Mode::WiFi;
      return;
    }
    if (pos.x < 214)
    {
      nowMode = Mode::Time;
      return;
    }
    nowMode = Mode::Data;
  }
}

// WiFi モードの処理
void updateInWifiMode()
{
  M5.Lcd.setTextSize(2);
  if (wiFiState == WiFiState::Init)
  {
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.println("laoding");
    WiFi.scanNetworks(true);
    wiFiState = WiFiState::Loading;
    return;
  }
  if (wiFiState == WiFiState::Loading)
  {
    int16_t result = WiFi.scanComplete();
    if (result == -2)
    {
      M5.Lcd.fillRect(0, 0, 320, 200, BLACK);
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.setTextSize(1);
      M5.Lcd.println("faild");
      wiFiState = WiFiState::Loaded;
      return;
    }
    if (result == -1)
    {
      return;
    }
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(1);
    M5.Lcd.fillRect(0, 0, 320, 200, BLACK);
    if (result == 0)
    {
      //ネットワークが見つからないとき
      M5.Lcd.println("no networks found");
      wiFiState = WiFiState::Loaded;
      return;
    }
    wiFiState = WiFiState::Loaded;
    //ネットワークが見つかったとき
    M5.Lcd.print(String(result) + "!");
    for (int i = 0; i < result; i++)
    {
      M5.Lcd.print(i + 1);
      M5.Lcd.print(": ");
      M5.Lcd.print(WiFi.SSID(i)); //SSID(アクセスポイントの識別名)を表示
      M5.Lcd.print(":");
      M5.Lcd.print(WiFi.channel(i)); //チャンネルを表示
      M5.Lcd.print("CH (");
      M5.Lcd.print(WiFi.RSSI(i)); //RSSI(受信信号の強度)を表示
      M5.Lcd.print(")");
      M5.Lcd.print(String(WiFi.encryptionType(i))); //暗号化の種類がOPENか否か
      M5.Lcd.print("  ");

      M5.Lcd.println("");
    }
  }
}

// 時刻モードの処理
void updateInTimeMode()
{
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

// 時刻を表示する
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
  M5.Lcd.setCursor(108, 20);
  M5.Lcd.setTextSize(3);
  sprintf(sprintfBuf, "%02d-%02d", month, day);
  M5.Lcd.print(sprintfBuf);

  // hours, minutes
  M5.Lcd.setCursor(20, 100);
  M5.Lcd.setTextSize(4);
  sprintf(sprintfBuf, "%02d:%02d", hours, minutes);
  M5.Lcd.print(sprintfBuf);

  // seconds, milliSeconds
  M5.Lcd.setCursor(180, 116);
  M5.Lcd.setTextSize(2);
  sprintf(sprintfBuf, ":%02d'%03d", seconds, milliSeconds);
  M5.Lcd.print(sprintfBuf);
}

// データモードでの処理
void updateInDataMode()
{
  M5.Lcd.setCursor(180, 116);
  M5.Lcd.setTextSize(2);
  float temperature = 0.0f;
  M5.IMU.getTempData(&temperature);
  sprintf(sprintfBuf, "%03.3f ^c", temperature);
  M5.Lcd.print(sprintfBuf);
}

// 下のモード切り替えボタンを表示する
void drawMenu()
{
  M5.Lcd.fillRect(0, 320, 221, 19, BLACK);
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
