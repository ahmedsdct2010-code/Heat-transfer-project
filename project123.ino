#include <Wire.h> 
#include <LiquidCrystal_I2C.h> 
#include "max6675.h" 

const int PELTIER_HEAT_PIN = 26; 
const int PELTIER_COOL_PIN_2 = 21; 
const int PELTIER_COOL_PIN_3 = 22; 
const int PELTIER_COOL_PIN_4 = 23; 
const int COOLER_FAN_1_PIN = 12; 
const int COOLER_FAN_2_PIN = 17; 
const int WATER_PUMP_PIN = 27;

const float TARGET_HEAT_TEMP = 50.0; 
const float TARGET_COOL_TEMP = 10.0; 

const int SCK_PIN = 18; 
const int MISO_PIN = 19; 
const int CS_HOT_PART = 5;     
const int CS_COLD_WATER = 4;    

MAX6675 hot_sensor(SCK_PIN, CS_HOT_PART, MISO_PIN);
MAX6675 cold_sensor(SCK_PIN, CS_COLD_WATER, MISO_PIN);

const int LCD_SDA_PIN = 32; 
const int LCD_SCL_PIN = 33; 
const int LCD_I2C_ADDRESS = 0x27; 
const int LCD_COLS = 16; 
const int LCD_ROWS = 2;  

LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);

float readHotTemperature() {
  float temp = hot_sensor.readCelsius();
  return isnan(temp) ? -99.0 : temp;
}
float readColdTemperature() {
  float temp = cold_sensor.readCelsius();
  return isnan(temp) ? -99.0 : temp;
}

void startCoolingSystem() {
  digitalWrite(PELTIER_COOL_PIN_2, HIGH);
  digitalWrite(PELTIER_COOL_PIN_3, HIGH);
  digitalWrite(PELTIER_COOL_PIN_4, HIGH);
  digitalWrite(COOLER_FAN_1_PIN, HIGH);
  digitalWrite(COOLER_FAN_2_PIN, HIGH);
  digitalWrite(WATER_PUMP_PIN, HIGH);
}

void stopCoolingSystem() {
  digitalWrite(PELTIER_COOL_PIN_2, LOW);
  digitalWrite(PELTIER_COOL_PIN_3, LOW);
  digitalWrite(PELTIER_COOL_PIN_4, LOW);
  digitalWrite(COOLER_FAN_1_PIN, LOW);
  digitalWrite(COOLER_FAN_2_PIN, LOW);
  digitalWrite(WATER_PUMP_PIN, LOW);
}

void updateLCD(float hotTemp, float coldTemp, String status) {
  lcd.setCursor(0, 0);
  lcd.print("H:");
  lcd.print(hotTemp, 1);
  lcd.print((char)223); lcd.print("C ");
  
  lcd.setCursor(8, 0); 
  lcd.print("        "); 
  lcd.setCursor(8, 0);
  lcd.print(status);

  lcd.setCursor(0, 1);
  lcd.print("C:");
  lcd.print(coldTemp, 1);
  lcd.print((char)223); lcd.print("C ");
  lcd.print("      ");
}

void setup() {
  Serial.begin(115200);
  
  pinMode(PELTIER_HEAT_PIN, OUTPUT);
  pinMode(PELTIER_COOL_PIN_2, OUTPUT);
  pinMode(PELTIER_COOL_PIN_3, OUTPUT);
  pinMode(PELTIER_COOL_PIN_4, OUTPUT);
  pinMode(COOLER_FAN_1_PIN, OUTPUT);
  pinMode(COOLER_FAN_2_PIN, OUTPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);
  
  int LCD_SDA_PIN = 32;
  int LCD_SCL_PIN = 33; 

  lcd.init(); lcd.backlight(); 
  lcd.print("System Ready...");

  digitalWrite(PELTIER_HEAT_PIN, LOW);
  stopCoolingSystem();
  
  delay(2000); 
  lcd.clear();
}

void loop() {
  float hotTemp = readHotTemperature();
  float coldTemp = readColdTemperature();
  String systemStatus = "Idle"; 

 
  if (hotTemp < -50.0) {
    digitalWrite(PELTIER_HEAT_PIN, LOW);
    stopCoolingSystem();
    systemStatus = "Fault";
  } else {
  
    if (hotTemp <= TARGET_HEAT_TEMP) {
      stopCoolingSystem(); 
      digitalWrite(PELTIER_HEAT_PIN, HIGH); 
      systemStatus = "Heating";
    }
    if (hotTemp >= TARGET_HEAT_TEMP) {
      digitalWrite(PELTIER_HEAT_PIN, LOW);
   void startcoolingSystem();
    systemStatus = "cooling";
    } else if ( hotTemp = 10){
void stopcoolingSystem();
  }

  updateLCD(hotTemp, coldTemp, systemStatus);
  delay(2000);
}}
/*
Serial.print("Name:");
Serial.print(name);
Serial.print("  ");
Serial.print("age:");
Serial.println(age);
*/