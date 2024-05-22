unsigned char data[4] = {};
float distance;
unsigned long previousMillis = 0; // Stores the last time data was read
const long interval = 10000; // Interval between reads (10 seconds)

void setup()
{
  Serial.begin(57600); // Initialize the USB serial for debugging
  Serial1.begin(9600); // Initialize Serial1 for communication
}

void loop()
{
  unsigned long currentMillis = millis(); // Get the current time

  if (currentMillis - previousMillis >= interval) {
    // Save the last time you read the data
    previousMillis = currentMillis;

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
          }
          else
          {
            Serial.println("Below the lower limit");
          }
        }
        else
        {
          Serial.println("ERROR");
        }
      }
    }
  }
}
