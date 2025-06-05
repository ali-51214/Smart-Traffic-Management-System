# 🚦 Smart Traffic Management System using YOLO and LoRa

This capstone project aims to build an AI-powered smart traffic control system. The system detects vehicles using a YOLO-based object detection model via a laptop camera, processes the data, and transmits car counts through LoRa communication to an ESP32 microcontroller. The ESP32 dynamically adjusts traffic signal lights based on traffic density, improving road efficiency.

---

## 📌 Features

- Real-time vehicle detection using YOLOv5
- Lane-wise car counting
- Wireless communication using LoRa (RYLR modules)
- ESP32-based traffic light control logic
- Hardware emulation using LEDs
- Data update every 3 seconds

---

## 🧰 Tech Stack

- **Computer Vision**: YOLOv5 (Python, OpenCV)
- **Microcontroller**: ESP32 (C++)
- **Communication**: LoRa RYLR modules using AT commands
- **Data Transmission**: UART Serial
- **Development Tools**: Arduino IDE, Python 3.x

---

## 🔌 Hardware Used

- ESP32 WROVER board
- Reyax RYLR896 / RYLR400 LoRa Modules
- USB-to-UART converter (for laptop connection)
- LED indicators (Red/Green lights for traffic signal)
- Breadboard, jumper wires

---

## ⚙️ Setup Instructions

1. Clone this repository:
    ```bash
    git clone https://github.com/yourusername/smart-traffic-system.git
    cd smart-traffic-system
    ```

2. **Laptop Side (Python + YOLO):**
    - Install dependencies: `pip install -r requirements.txt`
    - Run: `python detect_and_send.py`

3. **ESP32 Side (C++ / Arduino):**
    - Load the `traffic_control.ino` sketch to ESP32
    - Ensure LoRa is configured:
      ```
      AT+MODE=0
      AT+ADDRESS=10
      AT+NETWORKID=6
      AT+BAND=915000000
      ```

---

## 📊 How It Works

- The laptop camera detects cars using YOLOv5.
- Python script counts vehicles in each lane and sends the count every 3 seconds via LoRa.
- ESP32 receives the data and applies signal control logic.
- The traffic light (LEDs) changes based on traffic density.

---

## 📸 Demo

*Add video or screenshots here*

---

## 👤 Author

**Syed Ali**  
Master's Student – Cyber-Physical Systems  
Northeastern University, Toronto

---

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
