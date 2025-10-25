┌─────────────────────────────────────────────────────────────────────┐
│                         IN THE AIR                                  │
│                                                                     │
│   ┌──────────────────────────────────────────────────────────────┐│
│   │ MAVIC 3 ENTERPRISE                                           ││
│   │                                                              ││
│   │  ┌────────────────┐                                         ││
│   │  │ Flight         │                                         ││
│   │  │ Controller     │                                         ││
│   │  │ (Drone System) │                                         ││
│   │  └────────┬───────┘                                         ││
│   │           │                                                 ││
│   │           │ E-Port Coaxial Cable                            ││
│   │           │                                                 ││
│   │           ↓                                                 ││
│   │  ┌──────────────────────────────────────────────────────┐  ││
│   │  │ E-Port Development Kit                               │  ││
│   │  │                                                      │  ││
│   │  │  ┌────────────────────────────────────────────────┐ │  ││
│   │  │  │ Raspberry Pi 4 Model B                         │ │  ││
│   │  │  │ - Powered by drone battery (via E-Port)        │ │  ││
│   │  │  │ - Running PSDK application                     │ │  ││
│   │  │  │ - Signal processing & triangulation           │ │  ││
│   │  │  │ - Flight instruction generation                │ │  ││
│   │  │  └───────────┬────────────────────────────────────┘ │  ││
│   │  │              │ GPIO Connection                       │  ││
│   │  │              ↓                                       │  ││
│   │  │  ┌────────────────────────────────────────────────┐ │  ││
│   │  │  │ Beacon Receiver (on breadboard)                │ │  ││
│   │  │  │ - 457 kHz avalanche beacon receiver            │ │  ││
│   │  │  │ - Signal strength measurement (RSSI)           │ │  ││
│   │  │  │ - Connected to Raspberry Pi GPIO               │ │  ││
│   │  │  └────────────────────────────────────────────────┘ │  ││
│   │  └──────────────────────────────────────────────────────┘  ││
│   │                          ↑                                  ││
│   │                          │ 457 kHz Radio Signal             ││
│   │                          │ (Wireless - up to 50m+ range)    ││
│   └──────────────────────────┼──────────────────────────────────┘│
│                              │                                    │
│                              │                                    │
└──────────────────────────────┼────────────────────────────────────┘
                               │
                               │ 457 kHz Distress Signal
                               │ (Emitted continuously)
                               │
                               │
┌──────────────────────────────┼────────────────────────────────────┐
│                   ON THE GROUND / IN SNOW                         │
│                              │                                    │
│                              ↓                                    │
│                    ┌──────────────────┐                          │
│                    │  TRANSPONDER     │                          │
│                    │  BEACON          │                          │
│                    │                  │                          │
│                    │ - 457 kHz TX     │                          │
│                    │ - Continuous     │                          │
│                    │   pulse pattern  │                          │
│                    │ - Standard       │                          │
│                    │   avalanche      │                          │
│                    │   beacon signal  │                          │
│                    │ - Battery        │                          │
│                    │   powered        │                          │
│                    │ - Worn by        │                          │
│                    │   person         │                          │
│                    └──────────────────┘                          │
│                                                                   │
│                    (Buried in snow or                            │
│                     carried by person in distress)               │
└───────────────────────────────────────────────────────────────────┘

                               ↕
                        OcuSync Wireless
                               ↕

┌───────────────────────────────────────────────────────────────────┐
│                     GROUND CONTROL STATION                        │
│                                                                   │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │ Mobile Application (MSDK)                                │   │
│  │ - DJI Pilot 2 app on phone/tablet/RC Pro                 │   │
│  │                                                           │   │
│  │ Displays:                                                │   │
│  │ ✓ Drone position (GPS)                                   │   │
│  │ ✓ Beacon detection alerts                                │   │
│  │ ✓ Signal strength readings (RSSI)                        │   │
│  │ ✓ Estimated beacon location (triangulation)              │   │
│  │ ✓ Search pattern progress                                │   │
│  │ ✓ Flight path adjustments                                │   │
│  │                                                           │   │
│  │ Controls:                                                │   │
│  │ ✓ Send commands to payload                               │   │
│  │ ✓ Adjust search parameters                               │   │
│  │ ✓ Manual override of flight instructions                 │   │
│  └──────────────────────────────────────────────────────────┘   │
│                                                                   │
│                    RESCUE OPERATOR LOCATION                       │
└───────────────────────────────────────────────────────────────────┘
```

---

## Signal Flow Diagram
```
┌─────────────────────────────────────────────────────────────────┐
│                    SIGNAL DETECTION FLOW                        │
└─────────────────────────────────────────────────────────────────┘

STEP 1: BEACON TRANSMISSION
┌──────────────────────┐
│ Transponder Beacon   │
│ (Person in distress) │
│                      │
│ Transmits:           │
│ - Frequency: 457kHz  │
│ - Pattern: Pulses    │
│ - Power: ~1-50mW     │
│ - Range: 50m+        │
└──────────┬───────────┘
           │
           │ 457 kHz Radio Waves
           │ (Omnidirectional)
           ↓
           
