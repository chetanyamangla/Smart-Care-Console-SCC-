#include <Arduino.h>
#define BLYNK_TEMPLATE_ID "TMPL3DxjUcaz4"
#define BLYNK_TEMPLATE_NAME "Voice Assistant"
#define BLYNK_AUTH_TOKEN "URCcPi2gGaYQ9BEXXkV9qivThJViSq6j"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <time.h>

char ssid[] = "CHETANYA_WIFI";
char pass[] = "120812@@";
const char *TIME_ZONE = "IST-5:30";

const int LED_PIN = 18, FAN_PIN = 5, TV_PIN = 4, MASSAGER_PIN = 19;
const int SERVO1_PIN = 25, SERVO2_PIN = 26, BUZZER_PIN = 27;
// Set false only for a PASSIVE piezo buzzer (tone generated in software).
const bool ACTIVE_BUZZER = true;
const bool BUZZER_ACTIVE_HIGH = true;
const int STOP_PULSE = 1500, FORWARD_PULSE = 1300, REVERSE_PULSE = 1700;
const uint32_t FORWARD_TIME = 366, REVERSE_TIME = 366;
const uint32_t ALARM_DURATION = 5000, SERVO2_REVERSE_START = 2000;

Servo servo1, servo2;
Preferences prefs;
struct Alarm {
  int hour, minute, lastDay;
  bool enabled, active;
  uint32_t started;
  int stage;
};
Alarm alarms[2];
bool servoReady[2] = {false, false};
bool buzzerOn = false;
uint32_t lastWifiTry = 0, lastBlynkTry = 0, lastStatus = 0;
uint32_t lastAlarmCheck = 0;

