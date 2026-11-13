
#include <Arduino.h>

#define BLYNK_TEMPLATE_ID "YOUR_BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_BLYNK_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_arduino_version.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <time.h>

// ---------------- WIFI AND TIME ----------------

char ssid[] = "YOUR_SSID";
char pass[] = "YOUR_SSID_PASSWORD";

const char *TIME_ZONE = "IST-5:30";

// Must match the transmitter's channel.
// Your transmitter currently uses channel 1.
#define GLASSES_WIFI_CHANNEL 1

// ---------------- APPLIANCE GPIO PINS ----------------

const int LED_PIN = 18;
const int FAN_PIN = 5;
const int TV_PIN = 4;
const int MASSAGER_PIN = 19;

// ---------------- MEDICINE BOX GPIO PINS ----------------

const int SERVO1_PIN = 25;
const int SERVO2_PIN = 26;
const int BUZZER_PIN = 27;

// ---------------- BUZZER SETTINGS ----------------

const bool ACTIVE_BUZZER = true;
const bool BUZZER_ACTIVE_HIGH = true;

// ---------------- SERVO SETTINGS ----------------

const int STOP_PULSE = 1500;
const int FORWARD_PULSE = 1300;
const int REVERSE_PULSE = 1700;

const uint32_t FORWARD_TIME = 366;
const uint32_t REVERSE_TIME = 366;

const uint32_t ALARM_DURATION = 5000;
const uint32_t SERVO2_REVERSE_START = 2000;

// ---------------- ESP-NOW SETTINGS ----------------

// EXACT packet structure used by your ESP32-C3 transmitter.

struct BlinkPacket {
  uint32_t magic;
  uint32_t sequence;
  uint8_t command;
} __attribute__((packed));

const uint32_t BLINK_MAGIC = 0xB11A2026;

static_assert(sizeof(BlinkPacket) == 9,
              "BlinkPacket size mismatch");

// Receive command data safely from the ESP-NOW callback.
portMUX_TYPE espNowMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool commandPending = false;
volatile uint8_t pendingCommand = 0;
volatile uint32_t pendingSequence = 0;

uint32_t lastReceivedSequence = 0;
bool espNowReady = false;

// ---------------- OBJECTS ----------------

Servo servo1;
Servo servo2;

Preferences prefs;

struct Alarm {
  int hour;
  int minute;
  int lastDay;
  bool enabled;
  bool active;
  uint32_t started;
  int stage;
};

Alarm alarms[2];

bool servoReady[2] = {false, false};
bool buzzerOn = false;

uint32_t lastWifiTry = 0;
uint32_t lastBlynkTry = 0;
uint32_t lastStatus = 0;
uint32_t lastAlarmCheck = 0;

// ==================================================
// ESP-NOW RECEIVER CALLBACK
// ==================================================

#if ESP_ARDUINO_VERSION_MAJOR >= 3

