#include <M5Core2.h>

void setup()
{
  M5.begin(true, true, true, true);
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setTextColor(BLACK, WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.printf("DISPPLAY Test!");
}

void loop()
{
  static bool touch = false;
  static uint8_t colorOld = -1;
  TouchPoint_t pos = M5.Touch.getPressPoint();
  if (pos.x != -1)
  {
    touch = true;
    M5.Axp.SetLed(0);
  }
  if (pos.y > 240)
  {
    uint8_t color;
    if (pos.x < 109)
    {
      M5.Lcd.setTextColor(RED, WHITE);
      color = 1;
    }
    else if (pos.x > 218)
    {
      M5.Lcd.setTextColor(BLUE, WHITE);
      color = 2;
    }
    else if (pos.x >= 109 && pos.x <= 218)
    {
      M5.Lcd.setTextColor(GREEN, WHITE);
      color = 3;
    }

    if (color != colorOld)
    {
      colorOld = color;
      M5.Axp.SetLDOEnable(3, true);
      delay(100);
      M5.Axp.SetLDOEnable(3, false);
    }

    M5.Lcd.setCursor(0, 26);
    M5.Lcd.printf(" X:%3d\n Y:%3d", pos.x, pos.y);
  }
  else
  {
    touch = false;
    colorOld = -1;
    M5.Axp.SetLed(1);
  }
  delay(10);
}
