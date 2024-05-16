#define sensorPin A1
int distance = 0;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);

}

void read_sensor() {
distance = analogRead(sensorPin) * 5;
}

void print_data() {
Serial.print("distance = ");
Serial.print(distance);
Serial.println(" mm");
}

void loop() {
read_sensor();
print_data();
delay(1000);
}