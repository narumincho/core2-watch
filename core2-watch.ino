#include <M5Core2.h>

RTC_TimeTypeDef nowTime;
RTC_DateTypeDef nowDate;
uint8_t beforeSeconds;
unsigned long secondsStartMillis;
char sprintfBuf[64];

void setup()
{
  M5.begin();
}

void loop()
{
  M5.Rtc.GetDate(&nowDate);
  M5.Rtc.GetTime(&nowTime);

  if (beforeSeconds != nowTime.Seconds)
  {
    secondsStartMillis = millis();
    beforeSeconds = nowTime.Seconds;
  }

  DisplayDateTime(nowDate.Year,
                  nowDate.Month,
                  nowDate.Date,
                  nowTime.Hours,
                  nowTime.Minutes,
                  nowTime.Seconds,
                  millis() - secondsStartMillis);
}

void DisplayDateTime(uint16_t year,
                     uint8_t month,
                     uint8_t day,
                     uint8_t hours,
                     uint8_t minutes,
                     uint8_t seconds,
                     unsigned long milliSeconds)
{
  M5.Lcd.setCursor(20, 20);
  M5.Lcd.setTextSize(2);
  sprintf(sprintfBuf, "%04d", year);
  M5.Lcd.print(sprintfBuf);
  M5.Lcd.setTextSize(3);
  sprintf(sprintfBuf, "-%02d-%02d", month, day);
  M5.Lcd.print(sprintfBuf);

  M5.Lcd.setCursor(20, 60);
  M5.Lcd.setTextSize(4);
  sprintf(sprintfBuf, "%02d:%02d", hours, minutes);
  M5.Lcd.print(sprintfBuf);
  M5.Lcd.setTextSize(2);

  sprintf(sprintfBuf, ":%02d'%03d", seconds, milliSeconds);
  M5.Lcd.print(sprintfBuf);
}
