
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  Serial.println();
  Serial.println("ESP32 Receiver MAC Address:");
  Serial.println();

  uint8_t mac[6];
  WiFi.macAddress(mac);

  Serial.println("uint8_t RECEIVER_MAC[] = {");
  Serial.printf("  0x%02X, 0x%02X, 0x%02X,\n",
                mac[0], mac[1], mac[2]);
  Serial.printf("  0x%02X, 0x%02X, 0x%02X\n",
                mac[3], mac[4], mac[5]);
  Serial.println("};");
}

void loop() {
}
