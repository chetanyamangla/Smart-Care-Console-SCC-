# Smart Care Console (SCC) – Setup and Recreation Guide

This guide explains how to set up and recreate the Smart Care Console (SCC), including its voice-controlled appliance system, Blynk IoT application, wireless eye-blink control system and automatic medicine box.

The project uses three main firmware components:

1. **Arduino UNO – Voice Control:** Receives commands from the VC-02 voice-recognition module and controls household appliances.
2. **ESP32 – Receiver and Medicine Box Controller:** Receives appliance commands from the Blynk IoT application and the wireless eye-blink transmitter. It also controls both medicine box servo motors and the buzzer.
3. **ESP32-C3 – Wireless Eye-Blink Transmitter:** Detects eye blinks using an IR sensor and sends appliance commands wirelessly to the ESP32 receiver using ESP-NOW.

You can either use the firmware provided in the repository or develop and configure your own firmware by following the instructions below.

---

## 1. Required Hardware


### A. Voice-Controlled Appliance System

- 1 Arduino UNO
- 1 VC-02 voice-recognition module
- Relay module(s)
- LED / light
- LCD / TV
- Fan
- Massager
- Required power supplies and connecting wires

### B. ESP32 Receiver and Medicine Box

- 1 ESP32 development board
- 2 servo motors
- 1 buzzer
- Relay module(s)
- Medicine box mechanism
- Required power supplies and connecting wires

### C. Wireless Eye-Blink Transmitter

- 1 ESP32-C3 development board
- 1 IR eye sensor
- 1 buzzer
- Required power supply and connecting wires

**Important:** The Arduino UNO, ESP32 receiver and ESP32-C3 transmitter are separate controllers. Each must be programmed with its corresponding firmware.

## 2. Hardware Connections

All circuit diagrams and pin connection details are available in the `hardware` folder.

1. Open the `hardware` folder.
2. Refer to the voice-control circuit diagram for the Arduino UNO, VC-02 module and appliance connections.
3. Refer to the ESP32 receiver circuit diagram for the relay modules, two servo motors and buzzer.
4. Refer to the eye-blink transmitter circuit diagram for the ESP32-C3, IR sensor and buzzer.
5. Follow the respective diagrams when connecting the components.

Make sure the power supply requirements and common ground connections are followed.

## 3. Configure the VC-02 Voice Recognition Module

The VC-02 module must be configured with voice commands and their corresponding output codes before it can operate the appliances.

There are two ways to configure it.

### Option 1: Use the Provided VC-02 Firmware

The repository contains the VC-02 firmware files in the `firmware` folder.

1. Open the `firmware` folder.
2. Locate the provided VC-02 firmware and its associated upload files.
3. Follow the VC-02 configuration tutorial to understand the firmware uploading process.
4. Upload the provided firmware to the VC-02 module using the appropriate upload tool.
5. Connect the VC-02 module to the Arduino UNO according to the circuit diagram.

### Option 2: Build Your Own VC-02 Firmware

You can create your own voice-recognition configuration with customised commands.

**Follow the original tutorial for VC-02 configuration and firmware uploading:**

