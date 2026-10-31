int led = 3;
int lcd = 4;
int fan = 6;         // Pin connected to relay controlling the fan
int massager = 5;
unsigned int receivedValue = 0;

void setup() {
  pinMode(led, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(massager, OUTPUT);
  pinMode(lcd, OUTPUT);
  Serial.begin(9600);
  digitalWrite(massager, LOW);
  digitalWrite(fan, LOW);
}

void loop() {
  if (Serial.available()) {
    Serial.println("Data From VC-02");
    // Read the incoming bytes
    byte highByte = Serial.read();
    byte lowByte = Serial.read();

    // Combine the two bytes into a single 16-bit value
    receivedValue = (highByte << 8) | lowByte;

    // Print the received value in HEX format
    Serial.print("Received HEX value: 0x");
    Serial.println(receivedValue, HEX);
  }

  // Control LCD
  if (receivedValue == 0xC190) {  // Command to turn ON the LCD
    digitalWrite(lcd, HIGH);
  } else if (receivedValue == 0xC090) {  // Command to turn OFF the LCD
    digitalWrite(lcd, LOW);
  }

  // Control LED
  if (receivedValue == 0xA190) {  // Command to turn ON the LED
    digitalWrite(led, HIGH);
  } else if (receivedValue == 0xA090) {  // Command to turn OFF the LED
    digitalWrite(led, LOW);
  }

  // Control Fan
  if (receivedValue == 0xB190) {  // Command to turn ON the fan
    digitalWrite(fan, HIGH);      // Activate the relay to supply power to the fan
  } else if (receivedValue == 0xB090) {  // Command to turn OFF the fan
    digitalWrite(fan, LOW);       // Deactivate the relay
  }

  // Control Massager
  if (receivedValue == 0xD190) {  // Command to turn ON the massager
    digitalWrite(massager, HIGH);
  } else if (receivedValue == 0xD090) {  // Command to turn OFF the massager
    digitalWrite(massager, LOW);
  }

  delay(10);
  receivedValue = 0;  // Reset receivedValue after processing
}
