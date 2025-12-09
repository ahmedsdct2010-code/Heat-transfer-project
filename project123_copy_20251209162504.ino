#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "max6675.h"
#include <WiFi.h>
#include <WebServer.h>

// إعدادات WiFi
const char* ssid = "Your_WiFi_SSID";
const char* password = "Your_WiFi_Password";

// تعريف المنافذ
const int PELTIER_HEAT_PIN = 23;
const int PELTIER_COOL_PIN_2 = 33;
const int PELTIER_COOL_PIN_3 = 32;
const int PELTIER_COOL_PIN_4 = 26;

const int COOLER_FAN_1_PIN = 19;
const int COOLER_FAN_2_PIN = 18;
const int WATER_PUMP_PIN = 27;

// درجات الحرارة المستهدفة (قابلة للتعديل)
float TARGET_HEAT_TEMP = 50.0;
float TARGET_COOL_TEMP = 10.0;
const float HYSTERESIS = 2.0;

// تعريف دبابيس MAX6675
const int SCK_PIN = 4;
const int MISO_PIN = 14;
const int CS_HOT_PART = 13;

const int SCK_PIN1 = 17;
const int MISO_PIN1 = 16;
const int CS_COLD_WATER1 = 5;

// إنشاء كائنات أجهزة الاستشعار
MAX6675 hot_sensor(SCK_PIN, CS_HOT_PART, MISO_PIN);
MAX6675 cold_sensor(SCK_PIN1, CS_COLD_WATER1, MISO_PIN1);

// تعريف دبابيس وشاشة LCD
const int LCD_SDA_PIN = 21;
const int LCD_SCL_PIN = 22;
const int LCD_I2C_ADDRESS = 0x27;
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);

// إنشاء خادم ويب على المنفذ 80
WebServer server(80);

// متغيرات للتحكم
bool heatingActive = false;
bool coolingActive = false;
bool systemAuto = true;
bool coolingEnabled = false;

// قراءة درجة حرارة الجزء الساخن
float readHotTemperature() {
  float temp = hot_sensor.readCelsius();
  return isnan(temp) ? -99.0 : temp;
}

// قراءة درجة حرارة الماء البارد
float readColdTemperature() {
  float temp = cold_sensor.readCelsius();
  return isnan(temp) ? -99.0 : temp;
}

// تشغيل نظام التبريد بالكامل
void startCoolingSystem() {
  if (coolingEnabled) {
    digitalWrite(PELTIER_COOL_PIN_2, HIGH);
    digitalWrite(PELTIER_COOL_PIN_3, HIGH);
    digitalWrite(PELTIER_COOL_PIN_4, HIGH);
    digitalWrite(COOLER_FAN_1_PIN, HIGH);
    digitalWrite(COOLER_FAN_2_PIN, HIGH);
    digitalWrite(WATER_PUMP_PIN, HIGH);
    coolingActive = true;
  }
}

// إيقاف نظام التبريد بالكامل
void stopCoolingSystem() {
  digitalWrite(PELTIER_COOL_PIN_2, LOW);
  digitalWrite(PELTIER_COOL_PIN_3, LOW);
  digitalWrite(PELTIER_COOL_PIN_4, LOW);
  digitalWrite(COOLER_FAN_1_PIN, LOW);
  digitalWrite(COOLER_FAN_2_PIN, LOW);
  digitalWrite(WATER_PUMP_PIN, LOW);
  coolingActive = false;
}

// تشغيل نظام التسخين
void startHeatingSystem() {
  digitalWrite(PELTIER_HEAT_PIN, HIGH);
  heatingActive = true;
}

// إيقاف نظام التسخين
void stopHeatingSystem() {
  digitalWrite(PELTIER_HEAT_PIN, LOW);
  heatingActive = false;
}

