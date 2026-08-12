#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"
#include <RTClib.h>
#include <Preferences.h>

// ===== WIFI SETTINGS =====
const char* ssid = "MediTrack_Fast";
const char* password = "12345678";

WebServer server(80);
Preferences prefs;

// ===== PIN DEFINITIONS =====
#define DT1 4
#define SCK1 5
#define DT2 26
#define SCK2 27
#define BUZZER 23
#define LED_PIN 2 
#define BUTTON 25

HX711 scale1, scale2;
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

// ===== VARIABLES =====
float weight1 = 0, weight2 = 0;
float smooth1 = 0, smooth2 = 0;
float gppMorning = 2.0, gppEvening = 2.0;
float cal1 = 290.0, cal2 = 290.0;

int mHour = 8, mMin = 30;
int eHour = 20, eMin = 30;

bool alarmActive = false;
int lastAlarmMinute = -1;
unsigned long lastBeep = 0;
bool buzState = false;
bool stockAlertDone = false; 

// ================= LOAD/SAVE =================
void loadSettings() {
  prefs.begin("med", true);
  gppMorning = prefs.getFloat("gppm", 2.0);
  gppEvening = prefs.getFloat("gppe", 2.0);
  mHour = prefs.getInt("mh", 8);
  mMin = prefs.getInt("mm", 30);
  eHour = prefs.getInt("eh", 20);
  eMin = prefs.getInt("em", 30);
  prefs.end();
}

void saveSettings() {
  prefs.begin("med", false);
  prefs.putFloat("gppm", gppMorning);
  prefs.putFloat("gppe", gppEvening);
  prefs.putInt("mh", mHour);
  prefs.putInt("mm", mMin);
  prefs.putInt("eh", eHour);
  prefs.putInt("em", eMin);
  prefs.end();
}