void onEspNowReceive(
  const esp_now_recv_info_t *info,
  const uint8_t *data,
  int len
) {

#else

void onEspNowReceive(
  const uint8_t *mac,
  const uint8_t *data,
  int len
) {

#endif

  if (len != sizeof(BlinkPacket)) {
    return;
  }

  BlinkPacket packet;
  memcpy(&packet, data, sizeof(packet));

  // Validate the transmitter's magic value.
  if (packet.magic != BLINK_MAGIC) {
    return;
  }

  // Ignore duplicate transmissions.
  // Your transmitter sends each command three times
  // using the same sequence number.

  if (packet.sequence == lastReceivedSequence) {
    return;
  }

  if (packet.command < 1 || packet.command > 8) {
    return;
  }

  lastReceivedSequence = packet.sequence;

  portENTER_CRITICAL(&espNowMux);

  pendingCommand = packet.command;
  pendingSequence = packet.sequence;
  commandPending = true;

  portEXIT_CRITICAL(&espNowMux);
}

// ==================================================
// PROCESS RECEIVED ESP-NOW COMMANDS
// ==================================================

void processEspNowCommand() {

  uint8_t command = 0;
  uint32_t sequence = 0;
  bool received = false;

  portENTER_CRITICAL(&espNowMux);

  if (commandPending) {
    command = pendingCommand;
    sequence = pendingSequence;
    commandPending = false;
    received = true;
  }

  portEXIT_CRITICAL(&espNowMux);

  if (!received) {
    return;
  }

  Serial.println();
  Serial.println("========== ESP-NOW COMMAND ==========");

  Serial.print("Sequence: ");
  Serial.println(sequence);

  Serial.print("Command: ");
  Serial.println(command);

  int gpio = -1;
  int value = 0;
  int virtualPin = -1;

  const char *name = "";

  switch (command) {

    case 1:
      name = "LIGHT ON";
      gpio = LED_PIN;
      value = HIGH;
      virtualPin = V0;
      break;

    case 2:
      name = "LIGHT OFF";
      gpio = LED_PIN;
      value = LOW;
      virtualPin = V0;
      break;

    case 3:
      name = "FAN ON";
      gpio = FAN_PIN;
      value = HIGH;
      virtualPin = V1;
      break;

    case 4:
      name = "FAN OFF";
      gpio = FAN_PIN;
      value = LOW;
      virtualPin = V1;
      break;

    case 5:
      name = "TV ON";
      gpio = TV_PIN;
      value = HIGH;
      virtualPin = V2;
      break;

    case 6:
      name = "TV OFF";
      gpio = TV_PIN;
      value = LOW;
      virtualPin = V2;
      break;

    case 7:
      name = "MASSAGER ON";
      gpio = MASSAGER_PIN;
      value = HIGH;
      virtualPin = V3;
      break;

    case 8:
      name = "MASSAGER OFF";
      gpio = MASSAGER_PIN;
      value = LOW;
      virtualPin = V3;
      break;

    default:
      return;
  }

  // Control the appliance.
  digitalWrite(gpio, value);

  Serial.print("Executed: ");
  Serial.println(name);

  Serial.print("GPIO ");
  Serial.print(gpio);
  Serial.print(" = ");
  Serial.println(value == HIGH ? "HIGH" : "LOW");

  // Update the Blynk dashboard to reflect the
  // appliance state changed by the eye-blink system.
  if (Blynk.connected()) {
    Blynk.virtualWrite(virtualPin, value);
  }

  Serial.println("=====================================");
  Serial.println();
}

// ==================================================
// BUZZER
// ==================================================

void buzzerOutput(bool on) {
  digitalWrite(
    BUZZER_PIN,
    (on == BUZZER_ACTIVE_HIGH) ? HIGH : LOW
  );
}

void serviceBuzzer() {

  bool shouldBeep = false;
  uint32_t now = millis();

  for (int i = 0; i < 2; i++) {

    if (alarms[i].active &&
        now - alarms[i].started < ALARM_DURATION &&
        ((now - alarms[i].started) / 250) % 2 == 0) {

      shouldBeep = true;
    }
  }

  if (ACTIVE_BUZZER) {

    buzzerOutput(shouldBeep);

  } else {

    static uint32_t lastEdge = 0;
    static bool level = false;

    if (!shouldBeep) {

      level = false;
      buzzerOutput(false);

    } else if ((uint32_t)(micros() - lastEdge) >= 250) {

      lastEdge = micros();
      level = !level;
      buzzerOutput(level);
    }
  }

  if (shouldBeep != buzzerOn) {

    buzzerOn = shouldBeep;

    Serial.printf(
      "Buzzer GPIO%d: %s\n",
      BUZZER_PIN,
      shouldBeep ? "ON" : "OFF"
    );
  }
}

// ==================================================
// MEDICINE BOX ALARM FUNCTIONS
// ==================================================

void saveAlarm(int i) {

  String k = "a" + String(i);

  prefs.putInt((k + "h").c_str(), alarms[i].hour);
  prefs.putInt((k + "m").c_str(), alarms[i].minute);
  prefs.putBool((k + "e").c_str(), alarms[i].enabled);
}

void stopAlarm(int i) {

  if (!alarms[i].active) return;

  alarms[i].active = false;

  if (i == 0 && servoReady[0]) {
    servo1.write(0);
  }

  if (i == 1 && servoReady[1]) {
    servo2.writeMicroseconds(STOP_PULSE);
  }

  Serial.printf("Alarm %d stopped\n", i + 1);
}

void setAlarmHour(int i, int value) {

  if (value < 0 || value > 23) return;

  alarms[i].hour = value;
  saveAlarm(i);

  Serial.printf(
    "Alarm %d hour = %02d\n",
    i + 1,
    value
  );
}

void setAlarmMinute(int i, int value) {

  if (value < 0 || value > 59) return;

  alarms[i].minute = value;
  saveAlarm(i);

  Serial.printf(
    "Alarm %d minute = %02d\n",
    i + 1,
    value
  );
}

void setAlarmEnabled(int i, int value) {

  alarms[i].enabled = (value != 0);

  saveAlarm(i);

  if (!alarms[i].enabled) {
    stopAlarm(i);
  }

  Serial.printf(
    "Alarm %d %s at %02d:%02d\n",
    i + 1,
    alarms[i].enabled ? "ENABLED" : "DISABLED",
    alarms[i].hour,
    alarms[i].minute
  );
}

void startAlarm(int i, bool manualTest = false) {

  if (alarms[i].active) return;

  if (!servoReady[i]) {

    Serial.printf(
      "Alarm %d cannot start: servo not attached\n",
      i + 1
    );

    return;
  }

  alarms[i].active = true;
  alarms[i].started = millis();
  alarms[i].stage = 0;

  if (i == 0) {
    servo1.write(90);
  } else {
    servo2.writeMicroseconds(FORWARD_PULSE);
  }

  Serial.printf(
    "*** ALARM %d STARTED (%s) ***\n",
    i + 1,
    manualTest ? "MANUAL TEST" : "SCHEDULED"
  );
}

void serviceAlarms() {

  uint32_t now = millis();

  for (int i = 0; i < 2; i++) {

    Alarm &a = alarms[i];

    if (!a.active) continue;

    uint32_t elapsed = now - a.started;

    if (i == 1) {

      if (a.stage == 0 && elapsed >= FORWARD_TIME) {

        servo2.writeMicroseconds(STOP_PULSE);
        a.stage = 1;

        Serial.println(
          "Servo 2 STOP after forward 366 ms"
        );
      }

      if (a.stage == 1 &&
          elapsed >= SERVO2_REVERSE_START) {

        servo2.writeMicroseconds(REVERSE_PULSE);
        a.stage = 2;

        Serial.println(
          "Servo 2 REVERSE for 366 ms"
        );
      }

      if (a.stage == 2 &&
          elapsed >= SERVO2_REVERSE_START + REVERSE_TIME) {

        servo2.writeMicroseconds(STOP_PULSE);
        a.stage = 3;

        Serial.println(
          "Servo 2 STOP after reverse"
        );
      }
    }

    if (elapsed >= ALARM_DURATION) {
      stopAlarm(i);
    }
  }
}

// ==================================================
// BLYNK APPLIANCE CONTROL
// ==================================================

void appliance(
  int virtualPin,
  const char *name,
  int gpio,
  int value
) {

  digitalWrite(gpio, value ? HIGH : LOW);

  Serial.printf(
    "Blynk V%d %s = %d, GPIO%d = %d\n",
    virtualPin,
    name,
    value,
    gpio,
    digitalRead(gpio)
  );
}

BLYNK_WRITE(V0) {
  appliance(0, "LIGHT", LED_PIN, param.asInt());
}

BLYNK_WRITE(V1) {
  appliance(1, "FAN", FAN_PIN, param.asInt());
}

BLYNK_WRITE(V2) {
  appliance(2, "TV", TV_PIN, param.asInt());
}

BLYNK_WRITE(V3) {
  appliance(3, "MASSAGER", MASSAGER_PIN, param.asInt());
}

BLYNK_WRITE(V4) {
  setAlarmHour(0, param.asInt());
}

BLYNK_WRITE(V5) {
  setAlarmMinute(0, param.asInt());
}

BLYNK_WRITE(V6) {
  setAlarmEnabled(0, param.asInt());
}

BLYNK_WRITE(V7) {
  setAlarmHour(1, param.asInt());
}

BLYNK_WRITE(V8) {
  setAlarmMinute(1, param.asInt());
}

BLYNK_WRITE(V9) {
  setAlarmEnabled(1, param.asInt());
}

BLYNK_CONNECTED() {

  Serial.println(
    "Blynk connected; syncing dashboard"
  );

  Blynk.syncAll();
}

// ==================================================
// ALARM SCHEDULER
// ==================================================

void checkSchedule() {

  if (millis() - lastAlarmCheck < 200) return;

  lastAlarmCheck = millis();

  time_t utc = time(nullptr);

  if (utc < 1700000000) return;

  struct tm local;
  localtime_r(&utc, &local);

  int day =
    (local.tm_year + 1900) * 10000 +
    (local.tm_mon + 1) * 100 +
    local.tm_mday;

  static int lastPrintedSecond = -1;

  if (local.tm_sec != lastPrintedSecond) {

    lastPrintedSecond = local.tm_sec;

    Serial.printf(
      "IST %02d:%02d:%02d | A1 %02d:%02d %s | A2 %02d:%02d %s\n",

      local.tm_hour,
      local.tm_min,
      local.tm_sec,

      alarms[0].hour,
      alarms[0].minute,
      alarms[0].enabled ? "ON" : "OFF",

      alarms[1].hour,
      alarms[1].minute,
      alarms[1].enabled ? "ON" : "OFF"
    );
  }

  for (int i = 0; i < 2; i++) {

    Alarm &a = alarms[i];

    if (a.enabled &&
        !a.active &&
        a.hour == local.tm_hour &&
        a.minute == local.tm_min &&
        a.lastDay != day) {

      if (!servoReady[i]) {

        Serial.printf(
          "Alarm %d due, but servo unavailable\n",
          i + 1
        );

        continue;
      }

      a.lastDay = day;

      String key = "a" + String(i) + "d";
      prefs.putInt(key.c_str(), day);

      startAlarm(i);
    }
  }
}

// ==================================================
// SETUP
// ==================================================

void setup() {

  Serial.begin(115200);
  delay(300);

  Serial.println();
  Serial.println("==================================");
  Serial.println("SCC ESP32 RECEIVER STARTING");
  Serial.println("==================================");

  // Appliance GPIO setup.
  for (int p : {
    LED_PIN,
    FAN_PIN,
    TV_PIN,
    MASSAGER_PIN
  }) {

    pinMode(p, OUTPUT);
    digitalWrite(p, LOW);
  }

  // Buzzer setup.
  pinMode(BUZZER_PIN, OUTPUT);
  buzzerOutput(false);

  // Load saved medicine alarms.
  prefs.begin("medicine", false);

  for (int i = 0; i < 2; i++) {

    String k = "a" + String(i);

    alarms[i] = {
      prefs.getInt((k + "h").c_str(), 8),
      prefs.getInt((k + "m").c_str(), i * 10),
      prefs.getInt((k + "d").c_str(), -1),
      prefs.getBool((k + "e").c_str(), false),
      false,
      0,
      0
    };
  }

  // Servo setup.
  servo1.setPeriodHertz(50);
  servo2.setPeriodHertz(50);

  servo1.attach(SERVO1_PIN, 500, 2400);
  servo2.attach(SERVO2_PIN, 1000, 2000);

  servoReady[0] = servo1.attached();
  servoReady[1] = servo2.attached();

  if (servoReady[0]) {
    servo1.write(0);
  }

  if (servoReady[1]) {
    servo2.writeMicroseconds(STOP_PULSE);
  }

  Serial.printf(
    "Servos: GPIO25=%s GPIO26=%s\n",
    servoReady[0] ? "READY" : "FAILED",
    servoReady[1] ? "READY" : "FAILED"
  );

  // ----------------------------------------------
  // ESP-NOW INITIALIZATION
  // ----------------------------------------------

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Set channel before connecting to Wi-Fi.
  esp_err_t channelResult = esp_wifi_set_channel(
    GLASSES_WIFI_CHANNEL,
    WIFI_SECOND_CHAN_NONE
  );

  Serial.print("ESP-NOW channel setup: ");

  Serial.println(
    channelResult == ESP_OK ? "SUCCESS" : "FAILED"
  );

  // Initialize ESP-NOW.
  if (esp_now_init() == ESP_OK) {

    Serial.println("ESP-NOW initialization: SUCCESS");

    esp_err_t callbackResult =
      esp_now_register_recv_cb(onEspNowReceive);

    if (callbackResult == ESP_OK) {

      espNowReady = true;

      Serial.println(
        "ESP-NOW receive callback: REGISTERED"
      );

    } else {

      Serial.println(
        "ESP-NOW receive callback: FAILED"
      );
    }

  } else {

    Serial.println(
      "ESP-NOW initialization: FAILED"
    );
  }

  Serial.print("ESP-NOW Receiver Ready: ");

  Serial.println(
    espNowReady ? "YES" : "NO"
  );

  // ----------------------------------------------
  // TIME AND WIFI
  // ----------------------------------------------

  configTzTime(
    TIME_ZONE,
    "pool.ntp.org",
    "time.google.com"
  );

  WiFi.begin(ssid, pass);

  // ----------------------------------------------
  // BLYNK
  // ----------------------------------------------

  Blynk.config(BLYNK_AUTH_TOKEN);

  Serial.println();
  Serial.println("Serial medicine-box tests:");
  Serial.println("Type 1 = Alarm 1");
  Serial.println("Type 2 = Alarm 2");
  Serial.println("Type S = Stop both alarms");

  Serial.println();
  Serial.println("SCC ESP32 RECEIVER READY");
}

// ==================================================
// MAIN LOOP
// ==================================================

void loop() {

  // Process incoming ESP-NOW commands.
  processEspNowCommand();

  // Serial medicine-box tests.
  while (Serial.available()) {

    char c = Serial.read();

    if (c == '1') {
      startAlarm(0, true);
    }

    if (c == '2') {
      startAlarm(1, true);
    }

    if (c == 's' || c == 'S') {

      stopAlarm(0);
      stopAlarm(1);
      buzzerOutput(false);
    }
  }

  // Medicine-box functions.
  serviceAlarms();
  serviceBuzzer();
  checkSchedule();

  uint32_t now = millis();

  // Wi-Fi reconnection.
  if (WiFi.status() != WL_CONNECTED) {

    if (now - lastWifiTry >= 10000) {

      lastWifiTry = now;

      Serial.println("Wi-Fi reconnecting...");

      WiFi.reconnect();
    }

  } else if (Blynk.connected()) {

    Blynk.run();

  } else if (now - lastBlynkTry >= 3000) {

    lastBlynkTry = now;

    Serial.println("Blynk connecting...");

    Blynk.connect(100);
  }

  // Periodic status.
  if (now - lastStatus >= 10000) {

    lastStatus = now;

    Serial.printf(
      "Status: Wi-Fi=%s Blynk=%s ESP-NOW=%s Time=%s\n",

      WiFi.status() == WL_CONNECTED ? "OK" : "OFF",

      Blynk.connected() ? "OK" : "OFF",

      espNowReady ? "READY" : "OFF",

      time(nullptr) >= 1700000000 ? "OK" : "WAITING"
    );
  }
}