// تحديث شاشة LCD
void updateLCD(float hotTemp, float coldTemp, String status) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("H:");
  lcd.print(hotTemp, 1);
  lcd.print((char)223);
  lcd.print("C ");

  lcd.setCursor(8, 0);
  lcd.print(status);

  lcd.setCursor(0, 1);
  lcd.print("C:");
  lcd.print(coldTemp, 1);
  lcd.print((char)223);
  lcd.print("C ");

  if (!systemAuto) {
    lcd.setCursor(13, 1);
    lcd.print("M");
  }
}

// صفحة الويب الرئيسية
void handleRoot() {
  float hotTemp = readHotTemperature();
  float coldTemp = readColdTemperature();

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body{font-family:Arial;text-align:center;background:#f0f0f0;}";
  html += ".container{max-width:600px;margin:auto;padding:20px;}";
  html += ".card{background:white;border-radius:10px;padding:20px;margin:10px;box-shadow:0 4px 6px rgba(0,0,0,0.1);}";
  html += ".temp{font-size:48px;color:#333;}";
  html += ".button{padding:12px 24px;margin:5px;border:none;border-radius:5px;cursor:pointer;font-size:16px;}";
  html += ".heat{background:#ff6b6b;color:white;}";
  html += ".cool{background:#4ecdc4;color:white;}";
  html += ".auto{background:#45b7d1;color:white;}";
  html += ".off{background:#95a5a6;color:white;}";
  html += ".status{font-size:24px;margin:10px;}";
  html += "</style>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h1>🌡️ ESP32 Temperature Control</h1>";

  html += "<div class='card'>";
  html += "<h2>Current Temperatures</h2>";
  html += "<div class='temp'>🔥 " + String(hotTemp, 1) + "°C</div>";
  html += "<div class='temp'>❄️ " + String(coldTemp, 1) + "°C</div>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<h2>System Status</h2>";
  html += "<div class='status'>";
  html += "Heating: " + String(heatingActive ? "✅ ON" : "❌ OFF") + "<br>";
  html += "Cooling: " + String(coolingActive ? "✅ ON" : "❌ OFF") + "<br>";
  html += "Mode: " + String(systemAuto ? "🤖 AUTO" : "👤 MANUAL");
  html += "</div></div>";

  html += "<div class='card'>";
  html += "<h2>Manual Control</h2>";
  html += "<button class='button heat' onclick=\"location.href='/heat_on'\">🔥 Start Heating</button>";
  html += "<button class='button off' onclick=\"location.href='/heat_off'\">⛔ Stop Heating</button><br>";
  html += "<button class='button cool' onclick=\"location.href='/cool_on'\">❄️ Start Cooling</button>";
  html += "<button class='button off' onclick=\"location.href='/cool_off'\">⛔ Stop Cooling</button><br>";
  html += "<button class='button auto' onclick=\"location.href='/auto'\">🤖 Auto Mode</button>";
  html += "</div>";

  html += "<div class='card'>";
  html += "<h2>Temperature Settings</h2>";
  html += "<form action='/update' method='GET'>";
  html += "Heat Target: <input type='number' step='0.1' name='heat' value='" + String(TARGET_HEAT_TEMP, 1) + "'><br><br>";
  html += "Cool Target: <input type='number' step='0.1' name='cool' value='" + String(TARGET_COOL_TEMP, 1) + "'><br><br>";
  html += "<input class='button auto' type='submit' value='💾 Update Targets'>";
  html += "</form></div>";

  html += "</div></body></html>";

  server.send(200, "text/html", html);
}

// معالجة طلبات التحكم
void handleHeatOn() {
  systemAuto = false;
  coolingEnabled = false;
  startHeatingSystem();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleHeatOff() {
  stopHeatingSystem();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleCoolOn() {
  systemAuto = false;
  coolingEnabled = true;
  startCoolingSystem();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleCoolOff() {
  stopCoolingSystem();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleAuto() {
  systemAuto = true;
  coolingEnabled = true;
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleUpdate() {
  if (server.hasArg("heat")) {
    TARGET_HEAT_TEMP = server.arg("heat").toFloat();
  }
  if (server.hasArg("cool")) {
    TARGET_COOL_TEMP = server.arg("cool").toFloat();
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void connectToWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to");
  lcd.setCursor(0, 1);
  lcd.print("WiFi...");

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");
    lcd.setCursor(0, 1);
    lcd.print("IP:");
    lcd.print(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed");
    lcd.setCursor(0, 1);
    lcd.print("Using Auto Mode");
  }
  delay(2000);
  lcd.clear();
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nESP32 Temperature Control with WiFi");

  // تهيئة منافذ الإخراج
  pinMode(PELTIER_HEAT_PIN, OUTPUT);
  pinMode(PELTIER_COOL_PIN_2, OUTPUT);
  pinMode(PELTIER_COOL_PIN_3, OUTPUT);
  pinMode(PELTIER_COOL_PIN_4, OUTPUT);
  pinMode(COOLER_FAN_1_PIN, OUTPUT);
  pinMode(COOLER_FAN_2_PIN, OUTPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);

  // التأكد من إيقاف كل الأنظمة
  stopHeatingSystem();
  stopCoolingSystem();

  // تهيئة I2C وLCD
  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ESP32 Temp Ctrl");
  lcd.setCursor(0, 1);
  lcd.print("with WiFi");

  delay(2000);

  // الاتصال بـ WiFi
  connectToWiFi();

  // إعداد مسارات الويب
  server.on("/", handleRoot);
  server.on("/heat_on", handleHeatOn);
  server.on("/heat_off", handleHeatOff);
  server.on("/cool_on", handleCoolOn);
  server.on("/cool_off", handleCoolOff);
  server.on("/auto", handleAuto);
  server.on("/update", handleUpdate);

  // بدء خادم الويب
  server.begin();
  Serial.println("HTTP server started");

  lcd.clear();
  lcd.setCursor(0, 0);
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print("Web Interface:");
    lcd.setCursor(0, 1);
    IPAddress ip = WiFi.localIP();
    lcd.print(ip[2]);
    lcd.print(".");
    lcd.print(ip[3]);
  } else {
    lcd.print("Offline Mode");
  }
  delay(2000);
}

void loop() {
  // معالجة طلبات الويب
  server.handleClient();

  // قراءة درجات الحرارة
  float hotTemp = readHotTemperature();
  float coldTemp = readColdTemperature();
  String systemStatus = "Idle";

  // التحقق من وجود خطأ في أجهزة الاستشعار
  if (hotTemp < -50.0 || coldTemp < -50.0) {
    if (systemAuto) {
      stopHeatingSystem();
      stopCoolingSystem();
      systemStatus = "Fault";
    }
  } else if (systemAuto) {
    // منطق التحكم الآلي مع Hysteresis
    if (hotTemp < (TARGET_HEAT_TEMP - HYSTERESIS)) {
      stopCoolingSystem();
      startHeatingSystem();
      systemStatus = "Heating";
    } else if (hotTemp > (TARGET_HEAT_TEMP + HYSTERESIS)) {
      stopHeatingSystem();

      if (coldTemp > (TARGET_COOL_TEMP + HYSTERESIS)) {
        startCoolingSystem();
        systemStatus = "Cooling";
      } else if (coldTemp < (TARGET_COOL_TEMP - HYSTERESIS)) {
        stopCoolingSystem();
        systemStatus = "Idle";
      }
    }
  } else {
    // الوضع اليدوي
    if (heatingActive) systemStatus = "Heat(Man)";
    else if (coolingActive) systemStatus = "Cool(Man)";
    else systemStatus = "Manual";
  }

  // تحديث الشاشة
  updateLCD(hotTemp, coldTemp, systemStatus);

  // طباعة البيانات على Serial Monitor
  Serial.print("Hot: ");
  Serial.print(hotTemp);
  Serial.print("°C | Cold: ");
  Serial.print(coldTemp);
  Serial.print("°C | Status: ");
  Serial.print(systemStatus);
  Serial.print(" | Mode: ");
  Serial.println(systemAuto ? "Auto" : "Manual");

  delay(2000);
}