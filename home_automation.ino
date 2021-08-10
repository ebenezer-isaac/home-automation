#include "Arduino.h"
#include <Wire.h>
#include "PCF8574.h"
#include <SPI.h>
#include <Adafruit_ILI9341esp.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <XPT2046.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "";
char ssid[] = "";
char pass[] = "";

PCF8574 pcf8574(0x20);
#define TFT_DC 2
#define TFT_CS 15
#define BLYNK_PRINT Serial

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046 touch(0, 16);

Adafruit_GFX_Button fan_button;
Adafruit_GFX_Button tube_button;
Adafruit_GFX_Button cfl_button;
Adafruit_GFX_Button nite_button;
BlynkTimer timer;

int fan_status = 1;
int tube_status = 1;
int cfl_status = 1;
int nite_status = 1;
int fan_upload = 0;
int tube_upload = 0;
int cfl_upload = 0;
int nite_upload = 0;

BLYNK_WRITE(V0)
{
  fan_status = param.asInt();
  pcf8574.digitalWrite(P0, fan_status);
  if (fan_status == 0) {
    fan_button.drawButton(false);
  }
  else {
    fan_button.drawButton(true);
  }
}
BLYNK_WRITE(V1) {
  tube_status = param.asInt();
  pcf8574.digitalWrite(P1, tube_status);
  if (tube_status == 0) {
    tube_button.drawButton(false);
  }
  else {
    tube_button.drawButton(true);
  }
}
BLYNK_WRITE(V2) {
  cfl_status = param.asInt();
  pcf8574.digitalWrite(P2, cfl_status);
  if (cfl_status == 0) {
    cfl_button.drawButton(false);
  }
  else {
    cfl_button.drawButton(true);
  }
}
BLYNK_WRITE(V3) {
  nite_status = param.asInt();
  pcf8574.digitalWrite(P3, nite_status);
  if (nite_status == 0) {
    nite_button.drawButton(false);
  }
  else {
    nite_button.drawButton(true);
  }
}

BLYNK_CONNECTED() {
  Blynk.syncAll();
}

void setup()
{
  Serial.begin(9600);
  pcf8574.begin();
  Blynk.begin(auth, ssid, pass);
  pinMode(LED_BUILTIN, OUTPUT);
  pcf8574.pinMode(P0, OUTPUT);
  pcf8574.pinMode(P1, OUTPUT);
  pcf8574.pinMode(P2, OUTPUT);
  pcf8574.pinMode(P3, OUTPUT);
  SPI.setFrequency(ESP_SPI_FREQ);
  tft.begin();
  touch.begin(tft.width(), tft.height());
  Serial.print("tftx ="); Serial.print(tft.width()); Serial.print(" tfty ="); Serial.println(tft.height());
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(3);
  touch.setCalibration(1848, 265, 272, 1783);

  tft.setFont();

  fan_button.initButton(&tft, 80, 95, 150, 90, ILI9341_WHITE, ILI9341_WHITE, ILI9341_NAVY, "Fan", 2);
  fan_button.drawButton(true);
  tube_button.initButton(&tft, 80, 190, 150, 90, ILI9341_WHITE, ILI9341_WHITE, ILI9341_NAVY, "Tube", 2);
  tube_button.drawButton(true);
  cfl_button.initButton(&tft, 240, 95, 150, 90, ILI9341_WHITE, ILI9341_WHITE, ILI9341_NAVY, "CFL", 2);
  cfl_button.drawButton(true);
  nite_button.initButton(&tft, 240, 190, 150, 90, ILI9341_WHITE, ILI9341_WHITE, ILI9341_NAVY, "Night", 2);
  nite_button.drawButton(true);

  tft.setFont(&FreeSerif9pt7b);

  tft.setCursor(80, 30);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setTextWrap(true);
  tft.print("Elzi's Abode");

  tft.setFont();

  timer.setInterval(1000L, sendUptime);

}

void sendUptime()
{
  if (fan_upload == 1) {
    Blynk.virtualWrite(V0, fan_status);
    fan_upload = 0;
  }
  
  if (tube_upload == 1) {
    Blynk.virtualWrite(V1, tube_upload);
    tube_upload = 0;
  }
  
  if (cfl_upload == 1) {
    Blynk.virtualWrite(V2, cfl_upload);
    cfl_upload = 0;
  }
  
  if (nite_upload == 1) {
    Blynk.virtualWrite(V3, nite_upload);
    nite_upload = 0;
  }
}

static uint16_t prev_x = 0xffff, prev_y = 0xffff;

void loop()
{
  Blynk.run();
  timer.run();
  uint16_t x, y;
  if (touch.isTouching()) {
    touch.getPosition(x, y);
    x = x + y;
    y = x - y;
    x = 320 - (x - y);
    prev_x = x;
    prev_y = y;
  } else {
    prev_x = prev_y = 0xffff;
  }

  fan_button.press(fan_button.contains(x, y));
  if (fan_button.justPressed()) {
    fan_upload = 1;
    if (fan_status == 0) {
      fan_status = 1;
      fan_button.drawButton(true);
      pcf8574.digitalWrite(P0, HIGH);
    }
    else {
      fan_status = 0;
      fan_button.drawButton(false);
      pcf8574.digitalWrite(P0, LOW);
    }
  }

  tube_button.press(tube_button.contains(x, y));
  if (tube_button.justPressed()) {
    tube_upload = 1;
    if (tube_status == 0) {
      tube_status = 1;
      tube_button.drawButton(true);
      pcf8574.digitalWrite(P1, HIGH);
    }
    else {
      tube_status = 0;
      tube_button.drawButton(false);
      pcf8574.digitalWrite(P1, LOW);
    }
  }

  cfl_button.press(cfl_button.contains(x, y));
  if (cfl_button.justPressed()) {
    cfl_upload = 1;
    if (cfl_status == 0) {
      cfl_status = 1;
      cfl_button.drawButton(true);
      pcf8574.digitalWrite(P2, HIGH);
    }
    else {
      cfl_status = 0;
      cfl_button.drawButton(false);
      pcf8574.digitalWrite(P2, LOW);
    }
  }

  nite_button.press(nite_button.contains(x, y));
  if (nite_button.justPressed()) {
    nite_upload = 1;
    if (nite_status == 0) {
      nite_status = 1;
      nite_button.drawButton(true);
      pcf8574.digitalWrite(P3, HIGH);
    }
    else {
      nite_status = 0;
      nite_button.drawButton(false);
      pcf8574.digitalWrite(P3, LOW);
    }
  }

  delay(20);
}
