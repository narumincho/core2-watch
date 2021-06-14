#include <M5Core2.h>

void setup()
{
    M5.begin();
}

void loop()
{
    M5.Lcd.drawLine(0, 0, random(0, 320), random(0, 240), random(0, 65535));
    delay(26);
}
