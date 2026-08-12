# 💊 MediTrack — IoT Based Smart Medication System

![Platform](https://img.shields.io/badge/Board-ESP32-blue)
![Language](https://img.shields.io/badge/Language-Embedded%20C-orange)
![Type](https://img.shields.io/badge/Type-IoT%20Project-green)

An IoT-based smart medication reminder and stock-tracking system built on ESP32 — helps prevent missed doses and keeps track of how much medicine is left.

---

## 📌 About

A lot of people, especially elderly patients, either forget to take their medicine on time or run out of stock without realizing it. MediTrack tries to solve both problems using a simple embedded setup — a **load sensor** to track how much medicine is left, an **RTC module** to keep accurate time for reminders, and an **LCD** to show real-time status, all controlled by an **ESP32**.

The system continuously monitors medicine weight and current time, and gives automated alerts when it's time to take a dose or when stock is running low.

---

## 🧩 How It Works

1. **RTC module** keeps track of real time, independent of power resets
2. At scheduled medication times, the system triggers an **alert** (buzzer/LED — depending on setup)
3. **Load sensor** continuously measures the weight of the medicine container to estimate remaining stock
4. **LCD display** shows real-time info — current time, next dose, and stock level
5. ESP32 handles all the sensor data processing, timing logic, and display updates together

---

## 🛠️ Components Used

| Component | Purpose |
|---|---|
| ESP32 | Main microcontroller — handles logic & communication |
| RTC Module (DS3231) | Keeps accurate real-time clock for scheduling |
| Load Sensor + HX711 | Measures medicine container weight |
| LCD Display (I2C) | Shows time, reminders, and stock status |
| Buzzer/LED (optional) | Alerts for medication time |

---

## ⚙️ Key Concepts Involved

- Interfacing multiple sensors (RTC + Load Sensor) with a single microcontroller
- Real-time scheduling logic using RTC timestamps
- Analog-to-digital sensor data processing (load cell via HX711)
- I2C communication for LCD display
- Basic embedded control logic to trigger alerts based on conditions

---

## ▶️ How to Run

1. Wire up the ESP32 with the RTC module, load sensor (via HX711), and LCD as per I2C/SPI connections
2. Open the project in **Arduino IDE**
3. Install required libraries: `RTClib`, `HX711`, `LiquidCrystal_I2C`
4. Upload `src/meditrack.ino` to the ESP32
5. Set the current time via RTC (one-time setup) and place the medicine container on the sensor
6. The LCD will show live status, and alerts will trigger automatically at scheduled times

---

## 💡 What I Learned

- How to combine multiple sensors and make them work together on one microcontroller
- Real-time scheduling using RTC instead of just software delays
- Reading and calibrating load cell data through the HX711 amplifier
- Displaying dynamic real-time data on an I2C LCD
- Thinking through a real-world problem and designing a practical embedded solution for it

---

## 🚧 Next Steps

- [ ] Add WiFi connectivity to send alerts to a mobile app
- [ ] Add multiple medicine slot tracking instead of a single container
- [ ] Log historical dose data for better tracking

---

## 👤 About Me

**MD ZAID ADEEB**
B.Tech ECE student, learning embedded systems & IoT

📎 [LinkedIn](https://linkedin.com/in/md-zaid-adeeb-38b667287) · 📧 md.zaidadeeb2003@gmail.com

---

### 📄 License
MIT License — free to use, just keep my name in the credits.
