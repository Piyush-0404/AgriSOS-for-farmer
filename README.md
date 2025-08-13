# 🚨 AgriSOS – IoT Emergency Alert Device with Offline Keyword Detection

AgriSOS is an **IoT-based safety and alert system** that detects emergency voice keywords (like `"help"`) **offline** using an **Edge Impulse** ML model and sends real-time alerts (with GPS location) to a **Supabase** database.  
It is designed for **hackathons, field safety, and agricultural worker protection**.

---

## 📌 Features

- **Offline voice keyword detection** using **INMP441 I²S mic** + Edge Impulse ML model.
- **"Help"** keyword detection with high accuracy (custom-trained model).
- **GPS location tracking** via NEO-6M GPS module.
- **Cloud integration** using Supabase REST API.
- **ESP32 DevKit V1** as main microcontroller.
- **Fallback location** if GPS fix is not available.
- Works on **battery or USB power**.
- Ready for hackathon prototyping – minimal extra hardware required.

---

## 🛠 Hardware Used

| Component | Purpose | Pin Connections (ESP32 DevKit V1) |
|-----------|---------|------------------------------------|
| **ESP32 DevKit V1** | Main MCU | — |
| **INMP441 I²S Mic** | Audio capture for ML | VCC → 3V3, GND → GND, SCK → D14, WS → D32, SD → D33 |
| **NEO-6M GPS** | GPS location | VCC → 3V3, GND → GND, TX → D26, RX → D27 |
| **Li-ion Battery / USB** | Power | — |

---

## 📂 Project Structure

AgriSOS/
│
├── src/
│ ├── AgriSOS.ino # Main Arduino sketch
│ └── ei-voice-detection-arduino/ # Edge Impulse model library
│
├── README.md # Project documentation
├── LICENSE # License file (optional)
└── docs/ # Extra schematics & reference images

yaml
Copy
Edit

---

## ⚙️ Setup Instructions

### 1️⃣ Install Dependencies
- Install **Arduino IDE** (latest).
- Install ESP32 board package:  
  `Arduino IDE → Preferences → Additional Boards URLs → https://dl.espressif.com/dl/package_esp32_index.json`
- Install required libraries from Library Manager:
  - **TinyGPSPlus**
  - **ArduinoJson**
  - (WiFi and WiFiClientSecure are included in ESP32 core)

---

### 2️⃣ Get the ML Model
- Go to your **Edge Impulse project**.
- Export → **Arduino Library** → Download ZIP.
- In Arduino IDE: `Sketch → Include Library → Add .ZIP Library...` → select the downloaded ZIP.

---

### 3️⃣ Configure the Code
- Open `AgriSOS.ino`.
- Edit Wi-Fi credentials:
  ```cpp
  const char* WIFI_SSID     = "your_wifi_name";
  const char* WIFI_PASSWORD = "your_wifi_password";
Edit Supabase credentials:

cpp
Copy
Edit
const char* SUPABASE_HOST = "your-project.supabase.co";
const char* SUPABASE_ANON_KEY = "your_anon_key";
4️⃣ Wire the Hardware
INMP441 Mic
arduino
Copy
Edit
VCC → 3V3
GND → GND
SCK → D14
WS  → D32
SD  → D33
NEO-6M GPS
scss
Copy
Edit
VCC → 3V3
GND → GND
TX  → D26 (ESP32 RX1)
RX  → D27 (ESP32 TX1)
5️⃣ Upload and Test
Select ESP32 DevKit V1 from Tools → Board.

Select the correct COM port.

Upload the sketch.

Open Serial Monitor @ 115200 baud.

Speak "help" near the mic.

On detection above threshold → GPS + alert will be sent to Supabase.

📡 Supabase Table Schema
sql
Copy
Edit
CREATE TABLE emergency_alerts (
  id BIGSERIAL PRIMARY KEY,
  person_name TEXT NOT NULL,
  place_name TEXT NOT NULL,
  phone_number TEXT NOT NULL,
  threat_call TEXT NOT NULL CHECK (threat_call IN ('snake', 'fire', 'help', 'theft', 'animal')),
  gps_location JSONB,
  status TEXT DEFAULT 'new' CHECK (status IN ('new', 'active', 'resolved', 'false_alarm')),
  device_id TEXT,
  ml_confidence DECIMAL(3,2),
  detection_method TEXT,
  created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),
  updated_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
);
📸 Demo Flow
Device powers on, connects to Wi-Fi.

INMP441 listens continuously.

When "help" is detected above set threshold:

Fetch GPS coordinates (or use fallback).

POST alert JSON to Supabase.

Dashboard (frontend) displays alert in real-time.

🚀 Future Improvements
Add SIM800L GSM fallback for offline SMS alerts.

Support multiple keywords (fire, theft, etc.).

Waterproof and rugged enclosure for field deployment.

Battery charging and power management.

📜 License
This project is released under the MIT License – feel free to use and modify with attribution.

👨‍💻 Authors
Piyush Kumar – Project & hardware

Edge Impulse – ML platform

OpenAI ChatGPT – Code guidance
