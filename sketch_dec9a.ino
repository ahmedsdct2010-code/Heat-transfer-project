#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "max6675.h"

const int PELTIER_HEAT_PIN = 23;    
const int PELTIER_COOL_PIN_1 = 33; 
const int PELTIER_COOL_PIN_2 = 32;   
const int PELTIER_COOL_PIN_3 = 26;   

const int COOLER_FAN_1_PIN = 19;     
const int COOLER_FAN_2_PIN = 18;    
const int WATER_PUMP_PIN = 27;       

const int SCK_PIN = 4;
const int MISO_PIN = 14;
const int CS_HOT_PART = 13;

const int SCK_PIN1 = 17;
const int MISO_PIN1 = 16;
const int CS_COLD_WATER1 = 5;

const int LCD_SDA_PIN = 32;
const int LCD_SCL_PIN = 33;
LiquidCrystal_I2C lcd(0x27, 16, 2);

MAX6675 hot_temp(SCK_PIN, CS_HOT_PART, MISO_PIN);
MAX6675 cold_temp(SCK_PIN1, CS_COLD_WATER1, MISO_PIN1);

const float START_TEMP = 50.0; 
const float END_TEMP = 20.0;  

enum SystemState { HEATING, COOLING, FINISHED };
SystemState state = HEATING; 

unsigned long startMillis = 0;
unsigned long endMillis = 0;
float coolingTime = 0;

void startHeating() { digitalWrite(PELTIER_HEAT_PIN, HIGH); }
void stopHeating()  { digitalWrite(PELTIER_HEAT_PIN, LOW); }

void startCooling() {
  digitalWrite(PELTIER_COOL_PIN_1, HIGH);
  digitalWrite(PELTIER_COOL_PIN_2, HIGH);
  digitalWrite(PELTIER_COOL_PIN_3, HIGH);

  digitalWrite(COOLER_FAN_1_PIN, HIGH);  
  digitalWrite(COOLER_FAN_2_PIN, HIGH);

  digitalWrite(WATER_PUMP_PIN, HIGH);    
}

void stopCooling() {
  digitalWrite(PELTIER_COOL_PIN_1, LOW);
  digitalWrite(PELTIER_COOL_PIN_2, LOW);
  digitalWrite(PELTIER_COOL_PIN_3, LOW);

  digitalWrite(COOLER_FAN_1_PIN, LOW);   
  digitalWrite(COOLER_FAN_2_PIN, LOW);
  digitalWrite(WATER_PUMP_PIN, LOW);    
}

void stopAll() {
  stopHeating();
  stopCooling();
}

void updateLCD(float hotT, float coldT) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("H:");
  lcd.print(hotT, 1);
  lcd.print("C C:");
  lcd.print(coldT, 1);

  lcd.setCursor(0, 1);
  if (state == HEATING) lcd.print("State: HEATING");
  else if (state == COOLING) lcd.print("State: COOLING");
  else if (state == FINISHED) {
    lcd.print("Time:");
    lcd.print(coolingTime, 1);
    lcd.print("s");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PELTIER_HEAT_PIN, OUTPUT);
  pinMode(PELTIER_COOL_PIN_1, OUTPUT);
  pinMode(PELTIER_COOL_PIN_2, OUTPUT);
  pinMode(PELTIER_COOL_PIN_3, OUTPUT);
  pinMode(COOLER_FAN_1_PIN, OUTPUT);
  pinMode(COOLER_FAN_2_PIN, OUTPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);

  stopAll();

  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Cooling Project");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(1500);
  lcd.clear();
}

void loop() {
  float hot = hot_temp.readCelsius();
  float cold = cold_temp.readCelsius();


  if (state == HEATING) {
    startHeating();
    stopCooling();

    if (hot >= START_TEMP) {
      stopHeating();
      startMillis = millis();
      state = COOLING;
      Serial.println("Reached 50C -> Cooling...");
    }
  }
  else if (state == COOLING) {
    stopHeating();
    startCooling();

    if (hot <= END_TEMP) {
      stopCooling();
      endMillis = millis();
      coolingTime = (endMillis - startMillis) / 1000.0;
      state = FINISHED;

      Serial.print("Cooling Finished. Time = ");
      Serial.print(coolingTime);
      Serial.println(" sec");
    }
  }

  updateLCD(hot, cold);

  Serial.print("Hot = ");
  Serial.print(hot);
  Serial.print(" | Cold = ");
  Serial.print(cold);
  Serial.print(" | State = ");
  if (state == HEATING) Serial.println("HEATING");
  else if (state == COOLING) Serial.println("COOLING");
  else Serial.println("FINISHED");

  delay(500);
}
