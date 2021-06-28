#include <M5Core2.h>

void setup()
{
  M5.begin();
  M5.Lcd.setCursor(20, 20);
  M5.Lcd.setTextSize(2);
  M5.Lcd.print("2021-0");
  M5.Lcd.setTextSize(3);
  M5.Lcd.print("6-28");
  M5.Lcd.setCursor(20, 60);
  M5.Lcd.setTextSize(4);
  M5.Lcd.print("12:34");
  M5.Lcd.setTextSize(2);
  M5.Lcd.print(":56'78");
  M5.Lcd.println(M5.RTC.Second);
}

void loop()
{
}
