# TECHIN515_Lab4: Magic Wand - Real-Time Gesture Recognition on ESP32-C3

This project implements a gesture-based "magic wand" using an ESP32-C3 microcontroller and the Edge Impulse ML platform. It recognizes three distinct hand gestures — **Z**, **O**, and **V** — using MPU6050 motion data, and triggers RGB LED color changes in real-time as visual feedback.

---

## 🔧 Hardware Setup

- **ESP32-C3 Xiao** (Edge-compatible MCU)
- **MPU6050** accelerometer (I2C)
- **Push Button** for gesture trigger
- **RGB LED** (Common cathode)
- **Battery-powered prototyping board**

| Component | GPIO |
|----------|------|
| Button   | GPIO9 |
| RGB Red  | GPIO21 |
| RGB Green| GPIO20 |
| RGB Blue | GPIO8 |
| I2C SDA  | GPIO6 |
| I2C SCL  | GPIO7 |

---

## 🧠 Gesture Definitions

| Gesture | Label | Description |
|---------|-------|-------------|
| Z       | `Z`   | Fire Bolt (1HP attack, 1MP) |
| O       | `O`   | Reflect Shield (reflects Fire Bolt, 2MP) |
| V       | `V`   | Healing Spell (+1HP, 2MP) |

---

## 📁 Repository Structure
├── README.md
├── gesture_capture.ino # Arduino sketch for data collection
├── wand.ino # Inference sketch w/ RGB feedback
├── process_gesture_data.py # Python script to record sensor data via Serial
├── dataset.zip # Z/O/V gesture samples (compressed)
├── ei-auriazh-project-1-arduino-1.0.1.zip # Quantized model for ESP32


---

## 📊 Model Training (Edge Impulse)

- DSP Block: **Spectral Features**
- Window size: 1000ms, Stride: 200ms
- Neural Network: 2-layer dense (20 + 10 neurons)
- Learning rate: 0.0005, Epochs: 30
- Accuracy: ~95% on test set
- Model exported as Quantized (int8) Arduino library

---

## 🎯 System Behavior

1. Press the **button** to start gesture capture
2. MPU6050 collects 1-second motion data
3. Model classifies the gesture on-device
4. RGB LED color reflects prediction:
   - 🔴 Z → Red
   - 🟢 O → Green
   - 🔵 V → Blue
5. Inference result also printed via Serial Monitor

---

## 📦 Dataset

- 100+ samples collected per gesture (Z/O/V)
- Data collected at 100Hz using `gesture_capture.ino` + `process_gesture_data.py`
- Raw dataset stored as CSV files, zipped: `dataset.zip`
- Each sample: 3-axis acceleration x 100 frames

---

## ▶️ Demo Instructions

1. Upload `wand.ino` to ESP32
2. Connect to Serial Monitor (115200 baud)
3. Press button and perform one gesture
4. Observe RGB color and serial prediction output

---

## 🧾 Notes

- Model runs in real-time on-device at ~100ms latency
- RGB controlled via PWM (`analogWrite`)
- Inference runs using Edge Impulse SDK
- Project tested with both USB and battery-powered setups
- 
---

Built for TECHIN515 @ University of Washington | Spring 2025  
By: Auria Zhang

