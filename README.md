## Smart Library: IoT Library Management System

An IoT-powered library system that automates book tracking, shelf management, self-checkout, and occupancy monitoring using Arduino, ESP32, RFID, and infrared sensors, with real-time cloud logging to Google Sheets and a live Streamlit dashboard.

---

### Demo

[YouTube Video](https://youtu.be/sXnw_eqRRkI?si=FbnIAxDYKZOEBER0)

---

### Overview

The Smart Library consists of three physical subsystems, each built on its own Arduino, unified by an ESP32 that bridges them to the cloud via Wi-Fi. A Streamlit web dashboard provides students and librarians with real-time visibility into book availability, shelf activity, and library occupancy.

**Gate System**: Detects entry and exit using a PIR sensor and ultrasonic distance sensor. A servo motor simulates a physical gate, LEDs indicate gate status, and an LCD displays the live occupancy count. Direction (entry vs exit) is determined by analysing how distance changes over time rather than relying on fixed sensor positions.

**Smart Shelf**: Four TCRT5000 infrared sensors (one per book slot) detect when a book is placed or removed. On insertion, the corresponding MFRC522 RFID reader scans the book's sticker and compares the UID against the expected shelf/slot in the cloud inventory. If the book is in the wrong slot, the system flags it as a `MISMATCH` — alerting the librarian in real time.

**Checkout System**: A self-service station where students scan a book's RFID sticker to check it out. The system communicates bidirectionally through the ESP32: it sends the UID to Google Sheets (marking the book as `CHECKED_OUT`), retrieves the book's title and author, and displays them on the LCD as confirmation.

**IoT Dashboard**: A multi-page Streamlit web app that polls Google Sheets every 10 seconds, displaying book availability, library occupancy, peak hour analytics, and a password-protected librarian panel with mismatch alerts and detailed shelf logs.

### Architecture

```
┌─────────────┐    Serial     ┌────────┐    Wi-Fi / HTTP     ┌───────────────┐
│ Gate Arduino │──────────────→│ Python │───── Webhook ──────→│ Google Sheets │
│  (PIR, US,  │               │ Script │                     │  (Gate DB)    │
│  Servo, LCD)│               └────────┘                     └───────┬───────┘
└─────────────┘                                                     │
                                                                    ▼
┌──────────────┐    UART      ┌────────┐    Wi-Fi / HTTP     ┌───────────────┐
│ Shelf Arduino│─────────────→│  ESP32 │───── Webhook ──────→│ Google Sheets │
│  (TCRT5000,  │              │        │                     │ (Library DB)  │
│   2x RFID)   │              │        │                     └───────┬───────┘
└──────────────┘              │        │                             │
                              │        │◄── JSON response ───────────┘
┌──────────────┐    UART      │        │         │
│Checkout      │─────────────→│        │         ▼
│ Arduino      │◄─────────────│        │  ┌──────────────┐
│ (RFID, LCD)  │  (book info) └────────┘  │  Streamlit   │
└──────────────┘                          │  Dashboard   │
                                          └──────────────┘
```

### Project Structure

```
Smart_Library/
├── Arduino/
│   ├── gatesystem.ino          # Gate entry/exit detection, servo, LCD, LEDs
│   ├── smartshelf.ino          # TCRT5000 + RFID shelf monitoring
│   └── checkoutsystem.ino      # RFID self-checkout with LCD display
│
├── ESP32_DEVKIT_V1/
│   └── esp32code.ino           # Central hub - bridges both Arduinos to Google Sheets via Wi-Fi
│
├── Javascript/
│   ├── LibraryDatabase.js      # Google Apps Script webhook for inventory + logs (shelf & checkout)
│   └── GatewayScript.js        # Google Apps Script webhook for gate entry/exit events
│
├── Pyserial_Comms/
│   └── gatesystem.py           # Serial bridge - reads gate Arduino events, POSTs to Google Sheets
│
└── Streamlit/
    ├── dashboard.py             # Main dashboard - newsletter, navigation, librarian login
    └── Pages/
        ├── book_availability.py       # Live book availability table (colour-coded)
        ├── library_availability.py    # Occupancy display + peak hour bar charts
        └── librarian_dashboard.py     # Password-protected admin panel with mismatch alerts
```

### Hardware Components

| Component | Qty | Role |
|-----------|-----|------|
| Arduino UNO R3 | 3 | Microcontroller for each subsystem |
| ESP32 DEVKIT V1 | 1 | Wi-Fi hub — connects shelf + checkout to cloud |
| MFRC522 RFID Reader | 3 | Scans 13.56 MHz RFID stickers on books |
| 13.56 MHz RFID Stickers | 4 | Attached to books, each mapped to inventory by UID |
| TCRT5000 IR Sensor | 4 | Detects book presence/absence per shelf slot |
| PIR Motion Sensor | 1 | Detects motion at the gate |
| HC-SR04 Ultrasonic Sensor | 1 | Measures distance for entry/exit direction logic |
| Servo Motor | 1 | Simulates gate barrier |
| LCD Display (I2C) | 2 | Status display for gate and checkout |
| LEDs (Red + Green) | 2 | Gate open/closed indicators |

### Cloud & Data

| Google Sheet | Purpose |
|-------------|---------|
| **GateSystemDatabase** | Logs entry/exit events with timestamps, tracks live occupancy count |
| **LibraryDatabase**: Inventory | Book metadata (UID, title, author, genre, shelf, slot, status) |
| **LibraryDatabase**: Log | Timestamped event log for all shelf and checkout activity |

Data flows from Arduinos → ESP32/Python → Google Apps Script webhooks (HTTP POST/GET) → Google Sheets. The Streamlit dashboard pulls from Sheets via CSV export URLs with 10-second auto-refresh.

### Installation & Setup

#### Prerequisites

- Python 3.x
- Arduino IDE
- [CP210x USB Driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) (for ESP32 UART)
- [Espressif Arduino-ESP32 board package](https://github.com/espressif/arduino-esp32) (add to Arduino IDE)

#### Arduino Libraries (install via Library Manager)

- `MFRC522`
- `LiquidCrystal_I2C`
- `Servo`
- `ArduinoJson` (ESP32 only)

#### Python Dependencies

```bash
pip install streamlit streamlit-autorefresh requests pyserial pandas numpy matplotlib plotly
```

#### Running the Gate System

1. Connect the gate Arduino to your computer via USB.
2. Upload `gatesystem.ino` in Arduino IDE (select the correct COM port).
3. In a terminal, run the serial bridge:

```bash
cd Pyserial_Comms
python gatesystem.py
```

> Note: Update `COM_PORT` in `gatesystem.py` to match your system.

#### Running the Smart Shelf & Checkout System

1. Connect both the shelf and checkout Arduinos to power.
2. Connect the ESP32 to your computer via the data cable.
3. In Arduino IDE, upload `esp32code.ino` to the ESP32.
   - **Important:** Disconnect the TX→RX wires from the Arduinos before flashing the ESP32, then reconnect after upload.
   - Update the `ssid` and `password` in the code to match your Wi-Fi network (must be 2.4 GHz).
4. Once uploaded, the ESP32 connects to Wi-Fi and begins relaying events to Google Sheets automatically.

#### Running the IoT Dashboard

```bash
cd Streamlit
streamlit run dashboard.py
```

The dashboard opens in your browser. Sub-pages (book availability, library availability, librarian dashboard) are accessible via navigation buttons or the sidebar. Librarian login credentials: `admin` / `1234`.

#### Viewing the Cloud Database

- [GateSystemDatabase](https://docs.google.com/spreadsheets/d/16Sxj_9a2VlhHgwZPhsQ5AC0hwujbgwMiR8YOlTME3_w/edit?usp=sharing)
- [LibraryDatabase](https://docs.google.com/spreadsheets/d/17lmgOYsNALd3Q7lwNWvqmzuSnGsaE6nQch-l9HRbapQ/edit?usp=sharing)

### Book Status Reference

| Status | Meaning |
|--------|---------|
| `IN` | Book is on the correct shelf and slot |
| `OUT` | Book has been removed from the shelf |
| `MISMATCH` | Book was placed in the wrong shelf/slot |
| `CHECKED_OUT` | Book has been borrowed via the checkout system |

---

Built as a group coursework project for CST2590 — Internet of Things at Middlesex University Dubai (Autumn 2025).
