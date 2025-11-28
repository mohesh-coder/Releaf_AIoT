# ReLeaf: AI & IoT Smart Urban Cooling System 🌿💧

ReLeaf saves lives in overheating cities by deploying smart IoT misting stations that automatically activate when AI predicts dangerous heat stress. It combines real-time sensing, edge machine learning, and generative AI to provide instant cooling in public spaces.

---

## 📖 Table of Contents
- [The Problem](#-the-problem)
- [The Solution](#-the-solution)
- [Key Features](#-key-features)
- [System Architecture](#-system-architecture)
- [Hardware & Tech Stack](#-hardware--tech-stack)
- [Installation & Setup](#-installation--setup)
- [Usage](#-usage)
- [License](#-license)

---

## 🔥 The Problem
In the heart of the city, the summer sun isn't just bright—it's brutal. The Urban Heat Island effect turns bus stops and markets into ovens, causing heat exhaustion and heatstroke. A recent tragedy in Karur highlighted this when extreme heat triggered a deadly stampede. Traditional cooling fans running on timers are inefficient and unresponsive to actual danger levels.

---

## 💡 The Solution
ReLeaf is an intelligent micro-climate cooling network. Unlike "dumb" appliances, a ReLeaf station listens to the environment. It uses local sensors and an Edge AI brain to calculate the **Wet Bulb Globe Temperature (WBGT)**—the true measure of heat stress.

When the danger threshold is crossed, it automatically triggers cooling mist. It also connects to a city-wide dashboard where **Google Gemini AI** analyzes the data to provide actionable health advice to officials.

---

## ✨ Key Features
- **Hybrid Sensing:** Combines local DHT11 sensor data with Open-Meteo hourly forecasts for high accuracy.  
- **Edge AI Inference:** Uses a Scikit-Learn Decision Tree model running on a local gateway to predict “Danger” states based on heat stress metrics.  
- **Smart Actuation:** Automatically triggers the relay (misting pump) only when ML probability exceeds **80%** or local temp exceeds **40°C** (Safety Override).  
- **Generative AI Analyst:** Integrated Gemini API analyzes raw telemetry to create human-readable heat-risk reports.  
- **Real-time Dashboard:** Responsive Glassmorphism web app visualizing live data via MQTT.

---

## 🏗 System Architecture
The workflow follows a **Sense → Predict → Act → Inform** loop:

1. **Node (ESP32):** Reads Temp/Humidity & fetches Weather API data.  
2. **Gateway (Python):** Receives data via Serial, runs ML Model, returns Decision (Safe/Danger).  
3. **Actuator:** ESP32 triggers Relay if Danger is detected.  
4. **Cloud:** Status is published to EMQX MQTT Broker.  
5. **Interface:** Web App displays status and invokes Gemini AI for analysis.

---

## 🛠 Hardware & Tech Stack

### **Hardware**
- Controller: ESP32 Development Board (Wi-Fi + BLE)  
- Sensors: DHT11 (Temperature & Humidity)  
- Actuators: 5V Relay Module (Controls Water Pump/Solenoid)  
- Indicators: Red LED (Danger), Green LED (Safe)

### **Software**
- Firmware: C++ (Arduino IDE)  
- ML Gateway: Python 3.10, Scikit-Learn, Pandas, Joblib  
- Communication: MQTT (Paho), Serial (JSON)  
- Frontend: HTML5, Tailwind CSS, JavaScript  
- APIs: Open-Meteo (Weather), Google Gemini (GenAI)

---

## 🚀 Installation & Setup

### **1. Hardware Connections**

| ESP32 Pin | Component          |
|-----------|--------------------|
| GPIO 4    | DHT11 Data         |
| GPIO 15   | Relay / Mist Trigger |
| GPIO 5    | Red LED (Danger)   |
| GPIO 6    | Green LED (Safe)   |
| 3V3/5V    | VCC                 |
| GND       | GND                 |

---

### **2. Firmware (ESP32)**

1. Open `DeviceCode/DeviceCode.ino` in Arduino IDE.  
2. Install libraries:  
   - DHT sensor library  
   - ArduinoJson  
   - PubSubClient  
3. Update `WIFI_SSID` and `WIFI_PASS`.  
4. Upload to ESP32.

---

### **3. Edge Gateway (Python)**

Install dependencies:

```bash
pip install pyserial pandas scikit-learn joblib paho-mqtt
```
Update SERIAL_PORT in Inference.py, then run:

```
python Inference.py
```
### **4. Web Dashboard**

1. Open index.html in any modern browser.

2. Add your Gemini API Key inside the script configuration for AI features.

## 🎮 Usage

1. Start the System: Power up ESP32 + run Python inference engine.

2. Monitor: Watch terminal for "Safe" or "Danger" predictions.

3. Simulate Heat: Breathe on DHT11 to increase humidity/temperature.

## Expected Response:

- 🔴 Red LED turns ON

- ⚡ Relay clicks ON (misting activates)

- 📊 Dashboard pulses red

- 🤖 “Analyze Conditions” → Generates real-time heat-risk report using Gemini

## 📜 License

This project is open-source and available under the MIT License.

### Built with 💚 for a cooler, safer world.