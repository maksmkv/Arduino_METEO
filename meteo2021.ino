//ПЕРЕМЕННЫЕ
#define RESET_CLOCK 0       // сброс часов на время загрузки прошивки (для модуля с несъёмной батарейкой). Не забудь поставить 0 и прошить ещё раз! 
#define SENS_TIME 500     // время обновления показаний сенсоров на экране, миллисекунд
#define DISPLAY_ADDR 0x3F

boolean dotFlag;

//БИБЛИОТЕКИ
#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

LiquidCrystal_I2C lcd(DISPLAY_ADDR, 20, 4);

#include "RTClib.h"
RTC_DS1307 rtc;
DateTime now;

int8_t hrs, mins, secs;
byte mode = 0;
/*
  0 часы и данные
  1 Данные BME
*/


#include <lib_bl999.h>
static BL999Info info;

#define SEALEVELPRESSURE_HPA (1013.25)
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
Adafruit_BME280 bme;

unsigned long delayTime;

#include <GyverTimer.h>
GTimer_ms clockTimer1(500);
GTimer_ms drawSensorsTimer(SENS_TIME);
GTimer_ms plotTimer(240000);

#include "GyverButton.h"
#define BTN_PIN 4  //Пин кнопки 4
GButton button(BTN_PIN, LOW_PULL, NORM_OPEN);

