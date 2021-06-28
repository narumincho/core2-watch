#include <M5Core2.h>

RTC_TimeTypeDef nowTime;
RTC_DateTypeDef nowDate;

char timeStrbuff[64];

void setup()
{
  M5.begin();
}

void loop()
{
  M5.Rtc.GetDate(&nowDate);
  M5.Rtc.GetTime(&nowTime);

  DisplayDateTime(nowDate.Year, nowDate.Month, nowDate.Date, nowTime.Hours, nowTime.Minutes, nowTime.Seconds);
  delay(16);
}

void DisplayDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hours, uint8_t minutes, uint8_t seconds)
{
  M5.Lcd.setCursor(20, 20);
  M5.Lcd.setTextSize(2);
  M5.Lcd.print(String(year));
  M5.Lcd.setTextSize(3);
  M5.Lcd.print("-" + String(month) + "-" + String(day));
  M5.Lcd.setCursor(20, 60);
  M5.Lcd.setTextSize(4);
  M5.Lcd.print(String(hours) + ":" + String(minutes));
  M5.Lcd.setTextSize(2);
  M5.Lcd.print(":" + String(seconds));
}
