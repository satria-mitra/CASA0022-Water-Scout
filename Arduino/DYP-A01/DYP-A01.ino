#include <MKRWAN.h>
#include "arduino_secrets.h"

unsigned long previousMillis = 0; // Store the last time the sensor was read
const long interval = 60000; // Interval to read the sensor (10000 milliseconds or 10 seconds)

LoRaModem modem;

void setup() {
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
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    // Save the last time the sensor was read
    previousMillis = currentMillis;

    // Clear the serial buffer
    while (Serial1.available()) {
      Serial1.read();
    }
    
    // Send a trigger pulse
    Serial1.write(0x55); // Sending any data to trigger the sensor

    // Wait for the response with a timeout mechanism
    unsigned long responseTimeout = millis() + 100; // 100ms timeout
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
              String payload =String(distance);

              // Send data over LoRa
              modem.beginPacket();
              modem.print(payload);
              int err = modem.endPacket(true);  // true means async mode
              if (err > 0) {
                Serial.println("Message sent correctly!");
              } else {
                Serial.println("Error sending message :(");
              }
            } else {
              Serial.println("Checksum error");
              Serial.print("Data_H: ");
              Serial.print(Data_H, HEX);
              Serial.print(", Data_L: ");
              Serial.print(Data_L, HEX);
              Serial.print(", SUM: ");
              Serial.print(SUM, HEX);
              Serial.print(", Calculated SUM: ");
              Serial.println(calculatedSUM, HEX);
            }
          } else {
            Serial.println("Timeout waiting for full data packet");
          }
          // Break the loop after reading and processing the data once
          break;
        }
      }
    }
  }

  // Other tasks can be performed here without blocking
}
