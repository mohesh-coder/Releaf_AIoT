import serial
import time
import json
import threading
import requests
import joblib
import os
import pandas as pd  # Added to fix feature name warning
import numpy as np
from sklearn.linear_model import LogisticRegression

# --- CONFIGURATION ---
# ON WINDOWS: Use "COM3", "COM4", etc. Check Device Manager > Ports.
# ON LINUX/MAC: Use "/dev/ttyUSB0", "/dev/ttyACM0", etc.
SERIAL_PORT = "COM6"  

BAUD = 115200
MODEL_PATH = "tree_model.joblib"
DECISION_THRESHOLD = 0.8

# Feature names must match exactly what was used during training in Model.py
FEATURE_NAMES = ['temperature', 'humidity', 'apparent_temp', 'wbgt_est', 'hour', 'temp_delta']

def ensure_model():
    if os.path.exists(MODEL_PATH):
        print("Loading model from", MODEL_PATH)
        payload = joblib.load(MODEL_PATH)
        if isinstance(payload, dict) and 'model' in payload:
            return payload
        else:
            return {'model': payload, 'scaler': None, 'numeric_cols': None}
    else:
        print("Model not found")
        return None

def serial_listener(port, model_payload):
    try:
        ser = serial.Serial(port, BAUD, timeout=1)
        print(f"Opened serial port {port} successfully.")
    except serial.SerialException as e:
        print(f"CRITICAL ERROR: Could not open serial port '{port}'.")
        print(f"Details: {e}")
        return

    while True:
        try:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if not line:
                    continue
                
                # FIX 1: Ignore debug messages from MCU (lines not starting with '{')
                if not line.startswith('{'):
                    # print(f"MCU Debug: {line}") # Optional: verify debug messages
                    continue

                print("RX:", line)
                try:
                    msg = json.loads(line)
                except Exception as e:
                    print(f"Invalid JSON parsing error: {e}")
                    continue

                # Use telemetry fields
                station_id = msg.get('station_id', 'station')
                local_t = msg.get('local_temp', None)
                local_rh = msg.get('local_rh', None)
                om_temp = msg.get('om_temp', None)
                om_rh = msg.get('om_rh', None)
                om_app = msg.get('om_apparent', None)

                # Compute WBGT proxy and delta
                wbgt = None
                temp_delta = 0.0
                if om_temp is not None and om_app is not None:
                    wbgt = 0.7 * om_temp + 0.2 * (om_rh/100.0 * om_temp) + 0.1 * om_app
                    temp_delta = 0.0

                # Fallback logic if sensors/API fail
                temperature = om_temp if om_temp not in (None, 0) else (local_t if local_t is not None else 25.0)
                humidity = om_rh if om_rh not in (None, 0) else (local_rh if local_rh is not None else 50.0)
                apparent = om_app if om_app not in (None, 0) else temperature
                hour = time.localtime().tm_hour
                wbgt_val = wbgt if wbgt is not None else (0.7*temperature + 0.2*(humidity/100.0 * temperature) + 0.1*apparent)
                
                # Raw features list
                features_list = [temperature, humidity, apparent, wbgt_val, hour, temp_delta]

                # Run model
                if model_payload:
                    model = model_payload['model']
                    
                    # FIX 2: Create a DataFrame with column names to silence the warning
                    X = pd.DataFrame([features_list], columns=FEATURE_NAMES)
                    
                    if hasattr(model, "predict_proba"):
                        prob = float(model.predict_proba(X)[0,1])
                    else:
                        prob = float(model.predict(X)[0])
                    
                    decision = 1 if prob >= DECISION_THRESHOLD else 0
                else:
                    print("No model loaded, defaulting decision to 0")
                    decision = 0
                    prob = 0.0

                # Reply to MCU over serial
                reply = {"decision": decision, "prob": round(prob, 3), "duration": 60}
                ser.write((json.dumps(reply) + "\n").encode('utf-8'))
                print("Sent reply to MCU:", reply)

            else:
                time.sleep(0.1)

        except Exception as e:
            print("Serial loop error:", e)
            time.sleep(1)

if __name__ == "__main__":
    model_payload = ensure_model()
    serial_listener(SERIAL_PORT, model_payload)