STEP 2: AIRBORNE RECEPTION
┌──────────────────────────────────────────┐
│ Beacon Receiver (on drone breadboard)    │
│                                          │
│ - Antenna receives 457 kHz signal        │
│ - Amplifies weak signal                  │
│ - Demodulates to get pulse pattern       │
│ - Measures signal strength (RSSI)        │
│ - Outputs to GPIO pins:                  │
│   • Digital: Beacon detected (HIGH/LOW)  │
│   • Analog: Signal strength value        │
└──────────┬───────────────────────────────┘
           │
           │ GPIO Signals
           │
           ↓
           
STEP 3: SIGNAL PROCESSING
┌──────────────────────────────────────────┐
│ Raspberry Pi 4 Model B                   │
│                                          │
│ 1. GPIO Input:                           │
│    - Read digital beacon detection pin   │
│    - Read analog RSSI value (ADC)        │
│                                          │
│ 2. Signal Processing:                    │
│    - Validate beacon pattern             │
│    - Filter noise/false positives        │
│    - Calculate signal strength           │
│    - Timestamp detection                 │
│    - Store GPS position of detection     │
│                                          │
│ 3. Triangulation:                        │
│    - Collect RSSI from multiple points   │
│    - Calculate distance estimates        │
│    - Triangulate beacon position         │
│                                          │
│ 4. Flight Instructions:                  │
│    - Generate waypoints toward beacon    │
│    - Adjust search pattern based on RSSI │
│    - Optimize flight path                │
└──────────┬───────────────────────────────┘
           │
           │ PSDK Data Transmission
           │
           ↓
           
STEP 4: COMMUNICATION TO GROUND
┌──────────────────────────────────────────┐
│ E-Port → Mavic 3 → OcuSync → Ground      │
│                                          │
│ Transmitted Data:                        │
│ {                                        │
│   "event": "BEACON_DETECTED",            │
│   "timestamp": 1234567890,               │
│   "drone_gps": {                         │
│     "lat": 51.0447,                      │
│     "lon": -114.0719,                    │
│     "alt": 150                           │
│   },                                     │
│   "signal_strength": -67,                │
│   "estimated_distance": 25.3,            │
│   "confidence": 0.87                     │
│ }                                        │
└──────────┬───────────────────────────────┘
           │
           │ Wireless Link
           │
           ↓
           
STEP 5: GROUND STATION DISPLAY
┌──────────────────────────────────────────┐
│ DJI Pilot 2 / Mobile App                 │
│                                          │
│ 🚨 BEACON ALERT!                         │
│ ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━   │
│                                          │
│ Detection Time: 14:23:45                 │
│ Signal Strength: -67 dBm                 │
│ Estimated Distance: 25.3 m               │
│ Confidence: 87%                          │
│                                          │
│ [MAP VIEW]                               │
│   🛩️ Drone Position                      │
│   📍 Estimated Beacon Location           │
│   ━━ Suggested Flight Path              │
│                                          │
│ [NAVIGATE TO BEACON] [ADJUST SEARCH]     │
└──────────────────────────────────────────┘

---

## Key Clarifications

### ✅ What we HAVE Working:
1. **Raspberry Pi 4 powered by drone** via E-Port (this is awesome!)
2. **PSDK configured and working** - you can send commands to drone (got serial number)
3. **Beacon receiver connected** to Raspberry Pi via GPIO on breadboard
4. **Python GPIO communication** with beacon works
5. **E-Port Development Kit** successfully bridges Raspberry Pi to Mavic 3

### 🔧 What You NEED to Complete:
From your notes section:
> "Create a single C/C++ application that leverages the DJI payload SDK and listens for signals from the beacon."

we want to simplyfy and Skip Mobile SDK Data flow for now and Use Existing Tools.

This is **exactly** what your Milestone 1 should focus on!

---

## Milestone 1: Prove Bidirectional Communication (Simplified)Goal:

### Goal: 
Prove that your Raspberry Pi can:

Send data through PSDK → Mavic 3 → somewhere we can see it
Receive data from outside → Mavic 3 → PSDK → Raspberry Pi
How to Test Without Mobile SDK:Option 1: Log-Based Testing (Simplest - Start Here!)Test Flow:
Raspberry Pi (sends message) 
    → PSDK logs it
    → E-Port transmits it
    → Mavic 3 receives it
    → Check Mavic 3 logs or DJI Assistant 2

---


## Corrected System Architecture

### **Physical Components:**
1. **Mavic 3 Enterprise** - The drone
2. **E-Port Development Kit** - DJI's adapter that:
   - Provides power to Raspberry Pi from drone battery
   - Provides serial communication interface
   - Connects via coaxial cable to Mavic 3
3. **Raspberry Pi 4 Model B** - Your computation platform running:
   - Full Linux OS
   - PSDK libraries (C/C++)
   - Signal processing code
   - Python scripts for beacon GPIO
4. **Breadboard with Beacon Receiver** - Connected to Pi's GPIO pins
5. **Avalanche Beacon** - Transmitting distress signal

### **Communication Flow:**
```
Avalanche Beacon (457 kHz signal)
         ↓
Beacon Receiver (on breadboard)
         ↓
GPIO pins on Raspberry Pi
         ↓
[YOUR C/C++ APPLICATION - This is what needs to be written!]
         ↓
PSDK Data Transmission API
         ↓
E-Port Development Kit (serial interface)
         ↓
Mavic 3 Flight Controller
         ↓
OcuSync Wireless Link
         ↓
DJI RC / DJI Pilot 2 App (on ground)
         ↓
Mobile SDK receives payload data
         ↓
Display on map / Log data