void buzzerOutput(bool on) {
  digitalWrite(BUZZER_PIN, (on == BUZZER_ACTIVE_HIGH) ? HIGH : LOW);
}
void serviceBuzzer() {
  bool shouldBeep = false;
  uint32_t now = millis();
  for (int i=0; i<2; i++) {
    if (alarms[i].active && now - alarms[i].started < ALARM_DURATION &&
        ((now - alarms[i].started)/250)%2 == 0) shouldBeep = true;
  }
  if (ACTIVE_BUZZER) {
    buzzerOutput(shouldBeep);
  } else {
    // Approx 2 kHz square wave for passive piezo, no servo PWM timer changes.
    static uint32_t lastEdge = 0;
    static bool level = false;
    if (!shouldBeep) { level = false; buzzerOutput(false); }
    else if ((uint32_t)(micros()-lastEdge) >= 250) {
      lastEdge = micros(); level = !level; buzzerOutput(level);
    }
  }
  if (shouldBeep != buzzerOn) {
    buzzerOn = shouldBeep;
    Serial.printf("Buzzer GPIO%d: %s\n", BUZZER_PIN, shouldBeep ? "ON" : "OFF");
  }
}
void saveAlarm(int i) {
  String k = "a" + String(i);
  prefs.putInt((k+"h").c_str(), alarms[i].hour);
  prefs.putInt((k+"m").c_str(), alarms[i].minute);
  prefs.putBool((k+"e").c_str(), alarms[i].enabled);
}
void stopAlarm(int i) {
  if (!alarms[i].active) return;
  alarms[i].active = false;
  if (i == 0 && servoReady[0]) servo1.write(0);
  if (i == 1 && servoReady[1]) servo2.writeMicroseconds(STOP_PULSE);
  Serial.printf("Alarm %d stopped\n", i+1);
}
void setAlarmHour(int i, int value) {
  if (value < 0 || value > 23) return;
  alarms[i].hour = value; saveAlarm(i);
  Serial.printf("Alarm %d hour = %02d\n", i+1, value);
}
void setAlarmMinute(int i, int value) {
  if (value < 0 || value > 59) return;
  alarms[i].minute = value; saveAlarm(i);
  Serial.printf("Alarm %d minute = %02d\n", i+1, value);
}
void setAlarmEnabled(int i, int value) {
  alarms[i].enabled = (value != 0);
  saveAlarm(i);
  if (!alarms[i].enabled) stopAlarm(i);
  Serial.printf("Alarm %d %s at %02d:%02d\n", i+1,
                alarms[i].enabled ? "ENABLED" : "DISABLED",
                alarms[i].hour, alarms[i].minute);
}
void startAlarm(int i, bool manualTest = false) {
  if (alarms[i].active) return;
  if (!servoReady[i]) {
    Serial.printf("Alarm %d cannot start: servo not attached\n", i+1);
    return;
  }
  alarms[i].active = true;
  alarms[i].started = millis();
  alarms[i].stage = 0;
  if (i == 0) servo1.write(90);
  else servo2.writeMicroseconds(FORWARD_PULSE);
  Serial.printf("*** ALARM %d STARTED (%s) ***\n", i+1,
                manualTest ? "MANUAL TEST" : "SCHEDULED");
}
void serviceAlarms() {
  uint32_t now = millis();
  for (int i=0; i<2; i++) {
    Alarm &a = alarms[i];
    if (!a.active) continue;
    uint32_t elapsed = now - a.started;
    if (i == 1) {
      if (a.stage == 0 && elapsed >= FORWARD_TIME) {
        servo2.writeMicroseconds(STOP_PULSE);
        a.stage = 1;
        Serial.println("Servo 2 STOP after forward 366 ms");
      }
      if (a.stage == 1 && elapsed >= SERVO2_REVERSE_START) {
        servo2.writeMicroseconds(REVERSE_PULSE);
        a.stage = 2;
        Serial.println("Servo 2 REVERSE for 366 ms");
      }
      if (a.stage == 2 && elapsed >= SERVO2_REVERSE_START + REVERSE_TIME) {
        servo2.writeMicroseconds(STOP_PULSE);
        a.stage = 3;
        Serial.println("Servo 2 STOP after reverse");
      }
    }
    if (elapsed >= ALARM_DURATION) stopAlarm(i);
  }
}
void appliance(int virtualPin, const char *name, int gpio, int value) {
  digitalWrite(gpio, value ? HIGH : LOW);
  Serial.printf("Blynk V%d %s = %d, GPIO%d = %d\n",
                virtualPin, name, value, gpio, digitalRead(gpio));
}
BLYNK_WRITE(V0) { appliance(0,"LIGHT",LED_PIN,param.asInt()); }
BLYNK_WRITE(V1) { appliance(1,"FAN",FAN_PIN,param.asInt()); }
BLYNK_WRITE(V2) { appliance(2,"TV",TV_PIN,param.asInt()); }
BLYNK_WRITE(V3) { appliance(3,"MASSAGER",MASSAGER_PIN,param.asInt()); }
BLYNK_WRITE(V4) { setAlarmHour(0,param.asInt()); }
BLYNK_WRITE(V5) { setAlarmMinute(0,param.asInt()); }
BLYNK_WRITE(V6) { setAlarmEnabled(0,param.asInt()); }
BLYNK_WRITE(V7) { setAlarmHour(1,param.asInt()); }
BLYNK_WRITE(V8) { setAlarmMinute(1,param.asInt()); }
BLYNK_WRITE(V9) { setAlarmEnabled(1,param.asInt()); }
BLYNK_CONNECTED() {
  Serial.println("Blynk connected; syncing dashboard");
  Blynk.syncAll();
}
void checkSchedule() {
  if (millis() - lastAlarmCheck < 200) return;
  lastAlarmCheck = millis();
  time_t utc = time(nullptr);
  if (utc < 1700000000) return;
  struct tm local;
  localtime_r(&utc, &local);
  int day = (local.tm_year+1900)*10000 + (local.tm_mon+1)*100 + local.tm_mday;
  static int lastPrintedSecond = -1;
  if (local.tm_sec != lastPrintedSecond) {
    lastPrintedSecond = local.tm_sec;
    Serial.printf("IST %02d:%02d:%02d | A1 %02d:%02d %s | A2 %02d:%02d %s\n",
      local.tm_hour, local.tm_min, local.tm_sec,
      alarms[0].hour, alarms[0].minute, alarms[0].enabled ? "ON":"OFF",
      alarms[1].hour, alarms[1].minute, alarms[1].enabled ? "ON":"OFF");
  }
  for (int i=0; i<2; i++) {
    Alarm &a = alarms[i];
    if (a.enabled && !a.active && a.hour == local.tm_hour &&
        a.minute == local.tm_min && a.lastDay != day) {
      if (!servoReady[i]) {
        Serial.printf("Alarm %d due, but servo unavailable\n",i+1);
        continue;
      }
      // Persist once-per-day record before dispensing to avoid repeat on reboot.
      a.lastDay = day;
      String key = "a" + String(i) + "d";
      prefs.putInt(key.c_str(), day);
      startAlarm(i);
    }
  }
}
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("ESP32 medicine controller starting");
  for (int p : {LED_PIN,FAN_PIN,TV_PIN,MASSAGER_PIN}) {
    pinMode(p,OUTPUT); digitalWrite(p,LOW);
  }
  pinMode(BUZZER_PIN,OUTPUT);
  buzzerOutput(false);
  prefs.begin("medicine",false);
  for (int i=0;i<2;i++) {
    String k="a"+String(i);
    alarms[i] = {prefs.getInt((k+"h").c_str(),8),
                 prefs.getInt((k+"m").c_str(),i*10),
                 prefs.getInt((k+"d").c_str(),-1),
                 prefs.getBool((k+"e").c_str(),false),false,0,0};
  }
  servo1.setPeriodHertz(50);
  servo2.setPeriodHertz(50);
  servo1.attach(SERVO1_PIN,500,2400);
  servo2.attach(SERVO2_PIN,1000,2000);
  servoReady[0] = servo1.attached();
  servoReady[1] = servo2.attached();
  if (servoReady[0]) servo1.write(0);
  if (servoReady[1]) servo2.writeMicroseconds(STOP_PULSE);
  Serial.printf("Servos: GPIO25=%s GPIO26=%s\n",
      servoReady[0] ? "READY":"FAILED",servoReady[1] ? "READY":"FAILED");
  configTzTime(TIME_ZONE,"pool.ntp.org","time.google.com");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,pass);
  Blynk.config(BLYNK_AUTH_TOKEN);
  Serial.println("Serial test: type 1 for alarm 1, 2 for alarm 2, S to stop both");
}
void loop() {
  // Serial tests do not record a dose or change saved alarm settings.
  while (Serial.available()) {
    char c = Serial.read();
    if (c=='1') startAlarm(0,true);
    if (c=='2') startAlarm(1,true);
    if (c=='s' || c=='S') { stopAlarm(0); stopAlarm(1); buzzerOutput(false); }
  }
  serviceAlarms();
  serviceBuzzer();
  checkSchedule();
  uint32_t now=millis();
  if (WiFi.status()!=WL_CONNECTED) {
    if (now-lastWifiTry>=10000) {
      lastWifiTry=now;
      Serial.println("Wi-Fi reconnecting...");
      WiFi.reconnect();
    }
  } else if (Blynk.connected()) {
    Blynk.run();
  } else if (now-lastBlynkTry>=3000) {
    lastBlynkTry=now;
    Serial.println("Blynk connecting...");
    Blynk.connect(100);
  }
  if (now-lastStatus>=10000) {
    lastStatus=now;
    Serial.printf("Status: Wi-Fi=%s Blynk=%s Time=%s\n",
      WiFi.status()==WL_CONNECTED?"OK":"OFF",
      Blynk.connected()?"OK":"OFF",
      time(nullptr)>=1700000000?"OK":"WAITING");
  }
}
