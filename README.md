# 🔥 BLE Jammer using ESP32 and NRF24 🔥

---

![🚀 BLE Jammer]

---

## 🌟 Overview
This project is an **educational-only** BLE jammer that disrupts Bluetooth Low Energy (BLE) communication using the ESP32 microcontroller and NRF24L01 modules. It employs multiple SPI buses (VSPI and HSPI) to enhance jamming efficiency by simultaneously transmitting on different channels.

> **🚨 Disclaimer**: This project is for educational purposes only. It is intended to demonstrate the vulnerabilities of wireless communication systems. Any misuse or application in unauthorized environments is strictly prohibited.

---

## ✨ Features
- 🔄 **Dual SPI Operation**: Uses both HSPI and VSPI buses for simultaneous transmission.
- ⚡ **Channel Hopping**: Random or incrementally shifting channels to improve effectiveness.
- 🖥️ **OLED Display**: Displays status messages and operational feedback.
- 🕹️ **Debounced Toggle Switch**: Controls between fixed and random channel modes.
- 🔋 **Efficient Power Handling**: Reduces power usage by disabling unused peripherals.

---

## 🛠️ Components
- 🧠 **ESP32 Development Board**
- 📡 **NRF24L01+ Radio Modules (x2)**
- 📺 **SSD1306 OLED Display (128x32)**
- 🔘 **Push Button / Toggle Switch**

---

## 🔗 Wiring
### OLED Connection
| Pin | Function | ESP32 Pin |
|-----|----------|-----------|
| SDA | I2C Data | D21       |
| SCL | I2C Clock| D22       |

### NRF24L01 SPI Pin Configuration
| SPI Bus | Function | NRF24 Pin | ESP32 Pin |
|---------|----------|-----------|-----------|
| HSPI    | CE       | 26        |           |
|         | CS       | 15        |           |
|         | SCK      | 14        |           |
|         | MISO     | 12        |           |
|         | MOSI     | 13        |           |
| VSPI    | CE       | 4         |           |
|         | CS       | 2         |           |
|         | SCK      | 18        |           |
|         | MISO     | 19        |           |
|         | MOSI     | 23        |           |

---

## 📦 Software Dependencies
- **Arduino IDE**
- Libraries:
  - `RF24` by TMRh20
  - `ezButton`
  - `U8x8lib` for OLED

---

## 🚀 Installation and Setup
1. **Install Required Libraries**:
   - In Arduino IDE, go to `Sketch > Include Library > Manage Libraries` and search for the listed libraries.
2. **Connect Components** as per the wiring table.
3. **Upload the Code**:
   - Copy the provided code into the Arduino IDE.
   - Select the correct board (ESP32 Dev Module) and COM port.
   - Click on **Upload**.

---

## 🎮 Usage
1. **Power On** the device.
2. The OLED displays:
   - **Initialization Messages**
   - **Status of VSPI and HSPI Jammers**
3. **Toggle Switch Modes**:
   - **Fixed Channel Mode**: Increments and decrements channels within set limits.
   - **Random Channel Mode**: Randomizes channel selection and delay.
4. **Monitor Serial Output** for additional status messages and debug logs.

---

## 🖥️ OLED Interface Example
```
BLE JAMMER 
...............
EducationalOnly
```

---

## ⚙️ Functions
### `one()`
- Randomly selects a channel for both SPI buses and introduces a randomized delay.

### `two()`
- Increments and decrements the channel numbers, flipping direction at predefined limits.

---

## 🛠️ Customizations
- **Channel Range**: Adjust `ch` and `ch1` limits.
- **SPI Speed**: Modify `RF24` initialization to change SPI clock speed.
- **Display Text**: Modify OLED messages for personalized feedback.

---

## 🛑 Troubleshooting
| 🛑 Issue                        | 💡 Solution                                              |
|----------------------------------|---------------------------------------------------------|
| **No OLED Output**               | Check I2C connections and ensure the correct address.   |
| **Jammer Not Starting**          | Verify SPI wiring for NRF24 modules.                    |
| **WiFi/Bluetooth Not Disabled**  | Ensure `esp_wifi_stop()` and `esp_bt_controller_deinit()` are called. |

---

## 🏷️ License
This project is licensed under the **MIT License**.

---

## 🌍 Contributing
Pull requests are welcome! Please open an issue first to discuss any changes.

---

✨ **Enjoy Exploring Wireless Security with Responsibility!**

