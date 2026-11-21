
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#define IR 0
#define BUZZER 3

// ESP32 RECEIVER MAC ADDRESS
uint8_t RECEIVER_MAC[] = {
  0xxx, 0xxx, 0xxx,
  0xxx, 0xxx, 0xxx
};

// Must match receiver's actual Wi-Fi channel
#define GLASSES_WIFI_CHANNEL 1

// ==================================================
// EYE DETECTION SETTINGS
// ==================================================

#define EYE_CLOSED_THRESHOLD 400
#define HOLD_TIME 3000
#define BLINK_MIN_TIME 100
#define BLINK_MAX_TIME 800
#define BLINK_WAIT_TIME 2000

// LOW = OPEN
// HIGH = CLOSED

bool commandMode = false;

// Initial hold detection
bool holdArmed = false;
bool holdTiming = false;

unsigned long eyeCloseStart = 0;

// Blink detection (separate from hold detection)
bool blinkEyeClosed = false;
unsigned long blinkStart = 0;
unsigned long lastBlinkTime = 0;

int blinkCount = 0;

// Ignore the long hold that activated command mode
bool waitForEyeOpen = false;

// For eye-state monitoring
int lastEyeState = -1;

// ==================================================
// ESP-NOW PACKET
// ==================================================

struct BlinkPacket {
  uint32_t magic;
  uint32_t sequence;
  uint8_t command;
} __attribute__((packed));

const uint32_t BLINK_MAGIC = 0xB11A2026;

uint32_t sequenceNumber = 0;
bool espNowReady = false;

// ==================================================
// BUZZER
// ==================================================

void beep(int t) {

  digitalWrite(BUZZER, HIGH);
  delay(t);
  digitalWrite(BUZZER, LOW);
}

// ==================================================
// ESP-NOW TRANSMISSION
// ==================================================

void sendBlinkCommand(uint8_t command) {

  if (!espNowReady) {
    Serial.println("ESP-NOW NOT READY");
    return;
  }

  BlinkPacket packet = {
    BLINK_MAGIC,
    ++sequenceNumber,
    command
  };

  // Three attempts using the same sequence number
  for (int attempt = 0; attempt < 3; attempt++) {

    esp_err_t result = esp_now_send(
      RECEIVER_MAC,
      (uint8_t*)&packet,
      sizeof(packet)
    );

    Serial.print("Sending command: ");
    Serial.print(command);

    Serial.print(" Attempt: ");
    Serial.print(attempt + 1);

    Serial.print(" Queued: ");

    Serial.println(
      result == ESP_OK ? "YES" : "NO"
    );

    delay(35);
  }
}

// ==================================================
// ENTER COMMAND MODE
// ==================================================

void enterCommandMode() {

  commandMode = true;

  blinkCount = 0;
  blinkEyeClosed = false;

  lastBlinkTime = millis();

  // Disarm the next hold until eyes are open again.
  holdArmed = false;
  holdTiming = false;

  // Ignore the release of the 3-second hold.
  waitForEyeOpen = true;

  Serial.println();
  Serial.println("================================");
  Serial.println(">>> COMMAND MODE ENTERED");
  Serial.println("================================");

  beep(500);
}

// ==================================================
// EXIT COMMAND MODE
// ==================================================

void exitCommandMode() {

  commandMode = false;

  blinkCount = 0;
  blinkEyeClosed = false;

  waitForEyeOpen = false;

  // A fresh open-eye detection is required
  // before another 3-second hold can begin.
  holdArmed = false;
  holdTiming = false;

  Serial.println();
  Serial.println("================================");
  Serial.println("<<< COMMAND MODE EXITED");
  Serial.println("================================");

  delay(500);
}

// ==================================================
// EXECUTE BLINK COMMAND
// ==================================================

void executeBlinkCommand() {

  Serial.println();
  Serial.print("Executing command for ");
  Serial.print(blinkCount);
  Serial.println(" blinks");

  switch (blinkCount) {

    case 1:
      Serial.println("LIGHT ON");
      break;

    case 2:
      Serial.println("LIGHT OFF");
      break;

    case 3:
      Serial.println("FAN ON");
      break;

    case 4:
      Serial.println("FAN OFF");
      break;

    case 5:
      Serial.println("TV ON");
      break;

    case 6:
      Serial.println("TV OFF");
      break;

    case 7:
      Serial.println("MASSAGER ON");
      break;

    case 8:
      Serial.println("MASSAGER OFF");
      break;

    default:
      Serial.println("INVALID COMMAND");

      beep(150);
      beep(150);
      break;
  }

  // Send valid command wirelessly.
  if (blinkCount >= 1 && blinkCount <= 8) {

    sendBlinkCommand(blinkCount);
  }

  beep(300);

  exitCommandMode();
}

