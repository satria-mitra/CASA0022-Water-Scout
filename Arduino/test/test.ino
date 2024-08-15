#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <SD.h>

// SD card chip select pin
//const int chipSelect = 5;

// OLED screen parameters
//#define SCREEN_WIDTH 128 // OLED display width, in pixels
//#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Ultrasonic sensor interval
const unsigned long sensorInterval = 6000;  // 10 seconds
unsigned long previousMillis = 0;

// Global variable to store distance
int distance = -1;

// Function to initialize OLED display
// void setupOLED() {
//   if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
//     Serial.println(F("SSD1306 allocation failed"));
//     for (;;); // Don't proceed, loop forever
//   }
//   display.clearDisplay();
//   display.setTextSize(1);      // Set text size
//   display.setTextColor(WHITE); // Set text color
//   display.display();           // Display the initialized screen
// }

// Function to print data to OLED
// void printDataToOLED(String dataString) {
//   display.clearDisplay();
//   display.setCursor(0, 10);
//   display.setTextSize(1);  // Ensure text size is set
//   display.println(dataString);
//   display.display(); // Update the display with new content
// }

// Function to write data to SD card
// void writeDataToSD(String dataString) {
//   File dataFile = SD.open("datalog.txt", FILE_WRITE);
//   if (dataFile) {
//     dataFile.println(dataString);
//     dataFile.close();
//     Serial.println("Data written to SD card.");
//   } else {
//     Serial.println("Error opening datalog.txt");
//   }
// }

// Ultrasonic sensor read function
void readSensor() {
  ///delay(500); // Wait for the sensor to power up

  // Clear the serial buffer
  while (Serial1.available()) {
    Serial1.read();
  }

  // Send a trigger pulse
  Serial1.write(0x55);  // Sending any data to trigger the sensor

  // Wait for the response with a timeout mechanism
  unsigned long responseTimeout = millis() + 100;  // 100ms timeout
  bool sensorDataReceived = false;
  distance = -1;

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
        break;  // Exit the loop after reading data
      }
    }
  }

  if (!sensorDataReceived) {
    Serial.println("No sensor data received");
  }
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial1.begin(9600);  // Initialize Serial1 for the sensor

  //setupOLED(); // Initialize OLED display

  //Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  //   if (!SD.begin(chipSelect)) {
  //     Serial.println("Card failed, or not present");
  //     // don't do anything more:
  //     while (1);
  //   }
  //   Serial.println("Card initialized.");
  // }

  void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= sensorInterval) {
      previousMillis = currentMillis;

      readSensor();

      // if (distance != -1) { // Check if valid distance is read
      //   String dataString = "Distance=" + String(distance) + " mm";
      //   printDataToOLED(dataString);
      //   writeDataToSD(dataString);
    