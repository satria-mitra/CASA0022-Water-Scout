/* 
*****************************
Water Height Monitoring
Sensor : Adafruit DYP-A01, Adafruit MAX17048
Developed by : Satria Utama
Last Update : 10 July 2024
V1.8
*****************************

*/

#include <MKRWAN.h>
#include <ArduinoLowPower.h>
#include <Adafruit_MAX1704X.h>
#include "arduino_secrets.h"

/******** Pin, Variables and Class for Solar Charge Controller **********/
// Pin definitions for charge controller
#define PGOOD_PIN 1  // Power Good status pin
#define CHG_PIN 2    // Charge status pin

// Number of samples to take for determining status
#define CHARGE_SAMPLES 10
#define SAMPLE_DELAY 100  // Delay between samples in milliseconds

enum SolarStatus { OFF = 0, ACTIVE = 1 };
enum BatteryStatus { CHARGING = 0, FULL = 1, DRAINING = 2 };

// Variables to hold the status of the solar panel and battery
SolarStatus solar_status = OFF;
BatteryStatus battery_status = DRAINING;

// Class for Solar Charge Controller
class DeviceHealth {
public:
  SolarStatus solar_status;
  BatteryStatus battery_status;
  float batteryVoltage;
  float batteryPercentage;
  void updateDeviceHealth();
  void printDeviceHealth();
};
// Declare class globally
DeviceHealth deviceHealth;


/******** Pin, Variables and Class for Ultrasonic Sensor **********/

const int sensorPowerPin = 4; // Pin to control MOSFET gate
int interval = 55500; //595000; // Interval to wake up (60000 milliseconds or 60 seconds)

class DYP_A01 {
public:
  void readSensor();
};

// Declare class globally
DYP_A01 sensor;


/******** Pin, Variables and Class for LoRaWAN Connections **********/
#define MAX_JOIN_ATTEMPTS 8
#define JOIN_RETRY_DELAY_MINUTES 2 
#define JOIN_RETRY_SLEEP_MINUTES 30 // Rejoin LoRa Network in the next 30 minutes
#define LORA_CONNECTION_STABILIZATION_DELAY_SECONDS 5 // Stabilize LoRa connections

// Payload in CayenneLPP Format
byte payload[10];

// Class for LoRaWAN Connection
class LoRaWAN {
public:
  LoRaModem modem;
  String appEui = SECRET_APP_EUI;
  String appKey = SECRET_APP_KEY;
  uint16_t packetCount = 0;

  void init();
  void executeDownlink();
  void sendData(byte* payload, size_t payloadSize);
};

// Declare class globally
LoRaWAN lorawan;

/******** Class and Function for Power Management **********/
class PowerManagement {
public:
  void enablePowerSavingMode();
};

// Declare class globally
PowerManagement powerManagement;

/******** Class for Battery Monitoring **********/
class BatteryMonitor {
public:
  Adafruit_MAX17048 maxlipo;
  void begin();
  void readBattery();
  void powerOff();
  void powerOn();
};

// Declare class globally
BatteryMonitor batteryMonitor;

void setup() {
  // Enable power-saving mode
  powerManagement.enablePowerSavingMode();

  // Start the serial communication with the sensor and the serial monitor
  Serial.begin(9600);   // Serial monitor
  Serial1.begin(9600);  // Serial1 for hardware serial communication with the sensor
  Serial.println("Ultrasonic Sensor UART Test");

  // Initialize LoRaWAN Connections
  lorawan.init();

  // Initialize Battery Monitor
  batteryMonitor.begin();

  // Set pin modes for charge controller with pull-up resistors
  // pinMode(PGOOD_PIN, INPUT_PULLUP);
  // pinMode(CHG_PIN, INPUT_PULLUP);

  // Attach wakeup function to handle wakeup event
  //LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, wakeup, CHANGE);
}