// ==================================================
// SETUP
// ==================================================

void setup() {

  pinMode(IR, INPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(BUZZER, LOW);

  Serial.begin(9600);

  Serial.println();
  Serial.println("=== Eye Controlled System Started ===");

  Serial.println("Waiting for eyes to be detected OPEN.");

  beep(200);

  WiFi.mode(WIFI_STA);

  WiFi.setSleep(false);

  esp_wifi_set_channel(
    GLASSES_WIFI_CHANNEL,
    WIFI_SECOND_CHAN_NONE
  );

  if (esp_now_init() == ESP_OK) {

    esp_now_peer_info_t peer = {};

    memcpy(
      peer.peer_addr,
      RECEIVER_MAC,
      6
    );

    peer.channel = GLASSES_WIFI_CHANNEL;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;

    espNowReady =
      (esp_now_add_peer(&peer) == ESP_OK);
  }

  Serial.print("Glasses MAC: ");
  Serial.println(WiFi.macAddress());

  Serial.print("ESP-NOW Ready: ");

  Serial.println(
    espNowReady ? "YES" : "NO"
  );
}

// ==================================================
// MAIN LOOP
// ==================================================

void loop() {

  unsigned long now = millis();

  int sensorValue = analogRead(IR);

  int eyeState =
    (sensorValue < EYE_CLOSED_THRESHOLD) ? HIGH : LOW;

  // HIGH = CLOSED
  // LOW = OPEN

  // ----------------------------------------------
  // EYE STATE MONITOR
  // ----------------------------------------------

  if (eyeState != lastEyeState) {

    if (eyeState == HIGH) {
      Serial.println("Eye CLOSED");
    } else {
      Serial.println("Eye OPENED");
    }

    lastEyeState = eyeState;
  }

  // ==================================================
  // NORMAL MODE: DETECT 3-SECOND EYE CLOSURE
  // ==================================================

  if (!commandMode) {

    // First, eyes must be detected OPEN.
    // This prevents automatic activation at startup
    // when the sensor initially reports CLOSED.

    if (!holdArmed) {

      if (eyeState == LOW) {

        holdArmed = true;
        holdTiming = false;

        Serial.println(
          "System armed. Close eyes for 3 seconds."
        );
      }

      return;
    }

    // Start timing only when eyes actually close.
    if (eyeState == HIGH) {

      if (!holdTiming) {

        eyeCloseStart = now;
        holdTiming = true;

        Serial.println(
          "3-second eye closure started."
        );
      }

      // Enter command mode only if eyes remain
      // continuously CLOSED for at least 3 seconds.

      if (now - eyeCloseStart >= HOLD_TIME) {

        enterCommandMode();

        return;
      }

    } else {

      // Eyes opened before 3 seconds:
      // cancel the hold timer.

      if (holdTiming) {

        Serial.println(
          "Hold cancelled. Eyes opened too early."
        );
      }

      holdTiming = false;
    }

    return;
  }

  // ==================================================
  // COMMAND MODE
  // ==================================================

  if (commandMode) {

    // Ignore the release of the original
    // 3-second closure.

    if (waitForEyeOpen) {

      if (eyeState == LOW) {

        waitForEyeOpen = false;

        blinkEyeClosed = false;

        Serial.println(
          "Ready to count blinks."
        );
      }

      return;
    }

    // ----------------------------------------------
    // BLINK DETECTION
    // ----------------------------------------------

    if (eyeState == HIGH && !blinkEyeClosed) {

      blinkStart = now;
      blinkEyeClosed = true;
    }

    if (eyeState == LOW && blinkEyeClosed) {

      unsigned long blinkDuration =
        now - blinkStart;

      blinkEyeClosed = false;

      Serial.print("Blink Duration: ");
      Serial.print(blinkDuration);
      Serial.println(" ms");

      if (blinkDuration >= BLINK_MIN_TIME &&
          blinkDuration <= BLINK_MAX_TIME) {

        blinkCount++;

        lastBlinkTime = now;

        Serial.print("Blink COUNTED: ");
        Serial.println(blinkCount);

        beep(80);

      } else {

        Serial.println("Blink ignored");
      }
    }

    // ----------------------------------------------
    // EXECUTE COMMAND AFTER 2 SECONDS
    // ----------------------------------------------

    if (blinkCount > 0 &&
        (now - lastBlinkTime >= BLINK_WAIT_TIME)) {

      executeBlinkCommand();

      return;
    }
  }
}
