/* ESP32-C6 sketch for ReLeaf System (REDACTED)
   - Sensitive strings (WiFi credentials, exact location, broker id) replaced with placeholders.
   - Fill the placeholders before deploying.
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <PubSubClient.h>

// ===== CONFIGURATION =====
// Replace placeholders below with your real values before use:
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"
#define STATION_ID "station001" // safe generic id; replace if needed

// Replace with approximate coordinates or placeholders (avoid sharing exact coordinates publicly)
#define LAT 0.0     // YOUR_LATITUDE
#define LON 0.0     // YOUR_LONGITUDE

#define WARN 5  // Danger LED Pin
#define NOR 6   // Normal/Safe LED Pin

#define DHTPIN 4       // GPIO where DHT11 data pin is connected
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQTT Broker Settings - use a placeholder host (replace with your broker)
const char* mqtt_server = "YOUR_MQTT_BROKER"; // e.g. "broker.example.com"
const int mqtt_port = 1883; // replace if different

// Actuator Pins
const int MIST_PIN = 15; 
const unsigned long MIN_ACT_INTERVAL_MS = 5000UL; 
unsigned long last_activation_ms = 0;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Forward declarations
void activate_mist(unsigned long duration_s);

// --- MQTT CALLBACK (Handle triggers from Web App) ---
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  
  if (String(topic) == String("station/") + STATION_ID + "/command") {
    if (msg == "activate") {
      Serial.println(">>> REMOTE TRIGGER RECEIVED <<<");
      activate_mist(30); 
    }
  }
}

// --- WIFI & MQTT SETUP ---
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int tries=0;
  while (WiFi.status() != WL_CONNECTED && tries < 40) {
    delay(250);
    Serial.print(".");
    tries++;
  }
  if (WiFi.status()==WL_CONNECTED) Serial.println("connected.");
}

void connectMQTT() {
  if (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect(STATION_ID)) {
      Serial.println("connected");
      // Subscribe to command topic
      String cmdTopic = String("station/") + STATION_ID + "/command";
      mqttClient.subscribe(cmdTopic.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

// --- OPEN METEO FETCH ---
bool fetch_open_meteo(float &om_temp, float &om_rh, float &om_apparent) {
  if(WiFi.status() != WL_CONNECTED) return false;
  
  String url = String("https://api.open-meteo.com/v1/forecast?latitude=") + String(LAT) +
               "&longitude=" + String(LON) + "&hourly=temperature_2m,relativehumidity_2m,apparent_temperature&timezone=auto";
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode != 200) {
    http.end();
    return false;
  }
  String payload = http.getString();
  http.end();

  DynamicJsonDocument doc(2048); 
  DeserializationError err = deserializeJson(doc, payload);
  if (err) return false;
  
  om_temp = doc["hourly"]["temperature_2m"][0] | 0.0;
  om_rh = doc["hourly"]["relativehumidity_2m"][0] | 0.0;
  om_apparent = doc["hourly"]["apparent_temperature"][0] | 0.0;
  return true;
}

// --- MAIN SETUP ---
void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(MIST_PIN, OUTPUT);
  pinMode(WARN, OUTPUT);
  pinMode(NOR, OUTPUT);
  
  digitalWrite(MIST_PIN, LOW);
  digitalWrite(WARN, LOW);
  digitalWrite(NOR, HIGH); // Default to Normal ON at startup
  
  dht.begin();
  connectWiFi();
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback); 
  
  // Warm up fetch (non-fatal if fails)
  float t, r, a;
  fetch_open_meteo(t, r, a); 
}

// --- JSON HELPERS ---
String make_telemetry_json(float local_temp, float local_rh, float om_temp, float om_rh, float om_app) {
  StaticJsonDocument<512> doc;
  doc["station_id"] = STATION_ID;
  doc["local_temp"] = local_temp;
  doc["local_rh"] = local_rh;
  doc["om_temp"] = om_temp;
  doc["om_rh"] = om_rh;
  doc["om_apparent"] = om_app;
  doc["ts"] = (uint32_t) (millis()/1000);
  String out;
  serializeJson(doc, out);
  return out;
}

void activate_mist(unsigned long duration_s) {
  unsigned long now = millis();
  if (now - last_activation_ms < MIN_ACT_INTERVAL_MS) {
    Serial.println("Skipping activation (duty cycle)");
    return;
  }
  last_activation_ms = now;
  Serial.println(">>> MIST ON <<<");
  digitalWrite(MIST_PIN, HIGH);
  
  // Non-blocking wait to keep MQTT alive
  unsigned long startRun = millis();
  while(millis() - startRun < (duration_s * 1000UL)) {
    mqttClient.loop(); 
    delay(100);
  }
  
  digitalWrite(MIST_PIN, LOW);
  Serial.println(">>> MIST OFF <<<");
}

// --- MAIN LOOP ---
void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop(); 

  // 1. Read Data
  float local_h = dht.readHumidity();
  float local_t = dht.readTemperature();
  if (isnan(local_t)) local_t = 0.0;
  if (isnan(local_h)) local_h = 0.0;

  float om_temp=0, om_rh=0, om_app=0;
  fetch_open_meteo(om_temp, om_rh, om_app);

  // 2. Send to Python (Serial)
  String tel = make_telemetry_json(local_t, local_h, om_temp, om_rh, om_app);
  Serial.println(tel); 

  // 3. Wait for ML Decision
  unsigned long start = millis();
  bool got_reply = false;
  String line = "";
  
  // Wait loop (10 seconds)
  while (millis() - start < 10000UL) { 
    mqttClient.loop(); 
    
    if (Serial.available()) {
      char c = (char)Serial.read();
      if (c == '\n') {
        DynamicJsonDocument rdoc(256);
        auto err = deserializeJson(rdoc, line);
        if (!err) {
          int decision = rdoc["decision"] | 0;
          float prob = rdoc["prob"] | 0.0;
          int duration = rdoc["duration"] | 60;
          
          Serial.print("Received ML Decision: "); Serial.println(decision);

          // --- LED INDICATOR LOGIC ---
          if (decision == 1) {
            // DANGER DETECTED
            digitalWrite(WARN, HIGH);
            digitalWrite(NOR, LOW);
          } else {
            // SAFE / NORMAL
            digitalWrite(WARN, LOW);
            digitalWrite(NOR, HIGH);
          }
          // ---------------------------

          // 3A. PUBLISH DECISION TO WEB APP
          if (mqttClient.connected()) {
             StaticJsonDocument<512> mqttDoc;
             mqttDoc["station_id"] = STATION_ID;
             mqttDoc["status"] = (decision == 1) ? "DANGER" : "SAFE";
             mqttDoc["decision"] = decision;
             mqttDoc["probability"] = prob;
             // Send features so dashboard updates
             mqttDoc["features"]["local_temp"] = local_t;
             mqttDoc["features"]["local_rh"] = local_h;
             mqttDoc["features"]["om_temp"] = om_temp;
             mqttDoc["features"]["om_rh"] = om_rh;
             mqttDoc["features"]["om_apparent"] = om_app;
             
             String mqttPayload;
             serializeJson(mqttDoc, mqttPayload);
             String topic = String("station/") + STATION_ID + "/ml_result";
             mqttClient.publish(topic.c_str(), mqttPayload.c_str());
          }

          if (decision == 1) activate_mist(duration);
        }
        got_reply = true;
        break; 
      } else {
        line += c;
      }
    }
  }

  // 4. FALLBACK: Publish to Web App even if Python script didn't reply
  if (!got_reply) {
    Serial.println("ML Timeout. Publishing raw telemetry to Web App.");
    if (mqttClient.connected()) {
       StaticJsonDocument<512> mqttDoc;
       mqttDoc["station_id"] = STATION_ID;
       // Missing "decision" field -> Web App shows "MONITORING"
       mqttDoc["local_temp"] = local_t;
       mqttDoc["local_rh"] = local_h;
       mqttDoc["om_temp"] = om_temp;
       mqttDoc["om_rh"] = om_rh;
       mqttDoc["om_apparent"] = om_app;
       
       String mqttPayload;
       serializeJson(mqttDoc, mqttPayload);
       String topic = String("station/") + STATION_ID + "/ml_result";
       mqttClient.publish(topic.c_str(), mqttPayload.c_str());
    }
  }

  // Wait before next cycle (keep MQTT alive)
  for(int i=0; i<300; i++) { // 30 seconds
    mqttClient.loop();
    delay(100);
  }
}
