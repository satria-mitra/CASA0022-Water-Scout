#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define CONTROL_PIN 5   // This is the YELLOW wire, can be any data line

int16_t distance;  // The last measured distance
bool newData = false; // Whether new data is available from the sensor
uint8_t buffer[4];  // our buffer for storing data
uint8_t idx = 0;  // our idx into the storage buffer

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
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Adafruit DYP-ME007YS Test");

  // set the data rate for the Serial port, 9600 for the sensor
  Serial1.begin(9600);
  pinMode(CONTROL_PIN, OUTPUT);
  digitalWrite(CONTROL_PIN, HIGH);

  setupOLED(); // Initialize OLED display
}

void loop() { // run over and over
  if (Serial1.available()) {
    uint8_t c = Serial1.read();
    //Serial.println(c, HEX);

    // See if this is a header byte
    if (idx == 0 && c == 0xFF) {
      buffer[idx++] = c;
    }
    // Two middle bytes can be anything
    else if ((idx == 1) || (idx == 2)) {
      buffer[idx++] = c;
    }
    else if (idx == 3) {
      uint8_t sum = 0;
      sum = buffer[0] + buffer[1] + buffer[2];
      if (sum == c) {
        distance = ((uint16_t)buffer[1] << 8) | buffer[2];
        newData = true;
      }
      idx = 0;
    }
  }
  
  if (newData) {
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" mm");

    // Display on OLED
    display.clearDisplay();
    display.setCursor(0, 10);
    display.setTextSize(1);  // Ensure text size is set
    String distanceString = "Distance=" + String(distance) + " mm";
    //Serial.print("Buffer content: ");
    //Serial.println(distanceString); // Debug: Print the buffer
    display.println(distanceString);
    display.display(); // Update the display with new content

    newData = false;
  }
}
