#include <MKRWAN.h>
#include <ArduinoLowPower.h>
#include "arduino_secrets.h"

const int sensorPowerPin = 4; // Pin to control MOSFET gate
int interval = 57000; // Interval to wake up (60000 milliseconds or 60 seconds)

LoRaModem modem;

void setup() {
  // Disable unused peripherals
  // ADC->CTRLA.bit.ENABLE = 0;
  // while (ADC->STATUS.bit.SYNCBUSY);

  // DAC->CTRLA.bit.ENABLE = 0;
  // while (DAC->STATUS.bit.SYNCBUSY);

  // SERCOM4->USART.CTRLA.bit.ENABLE = 0;
  // while (SERCOM4->USART.SYNCBUSY.bit.ENABLE);

  // Initialize the sensor power pin
  pinMode(sensorPowerPin, OUTPUT);
  digitalWrite(sensorPowerPin, LOW); // Ensure the sensor is off initially

  // Start the serial communication with the sensor and the serial monitor
  Serial.begin(9600);   // Serial monitor
  Serial1.begin(9600);  // Serial1 for hardware serial communication with the sensor
  Serial.println("Ultrasonic Sensor UART Test");

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
  int connected = modem.joinOTAA(SECRET_APP_EUI, SECRET_APP_KEY);
  if (!connected) {
    Serial.println("Failed to join the network");
    while (1) {}
  }
  Serial.println("Successfully joined the network");

  // Attach wakeup function to handle wakeup event
  //LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, wakeup, CHANGE);
}

void loop() {
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
  unsigned long responseTimeout = millis() + 200; // 100ms timeout
  bool sensorDataReceived = false;
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
            int distance = (Data_H << 8) + Data_L;
            Serial.print("Distance: ");
            Serial.print(distance);
            Serial.println(" mm");

            // Prepare payload
            String payload = String(distance);

            // Send data over LoRa
            modem.beginPacket();
            modem.print(payload);
            int err = modem.endPacket(true);  // true means async mode
            if (err > 0) {
              Serial.println("Message sent correctly!");
            } else {
              Serial.println("Error sending message :(");
            }
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

  // Put the board to sleep for the defined interval
  Serial.println("Entering sleep mode...");
  LowPower.deepSleep(interval);
}

void wakeup() {
  // This function will be called when the microcontroller wakes up
  // Re-enable peripherals if needed
  // Serial.begin(9600);   // Serial monitor
  // Serial1.begin(9600);  // Serial1 for hardware serial communication with the sensor
  //Serial.println("Woke up from sleep");
}
