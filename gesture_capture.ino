#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

// ---- GPIO Configuration ----
#define BUTTON_PIN 9      // Physical push button (D9 = GPIO9)
#define RED_PIN    21     // RGB red channel (D6 = GPIO21)
#define GREEN_PIN  20     // RGB green channel (D7 = GPIO20)
#define BLUE_PIN   8      // RGB blue channel (D8 = GPIO8)

// ---- State Variables ----
long last_sample_millis = 0;
bool capture = false;
bool alreadyPressed = false;
unsigned long capture_start_time = 0;
const unsigned long CAPTURE_DURATION = 1000; // Capture window: 1 second

String currentGesture = "Z"; // Default label is "Z"; can switch via Serial input

// ---- RGB Control ----
void initRGB() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

void indicateGestureColor(String g) {
  if (g == "Z")      setRGB(255, 0, 0);   // Red: Z → Fire Bolt
  else if (g == "O") setRGB(0, 255, 0);   // Green: O → Reflect Shield
  else if (g == "V") setRGB(0, 0, 255);   // Blue: V → Healing
  else               setRGB(255, 255, 0); // Yellow: Unknown label
}

// ---- Initialization ----
void setup(void) {
  Serial.begin(115200);
  delay(2000); // Allow Serial to stabilize

  // Initialize I2C (SDA = GPIO6, SCL = GPIO7)
  Wire.begin(6, 7);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  initRGB();
  setRGB(0, 0, 0); // Turn off RGB by default

  // Initialize MPU6050
  while (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    delay(500);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("MPU6050 ready.");
}

// ---- Capture Accelerometer Data ----
void capture_data() {
  if ((millis() - last_sample_millis) >= 10) { // 100Hz sampling rate
    last_sample_millis = millis();

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    Serial.print(a.acceleration.x, 4);
    Serial.print(",");
    Serial.print(a.acceleration.y, 4);
    Serial.print(",");
    Serial.print(a.acceleration.z, 4);
    Serial.println();

    if (millis() - capture_start_time >= CAPTURE_DURATION) {
      capture = false;
      setRGB(0, 255, 0);  // Green LED to indicate capture complete (optional)
      Serial.print("\n\n\n\n");
      Serial.println("Capture complete (1 second)");
    }
  }
}

// ---- Main Loop ----
void loop() {
  // Button-press logic (replaces typing 'o' from Serial)
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && !alreadyPressed && !capture) {
    alreadyPressed = true;

    Serial.println("-,-,-"); // Sync signal for Python script
    Serial.println("Starting capture (will run for 1 second)");
    capture_start_time = millis();
    capture = true;

    indicateGestureColor(currentGesture);  // Light up RGB based on current label
  }

  if (buttonState == HIGH) {
    alreadyPressed = false;
  }

  // Sampling in progress
  if (capture) {
    capture_data();
  }

  // Allow Serial input to change gesture label
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'Z' || c == 'O' || c == 'V') {
      currentGesture = String(c);
      Serial.print("Gesture label changed to: ");
      Serial.println(currentGesture);
    }
  }
}

