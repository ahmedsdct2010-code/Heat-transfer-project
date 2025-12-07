#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "max6675.h"

MAX6675 thermo1(4, 13, 14);
MAX6675 thermo2(17, 5, 16);

LiquidCrystal_I2C lcd(0x27, 16, 2);
int LCD_SDA = 21;
int LCD_SCL = 22;

  const int fanPin =19;
void setup() {
  lcd.init();
  lcd.backlight();
pinMode(fanPin,OUTPUT);
digitalWrite(fanPin,LOW);
}

void loop() {
  lcd.clear();
  
  lcd.setCursor(0,0);
  lcd.print("T1:");
  lcd.print(thermo1.readCelsius(),1);
  lcd.print("C");
  
  lcd.setCursor(0,1);
  lcd.print("T2:");
  lcd.print(thermo2.readCelsius(),1);
  lcd.print("C");
  
  delay(1000);
  {
  digitalWrite(fanPin, HIGH);
delay(5000);
digitalWrite(fanPin,LOW);
delay(2000);}
}