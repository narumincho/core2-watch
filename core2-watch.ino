#include <M5Core2.h>
#include <WiFi.h>

const char *ssid = "ssid";
const char *password = "password";

void setup()
{
  M5.begin();
  M5.Lcd.setTextSize(2);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    M5.Lcd.print(".");
  }
  M5.Lcd.print("\r\nWiFi connected\r\nIP address: ");
  M5.Lcd.println(WiFi.localIP());
}

void loop()
{
}