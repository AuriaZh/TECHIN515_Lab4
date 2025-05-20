#include <auriazh-project-1_inferencing.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// MPU6050 sensor
Adafruit_MPU6050 mpu;

// GPIO pin definitions
#define BUTTON_PIN 9
#define RED_PIN    21
#define GREEN_PIN  20
#define BLUE_PIN   8

// Sampling settings
#define SAMPLE_RATE_MS 10
#define CAPTURE_DURATION_MS 1000
#define FEATURE_SIZE EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE

// Capture control
bool capturing = false;
bool buttonPressed = false;
unsigned long last_sample_time = 0;
unsigned long capture_start_time = 0;
int sample_count = 0;
float features[FEATURE_SIZE];

// RGB control
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

void indicateGestureColor(const char* label) {
  if (strcmp(label, "Z") == 0) setRGB(255, 0, 0);
  else if (strcmp(label, "O") == 0) setRGB(0, 255, 0);
  else if (strcmp(label, "V") == 0) setRGB(0, 0, 255);
  else setRGB(255, 255, 0); // unknown gesture: yellow
}

// Edge Impulse feature input
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
  memcpy(out_ptr, features + offset, length * sizeof(float));
  return 0;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(6, 7); // SDA, SCL

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  initRGB();
  setRGB(0, 0, 0);

  Serial.println("Initializing MPU6050...");
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("MPU6050 initialized. Press button to capture.");
}

void loop() {
  // Detect button press (falling edge)
  if (digitalRead(BUTTON_PIN) == LOW && !buttonPressed && !capturing) {
    buttonPressed = true;
    Serial.println("Button pressed. Starting gesture capture...");
    sample_count = 0;
    capturing = true;
    capture_start_time = millis();
    last_sample_time = millis();
  }

  if (digitalRead(BUTTON_PIN) == HIGH) {
    buttonPressed = false;
  }

  if (capturing) {
    capture_accelerometer_data();
  }
}

void capture_accelerometer_data() {
  if (millis() - last_sample_time >= SAMPLE_RATE_MS) {
    last_sample_time = millis();
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    if (sample_count < FEATURE_SIZE / 3) {
      int idx = sample_count * 3;
      features[idx] = a.acceleration.x;
      features[idx + 1] = a.acceleration.y;
      features[idx + 2] = a.acceleration.z;
      sample_count++;
    }

    if (millis() - capture_start_time >= CAPTURE_DURATION_MS) {
      capturing = false;
      Serial.println("Capture complete. Running inference...");
      run_inference();
    }
  }
}

void run_inference() {
  ei_impulse_result_t result = { 0 };
  signal_t features_signal;
  features_signal.total_length = FEATURE_SIZE;
  features_signal.get_data = &raw_feature_get_data;

  EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);
  if (res != EI_IMPULSE_OK) {
    Serial.print("Classifier error: "); Serial.println(res);
    return;
  }

  // Find highest score
  float max_val = 0.0;
  const char* predicted = "unknown";
  for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    if (result.classification[i].value > max_val) {
      max_val = result.classification[i].value;
      predicted = ei_classifier_inferencing_categories[i];
    }
  }

  Serial.print("Prediction: ");
  Serial.print(predicted);
  Serial.print(" (");
  Serial.print(max_val * 100);
  Serial.println("%)");

  indicateGestureColor(predicted);
}
