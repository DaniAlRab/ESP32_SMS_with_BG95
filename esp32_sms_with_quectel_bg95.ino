#include <BluetoothSerial.h>
#include <EEPROM.h>
#include <esp_system.h>

#define BG95_TX_PIN 17
#define BG95_RX_PIN 16
#define SMS_TRIGGER_PIN 18
#define PROGRAMMING_MODE_PIN 19
#define BG95_POWER_PIN 23
#define LED_PIN 12

HardwareSerial bg95Serial(1);
BluetoothSerial SerialBT;

bool smsSent = false;
unsigned long lastTriggerTime = 0;
const unsigned long debounceDelay = 200;
String phoneNumber;  // Single phone number storage
int smsInterval = 30000;  // Interval of 30 seconds between SMS
bool programmingMode = false;
String btPassword = "1234"; // Default password
bool authenticated = false;

void setup() {
  Serial.begin(115200);
  pinMode(SMS_TRIGGER_PIN, INPUT);
  pinMode(PROGRAMMING_MODE_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BG95_POWER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  powerOnBG95();
  EEPROM.begin(512);  // Initialize EEPROM
  loadPhoneNumber();   // Load phone number from EEPROM

  if (digitalRead(PROGRAMMING_MODE_PIN) == LOW) {
    programmingMode = true;
    Serial.println("Programming mode activated. Initializing Bluetooth...");
    SerialBT.begin("ESP32_BT");
    blinkLEDFast();
  } else {
    Serial.println("Version, 14.0");
    Serial.println("Normal mode. Bluetooth is not initialized.");
  }

  bg95Serial.begin(115200, SERIAL_8N1, BG95_RX_PIN, BG95_TX_PIN);
  Serial.println("Initializing BG95...");

  if (checkBG95Presence()) {
    Serial.println("BG95 module is present and responding.");
  } else {
    Serial.println("BG95 module not detected.");
    rebootSystem();
  }

  configureBG95();

  if (checkNetworkRegistration()) {
    Serial.println("BG95 successfully registered on the network.");
    digitalWrite(LED_PIN, HIGH);
    syncTime();
  } else {
    Serial.println("BG95 failed to register on the network.");
    digitalWrite(LED_PIN, LOW);
  }

  delay(1000);
}

void loop() {
  if (programmingMode && SerialBT.available()) {
    handleBluetoothCommand();
  }

  if (programmingMode) {
    blinkLEDFast();
    return;
  }

  unsigned long currentTime = millis();

  if (digitalRead(SMS_TRIGGER_PIN) == HIGH && !smsSent && (currentTime - lastTriggerTime > debounceDelay)) {
    sendSMS("Device Plugged");
    smsSent = true;
    lastTriggerTime = currentTime;
  }

  if (digitalRead(SMS_TRIGGER_PIN) == LOW && smsSent && (currentTime - lastTriggerTime > debounceDelay)) {
    sendSMS("Device Unplugged");
    smsSent = false;
    lastTriggerTime = currentTime;
  }

  if (bg95Serial.available()) {
    String response = bg95Serial.readString();
    Serial.print("BG95: ");
    Serial.println(response);
  }
}

void sendSMS(String message) {
  if (phoneNumber.length() > 0) {
    Serial.println("Sending SMS to: " + phoneNumber);
    sendATCommand("AT+CMGF=1");
    sendATCommand("AT+CMGS=\"" + phoneNumber + "\"");
    bg95Serial.print(message);
    bg95Serial.write(26);  // Send CTRL+Z to finish SMS
    Serial.println("SMS sent to: " + phoneNumber);
    delay(smsInterval);  // Wait for the interval between sending SMS
  } else {
    Serial.println("No phone number stored to send SMS.");
  }
} 


#Bluetooth funcion to change sms number

void handleBluetoothCommand() {
  String command = SerialBT.readStringUntil('\n');
  command.trim();

  if (!authenticated) {
    if (command.startsWith("AUTH")) {
      String password = command.substring(command.indexOf('=') + 1);
      if (password == btPassword) {
        authenticated = true;
        SerialBT.println("Authenticated.");
      } else {
        SerialBT.println("Wrong password.");
      }
    } else {
      SerialBT.println("Please authenticate first with AUTH=<password>.");
    }
    return;
  }

  if (command.startsWith("SET_NUMBER")) {
    phoneNumber = command.substring(command.indexOf('=') + 1);
    storePhoneNumber();  // Store the new number in EEPROM
    SerialBT.println("Number saved: " + phoneNumber);
  } else if (command.startsWith("SET_INTERVAL")) {
    smsInterval = command.substring(command.indexOf('=') + 1).toInt() * 1000;  // Convert to milliseconds
    SerialBT.println("SMS interval set to: " + String(smsInterval / 1000) + " seconds.");
  } else if (comand == "SHOW_NUMBER") {
    SerialBT.println("Stored number: " + phoneNumber);
  } else if (command == "CLEAR_NUMBER") {
    phoneNumber = "";  // Clear the phone number
    storePhoneNumber();  // Update EEPROM
    SerialBT.println("Phone number cleared.");
  } else {
    SerialBT.println("Unknown command.");
  }
}

void storePhoneNumber() {
  // Store the phone number in EEPROM (limit to 100 chars)
  for (int i = 0; i < 100; i++) {
    if (i < phoneNumber.length()) {
      EEPROM.write(i, phoneNumber[i]);
    } else {
      EEPROM.write(i, 0);  // Clear remaining space
    }
  }
  EEPROM.commit();  // Commit changes to EEPROM
}

void loadPhoneNumber() {
  phoneNumber = "";
  for (int i = 0; i < 100; i++) {
    char c = char(EEPROM.read(i));
    if (c == 0) break;
    phoneNumber += c;
  }
}

void sendATCommand(String command) {
  Serial.print("Sending command: ");
  Serial.println(command);
  bg95Serial.print(command);
  bg95Serial.print("\r\n");
  delay(1000);
}

bool checkBG95Presence() {
  delay(10000);
  sendATCommand("AT");
  unsigned long startTime = millis();
  
  while (millis() - startTime < 10000) {
    if (bg95Serial.available()) {
      String response = bg95Serial.readString();
      Serial.println("Response: " + response);
      if (response.indexOf("OK") != -1) {
        return true;
      }
    }
  }
  
  return false;
}

#Commands to configure modem 

void configureBG95() {
  sendATCommand("ATE0");
  sendATCommand("AT+CPIN?");
  sendATCommand("AT+CIMI");
  sendATCommand("AT+CFUN=0");
  sendATCommand("AT+CMEE=2");
  sendATCommand("AT+QICFG=\"tcp/keepalive\",1,1,25,3");
  sendATCommand("AT+QCFG=\"band\",0xf,0x8000004,0x8000004");
  sendATCommand("AT+QCFG=\"nwscanseq\",00,1");
  sendATCommand("AT+QCFG=\"iotopmode\",2,1");
  sendATCommand("AT+QICSGP=1,1,\"zap.vivo.com.br\",\"vivo\",\"vivo\",2");
  sendATCommand("AT+CFUN=1");
}

bool checkNetworkRegistration() {
  unsigned long startTime = millis();
  unsigned long endTime = startTime + 60000;
  
  while (millis() < endTime) {
    sendATCommand("AT+CEREG?");
    delay(5000);

    if (bg95Serial.available()) {
      String response = bg95Serial.readString();
      Serial.println("CEREG Response: " + response);
      if (response.indexOf("+CEREG: 0,1") != -1) {
        return true;
      }
    }
  }
  
  return false;
}

void syncTime() {
  Serial.println("Synchronizing time using NTP...");
  sendATCommand("AT+QNTP=1,\"time.google.com\"");
  delay(5000);
  Serial.println("Time synchronization complete.");
}

void blinkLEDFast() {
  static unsigned long lastBlinkTime = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastBlinkTime >= 100) {
    lastBlinkTime = currentTime;
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
}

void powerOnBG95() {
  digitalWrite(BG95_POWER_PIN, HIGH);
  delay(5000);
  digitalWrite(BG95_POWER_PIN, LOW);
}

void rebootSystem() {
  Serial.println("Rebooting system...");
  esp_restart();
}m