[Watch the VC-02 Configuration Tutorial on YouTube](https://youtu.be/dAqX4CmozfM?si=FC2gHPxsgGURy6of)

Follow the tutorial to:

1. Understand the VC-02 module and its configuration process.
2. Create a new voice-recognition project.
3. Add and configure the required voice commands.
4. Assign the appropriate output codes to each command.
5. Generate the customised firmware.
6. Download and upload the generated firmware to the VC-02 module using the appropriate upload tool.

### Required Voice Command Codes

Whether you use the provided firmware or create your own, the output codes must match the following values expected by the Arduino UNO firmware.

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

**Important:** If you create your own VC-02 firmware, make sure the output codes exactly match the table. Otherwise, the Arduino UNO will not recognise the commands correctly.

## 4. Upload the Arduino UNO Voice-Control Firmware

The Arduino UNO receives voice commands from the VC-02 module and controls the connected appliances.

1. Open the `firmware` folder.
2. Locate the Arduino UNO voice-control code.
3. Connect the Arduino UNO to your computer.
4. Open the code in the Arduino IDE.
5. Select the Arduino UNO board and the correct COM port.
6. Upload the firmware.
7. Connect the VC-02 module according to the voice-control circuit diagram.

The VC-02 sends the configured command codes to the Arduino UNO through serial communication. The Arduino processes the received commands and switches the corresponding appliance outputs.

**Note:** The Arduino code uses the hardware serial RX pin (D0). Disconnect the VC-02 TX connection temporarily if it interferes with uploading the Arduino firmware.

## 5. Configure the ESP32 Receiver

The ESP32 receiver handles appliance control through the Blynk IoT application and wireless commands received from the ESP32-C3 eye-blink transmitter. It also controls the two servo motors in the automatic medicine box.

### Step 1: Find the Receiver ESP32 MAC Address

Each ESP32 board has its own MAC address. The ESP32-C3 transmitter needs the receiver's MAC address to send commands to the correct board.

1. Open the `firmware` folder.
2. Locate the ESP32 MAC address test code.
3. Connect the ESP32 receiver board to your computer.
4. Open the test code in the Arduino IDE.
5. Select the correct ESP32 board and COM port.
6. Upload the MAC address test code to the receiver.
7. Open the Serial Monitor at `115200` baud.
8. Find the receiver's station MAC address displayed in the Serial Monitor.
9. Copy the complete MAC address and save it for the next step.

**Important:** Upload this test code to the ESP32 receiver, not the ESP32-C3 transmitter.

### Step 2: Configure and Upload the ESP32 Receiver Firmware

After obtaining the receiver MAC address:

1. Open the ESP32 receiver firmware from the `firmware` folder.
2. Enter your own Wi-Fi network name and password.
3. Enter your Blynk Template ID, Template Name and Authentication Token.
4. Check that the receiver's ESP-NOW channel matches the transmitter's channel.
5. Review the firmware settings and hardware connections.
6. Select the correct ESP32 board and COM port.
7. Upload the receiver firmware.
8. Open the Serial Monitor at `115200` baud to check the Wi-Fi connection, Blynk connection, time synchronisation and servo status.

The receiver uses Wi-Fi for Blynk communication and ESP-NOW for wireless eye-blink commands.

## 6. Build the Custom Blynk IoT Application

The ESP32 receiver uses Blynk IoT to provide smartphone-based appliance control and medicine box operation.

### Step 1: Create a Blynk Account and Template

1. Visit [Blynk IoT](https://blynk.cloud/).
2. Create an account or sign in.
3. Create a new template.
4. Select ESP32 as the hardware and Wi-Fi as the connection type.
5. Give your template a suitable name.
6. Create a device using the template.
7. Copy the Template ID, Template Name and Authentication Token.

### Step 2: Create the Datastreams

Create the following Virtual Pin datastreams in your Blynk template.

**Appliance Control Datastreams**

| Virtual Pin | Datastream Name | Data Type | Purpose |
|---|---|---|---|
| V0 | Light | Integer | Controls the LED / light |
| V1 | Fan | Integer | Controls the fan |
| V2 | TV | Integer | Controls the LCD / TV |
| V3 | Massager | Integer | Controls the massager |

**Medicine Box Alarm 1 Datastreams**

| Virtual Pin | Datastream Name | Data Type | Purpose |
|---|---|---|---|
| V4 | Alarm 1 Hour | Integer | Sets the first alarm hour |
| V5 | Alarm 1 Minute | Integer | Sets the first alarm minute |
| V6 | Alarm 1 Enable | Integer | Enables or disables Alarm 1 |

**Medicine Box Alarm 2 Datastreams**

| Virtual Pin | Datastream Name | Data Type | Purpose |
|---|---|---|---|
| V7 | Alarm 2 Hour | Integer | Sets the second alarm hour |
| V8 | Alarm 2 Minute | Integer | Sets the second alarm minute |
| V9 | Alarm 2 Enable | Integer | Enables or disables Alarm 2 |

Configure suitable integer ranges:

- Hour: 0–23
- Minute: 0–59
- Enable: 0–1

### Step 3: Create the Blynk Dashboard

Add the following widgets to your Blynk mobile application dashboard.

**Appliance Controls**
- Light switch connected to V0.
- Fan switch connected to V1.
- TV / LCD switch connected to V2.
- Massager switch connected to V3.

**Medicine Box Controls**
- Hour input for Alarm 1 connected to V4.
- Minute input for Alarm 1 connected to V5.
- Enable switch for Alarm 1 connected to V6.
- Hour input for Alarm 2 connected to V7.
- Minute input for Alarm 2 connected to V8.
- Enable switch for Alarm 2 connected to V9.

Use suitable numeric input widgets for the hour and minute values and switches for enabling or disabling the alarms.

### Step 4: Connect the Application to the ESP32

1. Copy your Blynk Template ID, Template Name and Authentication Token into the ESP32 receiver firmware.
2. Enter your own Wi-Fi credentials.
3. Upload the updated firmware.
4. Open the Blynk mobile application.
5. Check that the ESP32 device connects successfully.
6. Test all four appliance controls.
7. Configure both medicine box alarms.
8. Test the two servo motors and their corresponding alarm operations.

**Important:** Each builder must use their own Blynk credentials and Wi-Fi details. Never publish actual authentication tokens or Wi-Fi passwords in a public repository.

## 7. Configure the ESP32-C3 Wireless Eye-Blink Transmitter

The ESP32-C3 transmitter detects eye blinks through the IR sensor and sends the corresponding appliance commands wirelessly to the ESP32 receiver.

### Step 1: Enter the Receiver MAC Address

1. Open the ESP32-C3 transmitter code in the `firmware` folder.
2. Locate the `RECEIVER_MAC` array.
3. Replace the existing example MAC address with the MAC address obtained from the ESP32 receiver.

Use the following format:

`uint8_t RECEIVER_MAC[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};`

Make sure all six hexadecimal values are entered correctly.

### Step 2: Check the Wireless Channel

The transmitter and receiver must use the same Wi-Fi channel for ESP-NOW communication.

The provided transmitter code uses Wi-Fi channel 1. Make sure the receiver is configured consistently.

### Step 3: Upload the Transmitter Firmware

1. Connect the ESP32-C3 board to your computer.
2. Open the ESP32-C3 transmitter firmware.
3. Enter the receiver MAC address.
4. Check the Wi-Fi channel configuration.
5. Select the correct ESP32-C3 board and COM port.
6. Upload the firmware.
7. Open the Serial Monitor to verify that ESP-NOW initialises successfully.

### How Eye-Blink Control Works

1. The IR sensor detects whether the user's eye is open or closed.
2. Holding the eye closed for approximately three seconds activates command mode.
3. The system counts individual blinks.
4. Each blink count represents a specific appliance command.
5. The ESP32-C3 sends the command wirelessly to the ESP32 receiver using ESP-NOW.

### Eye-Blink Command Mapping

| Blink Count | Command |
|---|---|
| 1 | Light ON |
| 2 | Light OFF |
| 3 | Fan ON |
| 4 | Fan OFF |
| 5 | TV ON |
| 6 | TV OFF |
| 7 | Massager ON |
| 8 | Massager OFF |

The receiver processes the incoming command and controls the corresponding appliance.

## 8. Automatic Medicine Box Setup

The automatic medicine box is controlled directly by the ESP32 receiver. Both servo motors are connected to the ESP32 receiver, replacing the earlier separate Arduino UNO and DS3231 RTC arrangement.

The current system uses:

- One ESP32 receiver.
- Two servo motors for the medicine box mechanism.
- One buzzer for audible reminders.
- Blynk IoT for configuring and enabling the medicine alarms.
- Network time synchronisation for scheduled alarm operation.

### Connection and Configuration

1. Connect Servo Motor 1 to GPIO 25 of the ESP32 receiver.
2. Connect Servo Motor 2 to GPIO 26 of the ESP32 receiver.
3. Connect the buzzer signal to GPIO 27.
4. Connect the servo power supply and common ground according to the medicine box circuit diagram.
5. Upload the ESP32 receiver firmware.
6. Connect the ESP32 to Wi-Fi so it can synchronise its time.
7. Open the Blynk application.
8. Set the hour and minute for both medicine alarms.
9. Enable the required alarms using their respective switches.
10. Test both servo motors and the buzzer.

### Medicine Box Operation

- The first alarm controls Servo Motor 1.
- The second alarm controls Servo Motor 2.
- Each alarm can be configured separately through the Blynk application.
- The ESP32 checks the configured alarm times.
- When an enabled alarm is due, the corresponding servo operates and the buzzer provides an audible reminder.
- Both alarms can be enabled or disabled independently.

**Important:** No separate RTC module or medicine-box Arduino UNO is required in this current configuration. Both servo motors and the buzzer are controlled by the ESP32 receiver firmware.

## 9. Testing

After completing the hardware connections and uploading the firmware, test each part separately.

### A. Voice-Controlled Appliance System

- Verify that the VC-02 recognises the configured voice commands.
- Check that the correct command codes are sent.
- Test all eight voice commands.
- Verify that the Arduino UNO controls the correct appliance outputs.

### B. Blynk IoT Application

- Check the ESP32 Wi-Fi connection.
- Verify that the Blynk device connects successfully.
- Test the Light, Fan, TV and Massager controls.
- Test both medicine alarm settings.
- Verify that enabling and disabling the alarms works correctly.

### C. Wireless Eye-Blink Control

- Verify that the IR sensor detects eye closure and opening.
- Check that command mode activates after the programmed three-second eye closure.
- Test the one-to-eight blink command mapping.
- Verify that the ESP32-C3 sends commands to the correct receiver MAC address.
- Check that the receiver responds to the wireless commands.

### D. Automatic Medicine Box

- Test Servo Motor 1.
- Test Servo Motor 2.
- Verify the scheduled activation times.
- Check the buzzer alerts.
- Confirm that both alarms can be configured and controlled independently.

## 10. Troubleshooting

If a feature does not work, check the following:

**Voice Control**
- Verify the VC-02 firmware configuration.
- Check that the voice command output codes match the Arduino firmware.
- Recheck the VC-02 and Arduino serial connections.
- Verify the Arduino firmware and selected COM port.

**Blynk Application**
- Check the Wi-Fi credentials.
- Verify the Template ID, Template Name and Authentication Token.
- Check the Virtual Pin datastream configuration.
- Confirm that the widgets are connected to the correct datastreams.

**Wireless Eye-Blink Control**
- Verify the receiver MAC address in the ESP32-C3 firmware.
- Confirm that both boards use the same ESP-NOW Wi-Fi channel.
- Check that ESP-NOW initialises successfully.
- Recheck the IR sensor and buzzer connections.

**Medicine Box**
- Check the servo motor power supply.
- Verify the servo signal connections.
- Check the alarm settings in Blynk.
- Confirm that the ESP32 has successfully synchronised its time.

Always refer to the circuit diagrams and firmware files when troubleshooting.

## 11. Project Files

The repository contains the following resources:

- `hardware/` – Circuit diagrams and hardware connection details.
- `firmware/` – VC-02 firmware and configuration files, Arduino UNO voice-control code, ESP32 MAC address test code, ESP32 receiver code and ESP32-C3 transmitter code.
- `documentation/` – Setup instructions and additional project documentation.
- `images/` – Photographs of the physical prototype and supporting project images.
- `BOM.csv` – Component list, quantities, specifications and purchase references.

Refer to these resources throughout the recreation process.

**Final Note:** You can either use the provided firmware files or develop your own firmware by following the configuration instructions. If you use your own firmware, ensure that the command codes, Virtual Pins, receiver MAC address, communication channel and hardware pin assignments remain compatible with the rest of the system.
