#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define sensorPin A1
int distance = 0;
unsigned long previousMillis = 0; // Stores the last time data was read
const long interval = 200; // Interval between reads (200 milliseconds)

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

void setup() {
  Serial.begin(9600);
  setupOLED(); // Initialize OLED display
}

void read_sensor() {
  distance = analogRead(sensorPin) * 5;
}

void print_data() {
  Serial.print("distance = ");
  Serial.print(distance);
  Serial.println(" mm");

  // Display on OLED
  display.clearDisplay();
  display.setCursor(0, 10);
  display.setTextSize(1);  // Ensure text size is set
  String distanceString = "Distance=" + String(distance) + " mm";
  display.println(distanceString);
  display.display(); // Update the display with new content
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    read_sensor();
    print_data();
  }
}