// цифры
uint8_t LT[8] = {0b00111,  0b01111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
uint8_t UB[8] = {0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000};
uint8_t RT[8] = {0b11100,  0b11110,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
uint8_t LL[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b01111,  0b00111};
uint8_t LB[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};
uint8_t LR[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11110,  0b11100};
uint8_t UMB[8] = {0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111};
uint8_t LMB[8] = {0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};

void drawDig(byte dig, byte x, byte y) {
  switch (dig) {
    case 0:
      lcd.setCursor(x, y); // set cursor to column 0, line 0 (first row)
      lcd.write(0);  // call each segment to create
      lcd.write(1);  // top half of the number
      lcd.write(2);
      lcd.setCursor(x, y + 1); // set cursor to colum 0, line 1 (second row)
      lcd.write(3);  // call each segment to create
      lcd.write(4);  // bottom half of the number
      lcd.write(5);
      break;
    case 1:
      lcd.setCursor(x + 1, y);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(x + 2, y + 1);
      lcd.write(5);
      break;
    case 2:
      lcd.setCursor(x, y);
      lcd.write(6);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(7);
      break;
    case 3:
      lcd.setCursor(x, y);
      lcd.write(6);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(7);
      lcd.write(7);
      lcd.write(5);
      break;
    case 4:
      lcd.setCursor(x, y);
      lcd.write(3);
      lcd.write(4);
      lcd.write(2);
      lcd.setCursor(x + 2, y + 1);
      lcd.write(5);
      break;
    case 5:
      lcd.setCursor(x, y);
      lcd.write(0);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(x, y + 1);
      lcd.write(7);
      lcd.write(7);
      lcd.write(5);
      break;
    case 6:
      lcd.setCursor(x, y);
      lcd.write(0);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(5);
      break;
    case 7:
      lcd.setCursor(x, y);
      lcd.write(1);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(x + 1, y + 1);
      lcd.write(0);
      break;
    case 8:
      lcd.setCursor(x, y);
      lcd.write(0);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(5);
      break;
    case 9:
      lcd.setCursor(x, y);
      lcd.write(0);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x + 1, y + 1);
      lcd.write(4);
      lcd.write(5);
      break;
    case 10:
      lcd.setCursor(x, y);
      lcd.write(32);
      lcd.write(32);
      lcd.write(32);
      lcd.setCursor(x, y + 1);
      lcd.write(32);
      lcd.write(32);
      lcd.write(32);
      break;
  }
}

void drawdots(byte x, byte y, boolean state) {
  byte code;
  if (state) code = 165;
  else code = 32;
  lcd.setCursor(x, y);
  lcd.write(code);
  lcd.setCursor(x, y + 1);
  lcd.write(code);
}

void drawClock(byte hours, byte minutes, byte x, byte y, boolean dotState) {
  lcd.setCursor(x, y);
  lcd.print("               ");
  lcd.setCursor(x, y + 1);
  lcd.print("               ");

  if (hours / 10 == 0) drawDig(10, x, y);
  else drawDig(hours / 10, x, y);

  drawDig(hours % 10, x + 4, y);
  drawDig(minutes / 10, x + 8, y);
  drawDig(minutes % 10, x + 12, y);
}

void loadClock() {
  lcd.createChar(0, LT);
  lcd.createChar(1, UB);
  lcd.createChar(2, RT);
  lcd.createChar(3, LL);
  lcd.createChar(4, LB);
  lcd.createChar(5, LR);
  lcd.createChar(6, UMB);
  lcd.createChar(7, LMB);
}

#if (WEEK_LANG == 0)
static const char *dayNames[]  = {
  "Sund",
  "Mond",
  "Tues",
  "Wedn",
  "Thur",
  "Frid",
  "Satu",
};
#else
static const char *dayNames[]  = {
  "BOCK",
  "POND",
  "BTOP",
  "CPED",
  "4ETB",
  "5YAT",
  "CYBB",
};
#endif

void drawData() {
  lcd.setCursor(15, 0);
  if (now.day() < 10) lcd.print(0);
  lcd.print(now.day());
  lcd.print(".");
  if (now.month() < 10) lcd.print(0);
  lcd.print(now.month());
  lcd.setCursor(16, 1);
  int dayofweek = now.dayOfTheWeek();
  lcd.print(dayNames[dayofweek]);
}

void clockTick() {
  dotFlag = !dotFlag;
  if (dotFlag) {          // каждую секунду пересчёт времени
    secs++;
    if (secs > 59) {      // каждую минуту
      secs = 0;
      mins++;
      if (mins <= 59 && mode == 0) drawClock(hrs, mins, 0, 0, 1);
    }
    if (mins > 59) {      // каждый час
      now = rtc.now();
      secs = now.second();
      mins = now.minute();
      hrs = now.hour();
      if (mode == 0) drawClock(hrs, mins, 0, 0, 1);
      if (hrs > 23)
      {
        hrs = 0;
      }
    }
  }
  drawdots(7, 0, dotFlag);
}

void DrawBL999() {
  if (bl999_have_message() && bl999_get_message(info)) {
    lcd.setCursor(0, 2);
    lcd.print("Ch:" + String(info.channel));
    lcd.setCursor(5, 2);
    lcd.print("Pow: " + String(info.powerUUID));
    lcd.setCursor(13, 2);
    lcd.print("Bat:" + String(info.battery == 0 ? "Ok" : "Low"));
    lcd.setCursor(0, 3);
    lcd.print("Temp: " + String(info.temperature / 10.0));
    lcd.setCursor(12, 3);
    lcd.print("Hum: " + String(info.humidity) + "%");
  }
}

void DrawBME() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Temp in: " + String(bme.readTemperature()) + " C  ");
  lcd.setCursor(1, 1);
  lcd.print("Hum: " + String(bme.readHumidity()) + " %      ");
  lcd.setCursor(1, 2);
  lcd.print("Pre: " + String(bme.readPressure() / 133.33, 1) + " mm Hg   ");
  lcd.setCursor(1, 3);
  lcd.print("App. Alt = " + String(bme.readAltitude(SEALEVELPRESSURE_HPA)) + " m");
}

void setup() {
  lcd.init();
  lcd.clear();
  lcd.noBacklight();

  bme.begin();

  rtc.begin();
  if (RESET_CLOCK)
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();

  loadClock();
  drawClock(hrs, mins, 0, 0, 1);
  drawData();

  bl999_set_rx_pin(2);
  bl999_rx_start();
}

void loop() {
  if (clockTimer1.isReady()) {
    clockTick();
  }
  modesTick();

  if (mode == 0) {                                  // в режиме "главного экрана"
    if (drawSensorsTimer.isReady()) DrawBL999();  // обновляем показания датчиков на дисплее с периодом SENS_TIME
  } else {                                          // в любом из графиков
    if (plotTimer.isReady()) redrawPlot();          // перерисовываем график
  }

}
