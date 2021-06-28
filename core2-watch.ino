#include <M5Core2.h>

// 今の時刻を取得した後に保存しておく一時変数
RTC_TimeTypeDef nowTime;
// 今の日付を取得した後に保存しておく一時変数
RTC_DateTypeDef nowDate;

// 前の秒. 秒がいつ変化するか調べる必要があるため
uint8_t beforeSeconds;
unsigned long secondsStartMillis;
char sprintfBuf[64];
const uint8_t Mode_WiFi = 0;
const uint8_t Mode_Time = 1;

uint8_t nowMode = Mode_Time;
uint8_t beforeMode = Mode_Time;


void setup()
{
  M5.begin();
  drawMenu();
}

void loop()
{
  if (beforeMode != nowMode)
  {
    M5.Lcd.fillScreen(BLACK);
    beforeMode = nowMode;
    drawMenu();
  }
  if (nowMode == Mode_WiFi)
  {
    M5.Lcd.setCursor(0, 116);
    M5.Lcd.setTextSize(2);
    M5.Lcd.print("Now WiFi Mode.");
  }
  if (nowMode == Mode_Time)
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
  TouchPoint_t pos = M5.Touch.getPressPoint();
  if (220 < pos.y)
  {
    if (0 < pos.x && pos.x < 106)
    {
      nowMode = Mode_WiFi;
    }
    else if (pos.x < 214)
    {
      nowMode = Mode_Time;
    }
  }
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

// 下のモード切り替えボタンを表示する
void drawMenu()
{
  M5.Lcd.fillRect(0, 320, 221, 19, BLACK);
  M5.Lcd.drawFastHLine(0, 220, 319, WHITE);
  M5.Lcd.drawFastVLine(106, 220, 18, WHITE);
  M5.Lcd.drawFastVLine(214, 220, 18, WHITE);

  // WiFi
  M5.Lcd.setCursor(28, 222);
  M5.Lcd.setTextSize(2);
  if(nowMode == Mode_WiFi) {
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.fillRect(0, 221, 105, 20, ORANGE);
  } else {
    M5.Lcd.setTextColor(WHITE);
  }
  M5.Lcd.print("WiFi");

  // Time
  M5.Lcd.setCursor(134, 222);
  M5.Lcd.setTextSize(2);
  if(nowMode == Mode_Time) {
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.fillRect(107, 221, 104, 20, ORANGE);
  } else {
    M5.Lcd.setTextColor(WHITE);
  }
  M5.Lcd.print("Time");
  M5.Lcd.setTextColor(WHITE, BLACK);
}
