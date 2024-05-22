#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

unsigned char data[4] = {};
float distance;

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
  Serial.begin(57600); // Initialize the USB serial for debugging
  Serial1.begin(9600); // Initialize Serial1 for communication
  
  setupOLED(); // Initialize OLED display
}

void loop() {
  if (Serial1.available() >= 4) // Check if there are at least 4 bytes available
  {
    for (int i = 0; i < 4; i++)
    {
      data[i] = Serial1.read();
    }
    
    if (data[0] == 0xff)
    {
      int sum = (data[0] + data[1] + data[2]) & 0x00FF;
      if (sum == data[3])
      {
        distance = (data[1] << 8) + data[2];
        if (distance > 280)
        {
          Serial.print("distance=");
          Serial.print(distance / 10);
          Serial.println("cm");

          // Display on OLED
          display.clearDisplay();
          display.setCursor(0, 10);
          display.setTextSize(1);  // Ensure text size is set
          String distanceString = "Distance=" + String(distance / 10, 1) + " cm";
          Serial.print("Buffer content: ");
          Serial.println(distanceString); // Debug: Print the buffer
          display.println(distanceString);
          display.display(); // Update the display with new content
        }
        else
        {
          Serial.println("Below the lower limit");

          // Display on OLED
          display.clearDisplay();
          display.setCursor(0, 10);
          display.setTextSize(1);  // Ensure text size is set
          display.println("Below limit");
          display.display(); // Update the display with new content
        }
      }
      else
      {ß
        Serial.println("ERROR");

        // Display on OLED
        display.clearDisplay();
        display.setCursor(0, 10);
        display.setTextSize(1);  // Ensure text size is set
        display.println("ERROR");
        display.display(); // Update the display with new content
      }
    }
  }
  delay(150);
}
