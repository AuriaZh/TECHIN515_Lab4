#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;

// ---- GPIO 配置 ----
#define BUTTON_PIN 9      // 你焊的按钮（D9 = GPIO9）
#define RED_PIN    21     // RGB 红色通道（D6 = GPIO21）
#define GREEN_PIN  20     // RGB 绿色通道（D7 = GPIO20）
#define BLUE_PIN   8      // RGB 蓝色通道（D8 = GPIO8）

// ---- 状态变量 ----
long last_sample_millis = 0;
bool capture = false;
bool alreadyPressed = false;
unsigned long capture_start_time = 0;
const unsigned long CAPTURE_DURATION = 1000; // 1 秒

String currentGesture = "Z"; // 默认手势是 Z（可以通过串口切换为 O / V）

// ---- RGB 控制 ----
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
  if (g == "Z")      setRGB(255, 0, 0);   // 红：Z → Fire Bolt
  else if (g == "O") setRGB(0, 255, 0);   // 绿：O → Reflect Shield
  else if (g == "V") setRGB(0, 0, 255);   // 蓝：V → Healing
  else               setRGB(255, 255, 0); // 黄：未知标签
}

// ---- 初始化 ----
void setup(void) {
  Serial.begin(115200);
  delay(2000); // 串口稳定时间

  // I2C 初始化（你的板子 SDA = GPIO6, SCL = GPIO7）
  Wire.begin(6, 7);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  initRGB();
  setRGB(0, 0, 0); // 默认熄灭 RGB

  // MPU6050 初始化
  while (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    delay(500);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("MPU6050 ready.");
}

// ---- 数据采集 ----
void capture_data() {
  if ((millis() - last_sample_millis) >= 10) { // 100Hz 采样
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
      setRGB(0, 255, 0);  // 绿灯表示采集完成（可改为熄灯）
      Serial.print("\n\n\n\n");
      Serial.println("Capture complete (1 second)");
    }
  }
}

// ---- 主循环 ----
void loop() {
  // 按钮触发逻辑（代替输入 'o'）
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && !alreadyPressed && !capture) {
    alreadyPressed = true;

    Serial.println("-,-,-"); // 通知 Python 脚本开始采集
    Serial.println("Starting capture (will run for 1 second)");
    capture_start_time = millis();
    capture = true;

    indicateGestureColor(currentGesture);  // 显示当前手势颜色
  }

  if (buttonState == HIGH) {
    alreadyPressed = false;
  }

  // 进行数据采集
  if (capture) {
    capture_data();
  }

  // 支持串口输入 Z / O / V 切换标签
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'Z' || c == 'O' || c == 'V') {
      currentGesture = String(c);
      Serial.print("Gesture label changed to: ");
      Serial.println(currentGesture);
    }
  }
}