// ================= WEB DASHBOARD =================
String webpage() {
  return R"====(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>MediTrack FAST</title>
    <style>
        body { font-family: sans-serif; background: #f1f5f9; text-align: center; padding: 20px; }
        .card { background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); margin: 10px auto; max-width: 400px; }
        .val { font-size: 24px; font-weight: bold; color: #2563eb; }
        button { background: #2563eb; color: white; border: none; padding: 10px 20px; border-radius: 5px; cursor: pointer; width: 100%; margin-top:10px; }
        input { padding: 5px; width: 40px; text-align: center; }
    </style>
    <script>
        function update(){
            fetch('/data').then(r=>r.json()).then(d=>{
                document.getElementById("t").innerHTML = d.time;
                document.getElementById("m").innerHTML = d.mw + "g (Qty-" + d.mq + ")";
                document.getElementById("e").innerHTML = d.ew + "g (Qty-" + d.eq + ")";
            });
        }
        setInterval(update, 500);
    </script>
</head>
<body onload="update()">
    <div class="card"><h2>MediTrack</h2><p id="t">00:00:00</p></div>
    <div class="card">Morning: <span class="val" id="m">0</span></div>
    <div class="card">Evening: <span class="val" id="e">0</span></div>
    <div class="card">
        <form action="/setAlarm">
            M: <input name="mh" placeholder="H">:<input name="mm" placeholder="M"><br>
            E: <input name="eh" placeholder="H">:<input name="em" placeholder="M"><br>
            <button type="submit">Set Schedule</button>
        </form>
    </div>
</body>
</html>
)====";
}

void handleData() {
  DateTime now = rtc.now();
  char tBuf[10]; sprintf(tBuf, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  String json = "{";
  json += "\"time\":\"" + String(tBuf) + "\",";
  json += "\"mw\":" + String((int)round(weight1)) + ",";
  json += "\"ew\":" + String((int)round(weight2)) + ",";
  json += "\"mq\":" + String((int)round(weight1/gppMorning)) + ",";
  json += "\"eq\":" + String((int)round(weight2/gppEvening)) + "";
  json += "}";
  server.send(200, "application/json", json);
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  pinMode(BUZZER, OUTPUT); pinMode(LED_PIN, OUTPUT); pinMode(BUTTON, INPUT_PULLUP);
  Wire.begin(21, 22); lcd.init(); lcd.backlight(); rtc.begin();
  scale1.begin(DT1, SCK1); scale2.begin(DT2, SCK2);
  scale1.set_scale(cal1); scale2.set_scale(cal2);
  scale1.tare(); scale2.tare();
  loadSettings();
  WiFi.softAP(ssid, password);
  server.on("/", [](){ server.send(200, "text/html", webpage()); });
  server.on("/data", handleData);
  server.on("/setAlarm", [](){
      if(server.hasArg("mh")) mHour = server.arg("mh").toInt();
      if(server.hasArg("mm")) mMin = server.arg("mm").toInt();
      if(server.hasArg("eh")) eHour = server.arg("eh").toInt();
      if(server.hasArg("em")) eMin = server.arg("em").toInt();
      saveSettings(); server.send(200, "text/html", "Saved <a href='/'>Back</a>");
  });
  server.begin();
}

// ================= LOOP =================
void loop() {
  server.handleClient();

  // Fast Weight Sensing
  if (scale1.is_ready()) {
    float r1 = scale1.get_units(1); 
    smooth1 = (0.2 * r1) + (0.8 * smooth1);
    weight1 = (smooth1 < 0.5) ? 0 : smooth1;
  }
  if (scale2.is_ready()) {
    float r2 = scale2.get_units(1); 
    smooth2 = (0.2 * r2) + (0.8 * smooth2);
    weight2 = (smooth2 < 0.5) ? 0 : smooth2;
  }

  int mq = (int)round(weight1 / gppMorning);
  int eq = (int)round(weight2 / gppEvening);

  // Low Stock Buzzer (2s alert)
  if ((mq < 3 || eq < 3) && (mq > 0 || eq > 0)) {
    if (!stockAlertDone) {
      digitalWrite(BUZZER, HIGH);
      delay(2000);
      digitalWrite(BUZZER, LOW);
      stockAlertDone = true;
    }
  } else if (mq >= 3 && eq >= 3) {
    stockAlertDone = false;
  }

  // Alarm Logic
  DateTime now = rtc.now();
  if ((now.hour() == mHour && now.minute() == mMin) || (now.hour() == eHour && now.minute() == eMin)) {
    if (lastAlarmMinute != now.minute()) { alarmActive = true; lastAlarmMinute = now.minute(); }
  }
  if (digitalRead(BUTTON) == LOW) alarmActive = false;

  if (alarmActive) {
    digitalWrite(LED_PIN, (millis() / 200) % 2);
    if (millis() - lastBeep > 150) {
      lastBeep = millis(); buzState = !buzState;
      digitalWrite(BUZZER, buzState);
    }
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER, LOW);
  }

  // ===== LCD FIXED DISPLAY (Always showing Clock & Alarms) =====
  static unsigned long lastLCD = 0;
  static bool showWeightPage = false;

  // Toggle pages every 4 seconds
  if (millis() - lastLCD > 4000) {
    lastLCD = millis();
    showWeightPage = !showWeightPage;
    lcd.clear();
  }

  if (!showWeightPage) {
    // Page 1: TIME and ALARM SCHEDULE (Jaise pahle tha)
    lcd.setCursor(0, 0);
    char tBuf[17];
    sprintf(tBuf, "Time: %02d:%02d:%02d", now.hour(), now.minute(), now.second());
    lcd.print(tBuf);

    lcd.setCursor(0, 1);
    char sBuf[17];
    sprintf(sBuf, "M:%02d:%02d E:%02d:%02d", mHour, mMin, eHour, eMin);
    lcd.print(sBuf);
  } 
  else {
    // Page 2: QTY and WEIGHT (From your paper note)
    lcd.setCursor(0, 0);
    char mBuf[17];
    sprintf(mBuf, "M:%dg (Qty-%d)", (int)round(weight1), mq);
    lcd.print(mBuf);

    lcd.setCursor(0, 1);
    char eBuf[17];
    sprintf(eBuf, "E:%dg (Qty-%d)", (int)round(weight2), eq);
    lcd.print(eBuf);
  }
}
