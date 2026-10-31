#include <Wire.h>
#include <RTClib.h>
#include <Servo.h>

RTC_DS3231 rtc;
Servo servo1;
Servo servo2;

const int buzzerPin = 9;  // Pin for the buzzer
const int servo1Pin = 6;  // Pin for servo 1
const int servo2Pin = 7;  // Pin for servo 2

// Uncomment the next line to force time update every upload
#define FORCE_TIME_UPDATE

void setup() {
  Serial.begin(9600);
  

  pinMode(buzzerPin, OUTPUT);
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  servo1.write(0);
  servo2.write(0);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

#ifdef FORCE_TIME_UPDATE
  Serial.println("Forcing RTC time update to computer's upload time.");
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
#else
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
#endif
}

void loop() {
  DateTime now = rtc.now();

  // Print the current time to the Serial Monitor
  Serial.print("Current Time: ");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  // Check if it's 8:00 AM
  if (now.hour() == 21 && now.minute() == 45 && now.second() == 0) {
    Serial.println("Action at 8:00 AM: Activating buzzer and rotating servo 1.");
    triggerBuzzerAndServo(servo1, "Servo 1");
  }

  // Check if it's 8:10 AM
  if (now.hour() == 21 && now.minute() ==  46 && now.second() == 00) {
    Serial.println("Action at 8:10 AM: Activating buzzer and rotating servo 2.");
    triggerBuzzerAndServo(servo2, "Servo 2");
  }

  delay(1000); // Check every second
}

void triggerBuzzerAndServo(Servo &servo, const char* servoName) {
  Serial.println("Buzzer ON for 5 seconds with 0.5 sec interval.");
  
  // Activate buzzer for 5 seconds with a 0.5-sec interval
  servo.write(90);
  Serial.print(servoName);
  Serial.println(" rotated to 180 degrees.");

  for (int i = 0; i < 10; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(250);
    digitalWrite(buzzerPin, LOW);
    delay(250);
  }

  // Rotate the servo to 180 degrees
  servo.write(0);  // Reset servo position to 0 degrees
  Serial.print(servoName);
  Serial.println(" reset to 0 degrees.");
  Serial.println("All alert systems are now turned off.");
}
