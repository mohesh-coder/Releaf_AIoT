# train_and_export_tree.py
# -----------------------
# Input CSV format:
# time, temperature, humidity, apparent_temp
#
# Produces:
# - A trained Decision Tree
# -----------------------

import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.tree import DecisionTreeClassifier, export_text, _tree
from sklearn.metrics import classification_report
import joblib

CSV_FILE = "forecast_dataset.csv"   # <-- your file name here

# -----------------------
# 1. Load your real dataset
# -----------------------
df = pd.read_csv(CSV_FILE)

# Ensure correct types
df['time'] = pd.to_datetime(df['time'])

# -----------------------
# 2. Feature engineering
# -----------------------
df['hour'] = df['time'].dt.hour
df['is_day'] = df['hour'].apply(lambda h: 1 if 6 <= h <= 18 else 0)

# Create a simple WBGT-like proxy (not exact!)
df['wbgt_est'] = 0.7 * df['temperature'] + 0.2 * (df['humidity'] / 100 * df['temperature']) + 0.1 * (df['apparent_temp'])

# Create a 1-hour forecast delta feature
df['next_temp'] = df['temperature'].shift(-1)
df['temp_delta'] = df['next_temp'] - df['temperature']
df['temp_delta'] = df['temp_delta'].fillna(0)   # last row

# -----------------------
# 3. Create a danger label
# (modify as you like later)
# -----------------------
df['label'] = df.apply(
    lambda r: 1 if (r['wbgt_est'] > 30 or r['apparent_temp'] > 35) else 0,
    axis=1
)

# -----------------------
# 4. Select features
# -----------------------
FEATURES = [
    'temperature',
    'humidity',
    'apparent_temp',
    'wbgt_est',
    'hour',
    'temp_delta'
]

X = df[FEATURES]
y = df['label']

# -----------------------
# 5. Train/test split
# -----------------------
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42, stratify=y
)

# -----------------------
# 6. Train small Decision Tree
# -----------------------
clf = DecisionTreeClassifier(max_depth=4, min_samples_leaf=10, random_state=42)
clf.fit(X_train, y_train)

print("\n=== MODEL PERFORMANCE (TEST) ===")
print(classification_report(y_test, clf.predict(X_test)))

print("\n=== DECISION TREE (text) ===")
print(export_text(clf, feature_names=FEATURES))

# Save Python model (optional)
joblib.dump(clf, "tree_model.joblib")
# -----------------------
print("=== DONE ===")
