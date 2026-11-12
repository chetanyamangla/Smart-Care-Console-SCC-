
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

// Set to the receiver's actual Wi-Fi channel
#define GLASSES_WIFI_CHANNEL 1

// Original blink variables
bool commandMode = false;
bool eyeWasClosed = false;

unsigned long eyeCloseStart = 0;
unsigned long blinkStart = 0;
unsigned long lastBlinkTime = 0;

int blinkCount = 0;
int lastEyeState = LOW;

// LOW = OPEN, HIGH = CLOSED

// Ignore the release of the 3-second hold
bool waitForEyeOpen = false;

// ESP-NOW packet (matches receiver)
struct BlinkPacket {
  uint32_t magic;
  uint32_t sequence;
  uint8_t command;
} __attribute__((packed));

const uint32_t BLINK_MAGIC = 0xB11A2026;

uint32_t sequenceNumber = 0;
bool espNowReady = false;

// Original buzzer function
void beep(int t) {
  digitalWrite(BUZZER, HIGH);
  delay(t);
  digitalWrite(BUZZER, LOW);
}

/* ---------- ESP-NOW TRANSMISSION ---------- */

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

  // Three attempts, same sequence number
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

/* ---------- SETUP ---------- */

void setup() {

  pinMode(IR, INPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(BUZZER, LOW);

  Serial.begin(9600);

  Serial.println(
    "=== Eye Controlled System Started ==="
  );

  beep(200);

  WiFi.mode(WIFI_STA);

  // Prioritize accurate ESP-NOW communication
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

/* ---------- MAIN LOOP ---------- */

void loop() {

  int sensorValue = analogRead(IR);

  int eyeState =
    (sensorValue < 400) ? HIGH : LOW;

  // HIGH = CLOSED
  // LOW = OPEN

  unsigned long now = millis();

  /* ---------- EYE STATE MONITOR ---------- */

  if (eyeState != lastEyeState) {

    if (eyeState == HIGH) {
      Serial.println("Eye CLOSED");
    } else {
      Serial.println("Eye OPENED");
    }

    lastEyeState = eyeState;
  }

  /* ---------- COMMAND MODE ENTRY ---------- */

  if (!commandMode) {

    if (eyeState == HIGH) {

      if (!eyeWasClosed) {

        eyeCloseStart = millis();
        eyeWasClosed = true;

        Serial.println(
          "3-second hold started"
        );
      }

      // Activate WHILE eye is still closed
      if (millis() - eyeCloseStart >= 3000) {

        commandMode = true;
        blinkCount = 0;
        lastBlinkTime = millis();

        Serial.println(
          ">>> COMMAND MODE ENTERED"
        );

        // Immediate 500 ms confirmation beep
        beep(500);

        // Do not count the long hold as a blink
        waitForEyeOpen = true;

        return;
      }

    } else {

      eyeWasClosed = false;
    }

    return;
  }

  /* ---------- BLINK DETECTION ---------- */

  if (commandMode) {

    // Command mode is already active.
    // Wait only to ignore the original
    // long eye closure.

    if (waitForEyeOpen) {

      if (eyeState == LOW) {

        waitForEyeOpen = false;
        eyeWasClosed = false;

        Serial.println(
          "Ready to count blinks"
        );
      }

      return;
    }

    if (eyeState == HIGH && !eyeWasClosed) {

      blinkStart = now;
      eyeWasClosed = true;
    }

    if (eyeState == LOW && eyeWasClosed) {

      unsigned long blinkDuration =
        now - blinkStart;

      eyeWasClosed = false;

      Serial.print("Blink Duration: ");
      Serial.print(blinkDuration);
      Serial.println(" ms");

      if (blinkDuration >= 100 &&
          blinkDuration <= 800) {

        blinkCount++;
        lastBlinkTime = now;

        Serial.print("Blink COUNTED: ");
        Serial.println(blinkCount);

        beep(80);

      } else {

        Serial.println("Blink ignored");
      }
    }

    /* ---------- COMMAND EXECUTION ---------- */

    if (blinkCount > 0 &&
        (now - lastBlinkTime > 2000)) {

      Serial.print("Executing command for ");
      Serial.print(blinkCount);
      Serial.println(" blinks");

      if (blinkCount == 1) {

        Serial.println("LIGHT ON");

      }
      else if (blinkCount == 2) {

        Serial.println("LIGHT OFF");

      }
      else if (blinkCount == 3) {

        Serial.println("FAN ON");

      }
      else if (blinkCount == 4) {

        Serial.println("FAN OFF");

      }
      else if (blinkCount == 5) {

        Serial.println("TV ON");

      }
      else if (blinkCount == 6) {

        Serial.println("TV OFF");

      }
      else if (blinkCount == 7) {

        Serial.println("MASSAGER ON");

      }
      else if (blinkCount == 8) {

        Serial.println("MASSAGER OFF");

      }
      else {

        Serial.println("INVALID COMMAND");

        beep(150);
        beep(150);
      }

      // Send valid command wirelessly
      if (blinkCount >= 1 &&
          blinkCount <= 8) {

        sendBlinkCommand(blinkCount);
      }

      beep(300);

      blinkCount = 0;
      commandMode = false;
      eyeWasClosed = false;
      waitForEyeOpen = false;

      Serial.println(
        "<<< COMMAND MODE EXITED"
      );

      delay(500);
    }
  }
}