void loop() {
  sensor.readSensor();
  batteryMonitor.powerOn();
  batteryMonitor.readBattery();
  batteryMonitor.powerOff();
  
  printPayload(payload, sizeof(payload));

  lorawan.sendData(payload, sizeof(payload));
  lorawan.executeDownlink();

  // Put the board to sleep for the defined interval
  LowPower.deepSleep(interval);
}

void LoRaWAN::init() {
  int joinAttempts = 0;
    // Initialize LoRa module
  if (!modem.begin(EU868)) {
    Serial.println("Failed to start module");
    while (1) {}
  }
  Serial.print("Your module version is: ");
  Serial.println(modem.version());
  Serial.print("Your device EUI is: ");
  Serial.println(modem.deviceEUI());

  // Join the network
  int connected = modem.joinOTAA(appEui, appKey);

  // Attempt to reconnect to LoRa
  while (!connected && joinAttempts < MAX_JOIN_ATTEMPTS) {
    joinAttempts++;
    Serial.println("Not connected, trying again...");
    delay(JOIN_RETRY_DELAY_MINUTES * 60 * 1000);  // wait before next attempt
    connected = modem.joinOTAA(appEui, appKey);
  }

  if (connected) {
    Serial.println("Successfully joined the network");
  } else {
    Serial.println("Failed to join network after several attempts. The device is going into a low power mode before trying again.");
    LowPower.sleep(JOIN_RETRY_SLEEP_MINUTES * 60 * 1000);  // sleep
    init();  // try to initialize the LoRaWAN connection again
  }

  modem.setADR(true);
  modem.minPollInterval(60);
  delay(LORA_CONNECTION_STABILIZATION_DELAY_SECONDS * 1000);  // wait to stabilize connection
}

