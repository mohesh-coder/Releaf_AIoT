ReLeaf: AI & IoT Smart Urban Cooling System 🌿💧

ReLeaf saves lives in overheating cities by deploying smart IoT misting stations that automatically activate when AI predicts dangerous heat stress. It combines real-time sensing, edge machine learning, and generative AI to provide instant cooling in public spaces.

📖 Table of Contents

The Problem

The Solution

Key Features

System Architecture

Hardware & Tech Stack

Installation & Setup

Usage

License

🔥 The Problem

In the heart of the city, the summer sun isn't just bright—it's brutal. The Urban Heat Island effect turns bus stops and markets into ovens, causing heat exhaustion and heatstroke. A recent tragedy in Karur highlighted this when extreme heat triggered a deadly stampede. Traditional cooling fans running on timers are inefficient and unresponsive to actual danger levels.

💡 The Solution

ReLeaf is an intelligent micro-climate cooling network. Unlike "dumb" appliances, a ReLeaf station listens to the environment. It uses local sensors and an Edge AI brain to calculate the Wet Bulb Globe Temperature (WBGT)—the true measure of heat stress.

When the danger threshold is crossed, it automatically triggers cooling mist. It also connects to a city-wide dashboard where Google Gemini AI analyzes the data to provide actionable health advice to officials.

✨ Key Features

Hybrid Sensing: Combines local DHT11 sensor data with Open-Meteo hourly forecasts for high accuracy.

Edge AI Inference: Uses a Scikit-Learn Decision Tree model running on a local gateway to predict "Danger" states based on heat stress metrics.

Smart Actuation: Automatically triggers the relay (misting pump) only when ML probability exceeds 80% or local temp exceeds 40°C (Safety Override).

Generative AI Analyst: Integrated Gemini API analyzes raw telemetry to generate human-readable health reports on the dashboard.

Real-time Dashboard: A responsive Glassmorphism web app visualizing live data via MQTT.

🏗 System Architecture

The workflow follows a Sense → Predict → Act → Inform loop:

Node (ESP32): Reads Temp/Humidity & fetches Weather API data.

Gateway (Python): Receives data via Serial, runs ML Model, returns Decision (Safe/Danger).

Actuator: ESP32 triggers Relay if Danger is detected.

Cloud: Status is published to EMQX MQTT Broker.

Interface: Web App displays status and invokes Gemini AI for analysis.

🛠 Hardware & Tech Stack

Hardware

Controller: ESP32 Development Board (Wi-Fi + BLE)

Sensors: DHT11 (Temperature & Humidity)

Actuators: 5V Relay Module (Controls Water Pump/Solenoid)

Indicators: Red LED (Danger), Green LED (Safe)

Software

Firmware: C++ (Arduino IDE)

ML Gateway: Python 3.10, Scikit-Learn, Pandas, Joblib

Communication: MQTT (Paho), Serial (JSON)

Frontend: HTML5, Tailwind CSS, JavaScript

APIs: Open-Meteo (Weather), Google Gemini (GenAI)

🚀 Installation & Setup

1. Hardware Connections

ESP32 Pin

Component

GPIO 4

DHT11 Data

GPIO 15

Relay / Mist Trigger

GPIO 5

Red LED (Danger)

GPIO 6

Green LED (Safe)

3V3/5V

VCC

GND

GND

2. Firmware (ESP32)

Open DeviceCode/DeviceCode.ino in Arduino IDE.

Install libraries: DHT sensor library, ArduinoJson, PubSubClient.

Update WIFI_SSID and WIFI_PASS.

Upload to ESP32.

3. Edge Gateway (Python)

Install dependencies:

pip install pyserial pandas scikit-learn joblib paho-mqtt


Connect ESP32 to USB. Check your COM port (e.g., COM3 or /dev/ttyUSB0).

Update SERIAL_PORT in Inference.py.

Run the inference engine:

python Inference.py


4. Web Dashboard

Open index.html in any modern web browser.

(Optional) Add your Google Gemini API Key in the .env configuration section within the HTML script for AI features.

🎮 Usage

Start the System: Power up the ESP32 and run the Python script.

Monitor: Watch the CLI for "Safe" or "Danger" predictions.

Simulate Heat: Breathe on the DHT11 sensor to raise humidity/temp.

Observe Response:

Red LED turns ON.

Relay clicks ON (Mist activates).

Web Dashboard pulses Red.

AI Analysis: Click "Analyze Conditions" on the dashboard to get a real-time health report from Gemini.

📜 License

This project is open-source and available under the MIT License.

Built with 💚 by Mohesh for a cooler, safer world.