# Smart Care Console (SCC) – Setup and Recreation Guide

This guide explains how to set up, configure and recreate the Smart Care Console (SCC), including its voice-controlled assistant, eye-blink control system and automatic medicine box.

## 1. Required Hardware

Refer to `BOM.csv` for the complete component list, quantities, specifications and purchase links.

The project consists of two separate controller setups.

### A. Voice and Eye-Blink Controlled Assistant

- 1 ESP32 development board
- 1 Arduino UNO
- 1 VC-02 voice-recognition module
- 1 IR eye sensor
- Relay module(s)
- Buzzer
- LED
- LCD / TV
- Fan
- Massager
- Required power supplies and connecting wires

### B. Automatic Medicine Box

- 1 separate Arduino UNO
- 1 DS3231 RTC module
- 2 servo motors
- 1 buzzer
- Required power supply and connecting wires

**Important:** The medicine box and voice assistant use separate Arduino UNO boards. Do not combine their firmware into one Arduino.

## 2. Hardware Connections

1. Open the `hardware` folder in this repository.
2. Refer to the voice assistant circuit diagram for ESP32, Arduino UNO, VC-02, IR sensor and appliance connections.
3. Refer to the medicine box circuit diagram for Arduino UNO, DS3231 RTC, servo motors and buzzer connections.
4. Follow the diagrams carefully while making the connections.

## 3. Train and Configure the VC-02 Voice Recognition Module

Before using the voice assistant, the VC-02 module must be configured with the required voice commands and their corresponding output codes.

**Follow this tutorial for VC-02 configuration and firmware uploading:**

[Watch the VC-02 Configuration Tutorial on YouTube](https://youtu.be/dAqX4CmozfM?si=FC2gHPxsgGURy6of)

Follow the tutorial to:

1. Understand the VC-02 module and its configuration process.
2. Create and configure your voice-recognition project.
3. Add the required voice commands.
4. Generate the customized firmware.
5. Download and upload the generated firmware to the VC-02 module using the appropriate upload tool.

### Required Voice Command Codes

Configure the voice commands so that the VC-02 module sends the following codes, which are expected by the Smart Care Console Arduino firmware.

| Voice Command | Expected Code |
|---|---|
| LED ON | `0xA190` |
| LED OFF | `0xA090` |
| LCD / TV ON | `0xC190` |
| LCD / TV OFF | `0xC090` |
| Fan ON | `0xB190` |
| Fan OFF | `0xB090` |
| Massager ON | `0xD190` |
| Massager OFF | `0xD090` |

**Important:** The command codes configured in the VC-02 firmware must exactly match the codes listed above. Otherwise, the Arduino will not recognize the commands correctly.

After configuring and uploading the VC-02 firmware, connect the module to the Arduino UNO according to the circuit diagram and upload the Smart Care Console Arduino firmware.

## 4. Configure the Blynk IoT Application

The ESP32 uses the Blynk IoT platform for remote appliance control.

1. Create an account on the Blynk IoT platform.
2. Create a new template for the ESP32.
3. Configure the required device and connection settings.
4. Create four datastreams:
   - `V0` – LED
   - `V1` – Fan
   - `V2` – LCD / TV
   - `V3` – Massager
5. Add four control widgets and connect each widget to its corresponding datastream.
6. Obtain your own Template ID, Template Name and Authentication Token.
7. Enter these details and your own Wi-Fi credentials in the ESP32 firmware.
8. Upload the ESP32 firmware and test the controls through the Blynk application.

**Important:** Each builder must use their own Blynk credentials. Never publish your actual authentication token or Wi-Fi password in the repository.

## 5. Upload the Firmware

Open the `firmware` folder and select the code corresponding to the controller you want to program.

### A. Voice and Eye-Blink Controlled Assistant

**Arduino UNO:**

1. Connect the voice assistant Arduino UNO to your computer.
2. Open the Arduino IDE.
3. Open the voice assistant Arduino code.
4. Install the required libraries, if any.
5. Select the Arduino UNO board and correct COM port.
6. Upload the firmware.

**ESP32:**

1. Connect the ESP32 to your computer.
2. Open the ESP32 firmware.
3. Select the correct ESP32 board and COM port.
4. Enter your own Blynk Template ID, Template Name, Authentication Token and Wi-Fi credentials.
5. Upload the firmware.

### B. Automatic Medicine Box

1. Connect the separate medicine box Arduino UNO to your computer.
2. Open the medicine box firmware.
3. Install the required libraries, including RTClib and Servo.
4. Select the Arduino UNO board and correct COM port.
5. Upload the firmware.
6. Set the RTC time correctly and verify the scheduled servo and buzzer actions.

## 6. Testing

After completing the hardware connections and firmware uploads, test the following features:

### Voice Assistant
- Test each trained voice command.
- Verify that all four appliances respond correctly.
- Test the Blynk application controls.

### Eye-Blink Control
- Test eye detection using the IR sensor.
- Verify that the system enters command mode after the programmed eye-closure duration.
- Test the blink-based appliance commands.

### Medicine Box
- Verify the DS3231 RTC time.
- Check the programmed activation times.
- Test both servo motors and the buzzer.

Make sure each feature operates as expected before considering the recreation complete.

## 7. Troubleshooting

If a feature does not work:

- Recheck the circuit connections.
- Verify that the correct firmware was uploaded to each controller.
- Check that the VC-02 command codes match the Arduino firmware.
- Verify the Blynk datastream configuration and credentials.
- Check the RTC time and programmed activation settings.
- Check the power supply and connections of the components.
- Refer to the circuit diagrams and component documentation.

## 8. Project Files

The repository contains the following resources:

- `hardware/` – Circuit diagrams and hardware connection details.
- `firmware/` – ESP32, Arduino UNO and medicine box firmware.
- `documentation/` – Setup instructions and additional guides.
- `images/` – Photographs of the physical prototype (to be added).

Refer to these resources throughout the recreation process.
