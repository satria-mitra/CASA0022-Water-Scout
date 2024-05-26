#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MKRWAN.h>
#include "arduino_secrets.h"

// LoRa parameters
LoRaModem modem;

// Please enter your sensitive data in the Secret tab or arduino_secrets.h
String appEui = SECRET_APP_EUI;
String appKey = SECRET_APP_KEY;

// Define OLED screen parameters
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Sensor pin
#define sensorPin A1
int distance = 0;
unsigned long previousMillis = 0; // Stores the last time data was read
const long sensorInterval = 200; // Interval between sensor reads (200 milliseconds)
const long loraInterval = 60000; // Interval between LoRa sends (60000 milliseconds)

// OLED display initialization
void setupOLED() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);      // Set text size
  display.setTextColor(WHITE); // Set text color
  display.display();           // Display the initialized screen
}

// LoRa initialization
void setupLoRa() {
  // Attempt to connect to the LoRa network
  if (!modem.begin(EU868)) {
    Serial.println("Failed to start module");
    while (1) {}
  }
  
  Serial.print("Your module version is: ");
  Serial.println(modem.version());
  Serial.print("Your device EUI is: ");
  Serial.println(modem.deviceEUI());
  
  Serial.println("Connecting to the network...");
  
  if (!modem.joinOTAA(appEui, appKey)) {
    Serial.println("Something went wrong; are you indoor? Move near a window and retry");
    while (1) {}
  }
  
  Serial.println("Successfully joined network");
  modem.minPollInterval(60);
  modem.dataRate(5);
}

void setup() {
  Serial.begin(9600);
  setupOLED(); // Initialize OLED display
  setupLoRa(); // Initialize LoRa communication
}

void read_sensor() {
  distance = analogRead(sensorPin) * 5;
}

void print_data() {
  //Serial.print("distance = ");
  //Serial.print(distance);
  //Serial.println(" mm");

  // Display on OLED
  display.clearDisplay();
  display.setCursor(0, 10);
  display.setTextSize(1);  // Ensure text size is set
  String distanceString = "Distance=" + String(distance) + " mm";
  display.println(distanceString);
  display.display(); // Update the display with new content
}

void sendLoRaMessage() {
  String payload = String(distance);
  Serial.println(distance);
  int err;

  modem.beginPacket();
  modem.print(payload);
  err = modem.endPacket(true);

  if (err > 0) {
    Serial.println("Message sent correctly!");
  } else {
    Serial.println("Error sending message :(");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Update sensor data and display at sensorInterval
  if (currentMillis - previousMillis >= sensorInterval) {
    previousMillis = currentMillis;

    read_sensor();
    print_data();
  }

  // Send data over LoRa at loraInterval
  static unsigned long previousLoRaMillis = 0;
  if (currentMillis - previousLoRaMillis >= loraInterval) {
    previousLoRaMillis = currentMillis;
    sendLoRaMessage();
  }
}
