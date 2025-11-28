import requests
import pandas as pd

lat = 0.0 # replace with desired latitude
lon = 0.0 # replace with desired longitude
start_date = "2025-10-01" # replace with desired start date (YYYY-MM-DD)
end_date = "2025-10-31" # replace with desired end date (YYYY-MM-DD)
url = f"https://archive-api.open-meteo.com/v1/archive?latitude={lat}&longitude={lon}&start_date={start_date}&end_date={end_date}&hourly=temperature_2m,relativehumidity_2m,apparent_temperature,dewpoint_2m&timezone=auto"

res = requests.get(url)
data = res.json()

hours = data['hourly']['time']
temp = data['hourly']['temperature_2m']
rh = data['hourly']['relativehumidity_2m']
appt = data['hourly']['apparent_temperature']

df = pd.DataFrame({
    "time": hours,
    "temperature": temp,
    "humidity": rh,
    "apparent_temp": appt
})

print(df.head())
df.to_csv("forecast_dataset.csv", index=False)
