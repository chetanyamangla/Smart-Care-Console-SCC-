
#include <Arduino.h>
#include <WiFi.h>
#include <esp_mac.h>

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("==============================");
  Serial.println("SCC ESP32 MAC ADDRESS FINDER");
  Serial.println("==============================");

  // Set ESP32 to Wi-Fi station mode
  WiFi.mode(WIFI_STA);
  delay(500);

  // Read the station MAC address
  uint8_t mac[6];

  esp_err_t result = esp_read_mac(mac, ESP_MAC_WIFI_STA);

  if (result == ESP_OK) {
    Serial.println("Receiver MAC Address:");

    Serial.printf(
      "%02X:%02X:%02X:%02X:%02X:%02X\n",
      mac[0],
      mac[1],
      mac[2],
      mac[3],
      mac[4],
      mac[5]
    );

    Serial.println("==============================");
    Serial.println("MAC ADDRESS READ SUCCESSFULLY");
  } else {
    Serial.println("ERROR: MAC address reading failed");
  }
}

void loop() {
  // Nothing required here
}