void LoRaWAN::executeDownlink() {
  if (!modem.available()) {
    Serial.println("No downlink message received at this time.");
    return;
  }
  char rcv[64];
  int i = 0;
  while (modem.available()) {
    rcv[i++] = (char)modem.read();
  }

  rcv[i] = '\0'; // Null-terminate the string

  Serial.print("Received: ");
  for (unsigned int j = 0; j < i; j++) {
    Serial.print(rcv[j] >> 4, HEX);
    Serial.print(rcv[j] & 0xF, HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Check if the received command is "1"
  if (rcv[0] == 0x01) {
    Serial.println("Received reset command, sending reset payload and resetting the board.");
    
    // Prepare reset payload
    byte resetPayload[4] = { 'R', 'E', 'S', 'T' };

    // Send reset payload
    sendData(resetPayload, sizeof(resetPayload));

    // Delay to ensure the payload is sent
    delay(1000);

    // Reset the board
    NVIC_SystemReset();
  } else {
      Serial.println("No valid reset command received.");
    }
}

void LoRaWAN::sendData(byte* payload, size_t payloadSize) {
  int err;
  modem.beginPacket();
  modem.write(payload, payloadSize);
  err = modem.endPacket(true);

  if (err > 0) {
    packetCount++;
    Serial.println("LoRa Packet sent");
  } else {
    Serial.println("Error sending LoRa Packet");
  }
  delay(1000);
}

void DYP_A01::readSensor() {
  // Turn on the sensor by setting the MOSFET gate HIGH
  Serial.println("Turning on the sensor...");
  digitalWrite(sensorPowerPin, HIGH);

  delay(2000); // Wait for the sensor to power up

  // Clear the serial buffer
  while (Serial1.available()) {
    Serial1.read();
  }

  // Send a trigger pulse
  Serial1.write(0x55); // Sending any data to trigger the sensor

  // Wait for the response with a timeout mechanism
  unsigned long responseTimeout = millis() + 100; // 100ms timeout
  bool sensorDataReceived = false;
  int distance = -1;

  while (millis() < responseTimeout) {
    if (Serial1.available()) {
      // Read the frame header
      if (Serial1.read() == 0xFF) {
        // Ensure enough data is available
        while (Serial1.available() < 3 && millis() < responseTimeout) {
          // Wait for the remaining data to be available
        }

        if (Serial1.available() >= 3) {
          // Read the next three bytes
          byte Data_H = Serial1.read();
          byte Data_L = Serial1.read();
          byte SUM = Serial1.read();

          // Verify checksum
          byte calculatedSUM = (0xFF + Data_H + Data_L) & 0xFF;
          if (calculatedSUM == SUM) {
            // Calculate distance
            distance = (Data_H << 8) + Data_L;
            Serial.print("Distance: ");
            Serial.print(distance);
            Serial.println(" mm");
            sensorDataReceived = true;
          } else {
            Serial.println("Checksum error");
          }
        } else {
          Serial.println("Timeout waiting for full data packet");
        }
        break; // Exit the loop after reading data
      }
    }
  }

  if (!sensorDataReceived) {
    Serial.println("No sensor data received");
  }

  // Turn off the sensor to save power
  Serial.println("Turning off the sensor...");
  digitalWrite(sensorPowerPin, LOW);

  // Update and print device health status
  deviceHealth.updateDeviceHealth();
  deviceHealth.printDeviceHealth();

  // Construct the status byte into CayenneLPP format
  int batteryVoltageMilliVolts = deviceHealth.batteryVoltage * 1000;
  int batteryPercentageHundredths = deviceHealth.batteryPercentage * 100;

  payload[0] = highByte(distance);
  payload[1] = lowByte(distance);
  payload[2] = deviceHealth.battery_status;
  payload[3] = deviceHealth.solar_status;
  payload[4] = highByte(lorawan.packetCount);
  payload[5] = lowByte(lorawan.packetCount);
  payload[6] = highByte(batteryVoltageMilliVolts); // Convert voltage to mV
  payload[7] = lowByte(batteryVoltageMilliVolts);
  payload[8] = highByte(batteryPercentageHundredths); // Convert percentage to 0.01%
  payload[9] = lowByte(batteryPercentageHundredths);
}


// Function to update the status of the solar panel and battery
void DeviceHealth::updateDeviceHealth() {
  int pgood_count = 0;
  int chg_count = 0;

  // Take multiple samples to determine status
  for (int i = 0; i < CHARGE_SAMPLES; i++) {
    int pgood_value = digitalRead(PGOOD_PIN);
    int chg_value = digitalRead(CHG_PIN);
    
    // Debug prints for pin values
    Serial.print("Sample ");
    Serial.print(i);
    Serial.print(" - PGOOD_PIN value: ");
    Serial.println(pgood_value);
    Serial.print("Sample ");
    Serial.print(i);
    Serial.print(" - CHG_PIN value: ");
    Serial.println(chg_value);

    pgood_count += (pgood_value == LOW) ? 1 : 0;
    chg_count += (chg_value == LOW) ? 1 : 0;
    delay(SAMPLE_DELAY);
  }

  // Debugging counts
  Serial.print("PGOOD count: ");
  Serial.println(pgood_count);
  Serial.print("CHG count: ");
  Serial.println(chg_count);

  // Determine battery status
  if (chg_count > CHARGE_SAMPLES / 2) {
    battery_status = CHARGING;
    solar_status = ACTIVE;  // Consider solar panel active if charging
  } else if (pgood_count > CHARGE_SAMPLES / 2) {
    solar_status = ACTIVE;
    battery_status = FULL;  // Assume battery is full if not charging but solar is active
  } else {
    solar_status = OFF;
    battery_status = DRAINING;
  }

  // Read battery voltage and percentage from MAX17048
  batteryVoltage = batteryMonitor.maxlipo.cellVoltage();
  batteryPercentage = batteryMonitor.maxlipo.cellPercent();

  // Debug output for final status
  Serial.println(solar_status == ACTIVE ? "Solar panel is ACTIVE." : "Solar panel is OFF.");
  switch (battery_status) {
    case CHARGING:
      Serial.println("Battery is CHARGING.");
      break;
    case FULL:
      Serial.println("Battery is FULL.");
      break;
    case DRAINING:
      Serial.println("Battery is DRAINING.");
      break;
  }
  Serial.print("Battery Voltage: ");
  Serial.print(batteryVoltage);
  Serial.println(" V");
  Serial.print("Battery Percentage: ");
  Serial.print(batteryPercentage);
  Serial.println(" %");
  Serial.println();
}






// Function to print the current status of the solar panel and battery
void DeviceHealth::printDeviceHealth() {
  // Print solar panel status
  if (solar_status == ACTIVE) {
    Serial.println("Solar panel is ACTIVE.");
  } else {
    Serial.println("Solar panel is OFF.");
  }

  // Print battery status
  switch (battery_status) {
    case CHARGING:
      Serial.println("Battery is CHARGING.");
      break;
    case FULL:
      Serial.println("Battery is FULL.");
      break;
    case DRAINING:
      Serial.println("Battery is DRAINING.");
      break;
  }

  // Print battery voltage and percentage
  Serial.print("Battery Voltage: ");
  Serial.print(batteryVoltage);
  Serial.println(" V");

  Serial.print("Battery Percentage: ");
  Serial.print(batteryPercentage);
  Serial.println(" %");

  // Print a blank line for readability
  Serial.println();
}

// Function to print the payload array in hexadecimal format
void printPayload(byte* payload, size_t payloadSize) {
  Serial.print("Payload: ");
  for (size_t i = 0; i < payloadSize; i++) {
    if (payload[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(payload[i], HEX);
    if (i < payloadSize - 1) {
      Serial.print(" ");
    }
  }
  Serial.println();
}

void wakeup() {
  // This function will be called when the microcontroller wakes up
  // Re-enable peripherals if needed
  // Serial.begin(9600);   // Serial monitor
  // Serial1.begin(9600);  // Serial1 for hardware serial communication with the sensor
  //Serial.println("Woke up from sleep");
}

void PowerManagement::enablePowerSavingMode() {
  // Set digital pins to input
  for (int pin = 0; pin < 12; pin++) {
    pinMode(pin, INPUT);
    digitalWrite(pin, LOW);
  }

  // Disable ADC
  ADC->CTRLA.bit.ENABLE = 0;
  while (ADC->STATUS.bit.SYNCBUSY);

  // Disable DAC
  DAC->CTRLA.bit.ENABLE = 0;
  while (DAC->STATUS.bit.SYNCBUSY);

  // Disable SERCOM4
  SERCOM4->USART.CTRLA.bit.ENABLE = 0;
  while (SERCOM4->USART.SYNCBUSY.bit.ENABLE);

  // Initialize the sensor power pin
  pinMode(sensorPowerPin, OUTPUT);
  digitalWrite(sensorPowerPin, LOW); // Ensure the sensor is off initially

  // Initialize the charge controller pins
  // pinMode(PGOOD_PIN, OUTPUT);
  // pinMode(CHG_PIN, OUTPUT);
  digitalWrite(PGOOD_PIN, LOW);
  digitalWrite(CHG_PIN, LOW);

  USBDevice.detach();
}

void BatteryMonitor::begin() {
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);

  if (!maxlipo.begin()) {
    Serial.println(F("Could not find Adafruit MAX17048. Please check wiring!"));
    while (1) delay(10);
  }
  Serial.print(F("Found MAX17048 with Chip ID: 0x")); 
  Serial.println(maxlipo.getChipID(), HEX);
}

void BatteryMonitor::readBattery() {
  float voltage = maxlipo.cellVoltage();
  float percentage = maxlipo.cellPercent();

  if (isnan(voltage) || isnan(percentage)) {
    Serial.println("Failed to read from MAX17048, check the battery connection!");
  } else {
    deviceHealth.batteryVoltage = voltage;
    deviceHealth.batteryPercentage = percentage;
  }
}

void BatteryMonitor::powerOff() {
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  Serial.println("Battery monitor powered off.");
}

void BatteryMonitor::powerOn() {
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  Serial.println("Battery monitor powered on.");
  delay(500);
}