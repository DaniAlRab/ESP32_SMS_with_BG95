# BG95 SMS Communication and Bluetooth Programming with ESP32

## Project Overview
This project provides a firmware implementation for the **ESP32 microcontroller** to manage SMS communications via a **Quectel BG95 modem** and Bluetooth-based configuration. It supports sending SMS alerts based on external triggers, programming phone numbers and SMS intervals via Bluetooth, and synchronizing time using an NTP server.

---

## Key Features
- **SMS Alerts:**
  - Monitors a trigger pin to send SMS notifications for "Device Plugged" and "Device Unplugged" events.

- **Bluetooth Configuration:**
  - Allows updating the phone number and SMS interval via Bluetooth commands.
  - Supports password-based authentication for secure programming mode.

- **EEPROM Storage:**
  - Stores and retrieves configuration data (phone number) from EEPROM.

- **BG95 Modem Management:**
  - Initializes and configures the Quectel BG95 module for network communication.
  - Sends AT commands to check network registration and SMS capabilities.

- **Time Synchronization:**
  - Synchronizes the BG95 module's internal clock using an NTP server.

- **System Monitoring:**
  - Checks BG95 modem presence and network registration status.
  - Reboots the ESP32 in case of errors.

---

## Hardware Requirements
- **ESP32 Development Board**
- **Quectel BG95 Module** (connected via UART)
- **LED Indicator** (GPIO 12)
- **Push Buttons**:
  - SMS Trigger Button (GPIO 18)
  - Programming Mode Button (GPIO 19)

---

## Workflow
1. **Initialization:**
   - Configures GPIO pins and initializes EEPROM.
   - Checks BG95 modem presence and network registration.
   - Activates Bluetooth if programming mode is enabled.

2. **Trigger Monitoring:**
   - Detects changes on the SMS trigger pin and sends SMS alerts based on the event.

3. **Bluetooth Commands:**
   - Allows users to set phone numbers and SMS intervals securely.

4. **Error Handling:**
   - Monitors modem responses and reboots the system if errors are detected.

---

## Bluetooth Commands
- **Authentication:**
  - `AUTH=<password>`: Authenticate using a password (default: `1234`).
- **Set Phone Number:**
  - `SET_NUMBER=<phone_number>`: Stores the phone number.
- **Set SMS Interval:**
  - `SET_INTERVAL=<seconds>`: Sets the interval between SMS notifications.
- **Show Stored Number:**
  - `SHOW_NUMBER`: Displays the currently stored phone number.
- **Clear Phone Number:**
  - `CLEAR_NUMBER`: Deletes the stored phone number.

---

## Configuration
- **EEPROM Storage:**
  - Stores up to 100 characters for phone numbers.
- **Bluetooth Name:**
  - Defaults to `ESP32_BT` when in programming mode.
- **Modem Commands:**
  - Sends specific AT commands for BG95 configuration and network registration.

---

## Requirements
- **Arduino IDE or PlatformIO** with ESP32 support.
- Libraries:
  - `BluetoothSerial.h`
  - `EEPROM.h`
  - `esp_system.h`
- **Quectel BG95 Module** connected to GPIO 16 (RX) and GPIO 17 (TX).

---

## Usage
1. Connect the hardware and upload the code.
2. Activate **Programming Mode** by holding the button on GPIO 19 during startup.
3. Use a Bluetooth terminal app to connect to the ESP32 and send commands.
4. Configure the phone number and SMS interval as needed.
5. Monitor logs via serial output to verify modem status and SMS delivery.

---

## Example Log Output
```
Initializing BG95...
BG95 module is present and responding.
BG95 successfully registered on the network.
Synchronizing time using NTP...
Time synchronization complete.
Sending SMS to: +1234567890
SMS sent to: +1234567890
```

---

## Notes
- The system blinks the LED rapidly in programming mode.
- SMS alerts are sent only if the configured phone number is valid.
- The BG95 modem requires sufficient initialization time and network registration before SMS operations.

---
