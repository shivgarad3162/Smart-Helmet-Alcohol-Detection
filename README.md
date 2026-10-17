# Smart Helmet – Alcohol Detection System

## 1. Project Overview

The **Smart Helmet – Alcohol Detection System** is an Arduino-based safety prototype designed to prevent a bike from starting when alcohol is detected above a programmed threshold or when the helmet-side distance condition is not satisfied.

The system uses two Arduino Nano boards to communicate wirelessly. The helmet-side unit reads the MQ-3 alcohol sensor and ultrasonic sensor, while the bike-side unit receives the status and controls a relay connected to a low-voltage DC motor.

## 2. Objectives

- Detect alcohol using the MQ-3 sensor.
- Check the helmet-side distance condition using an ultrasonic sensor.
- Transmit sensor status wirelessly using RF modules.
- Prevent motor starting when alcohol is detected or start permission is unavailable.
- Display the system status on a 16×2 LCD.

## 3. Components Required

| Component | Purpose |
|---|---|
| 2 × Arduino Nano | Sensor processing and motor control |
| MQ-3 Alcohol Sensor | Detects alcohol-related vapour |
| HC-SR04 Ultrasonic Sensor | Measures distance for the helmet-side condition |
| RF Transmitter and Receiver | Wireless communication |
| 16×2 LCD | Displays system status |
| Relay Module | Controls the DC motor circuit |
| Buzzer | Gives an audible warning |
| DC Motor | Demonstrates starting and stopping |
| Battery and Switch | Provide power and manual switching |
| Jumper Wires | Connect the components |

## 4. System Architecture

The project has two main units.

### A. Helmet-Side Unit

The helmet-side Arduino Nano:

- Reads the MQ-3 sensor value.
- Checks the ultrasonic sensor distance.
- Activates the buzzer when alcohol is first detected.
- Sends the appropriate RF message to the bike-side unit.

### B. Bike-Side Unit

The bike-side Arduino Nano:

- Receives RF messages.
- Checks whether the message is recent.
- Applies the alcohol lockout and start-permission conditions.
- Controls the relay and DC motor.
- Displays the current status on the LCD.

### System Flow

```text
HELMET-SIDE UNIT
       |
       v
   MQ-3 Sensor
       |
       v
 Ultrasonic Sensor
       |
       v
  Arduino Nano
       |
       v
 RF Transmitter
       |
       | Wireless Communication
       v
 RF Receiver
       |
       v
  Arduino Nano
       |
       +------> 16×2 LCD
       |
       v
   Relay Module
       |
       v
     DC Motor
```

## 5. Working Principle

1. The MQ-3 sensor measures an analogue value related to alcohol vapour.
2. The Arduino compares the reading with the programmed threshold.
3. The ultrasonic sensor checks whether an object is within the configured distance.
4. The helmet-side Arduino sends one of three messages: `ALLOW`, `NOHELMET`, or `ALCOHOL`.
5. The bike-side Arduino processes the received message and checks the signal timeout and lockout conditions.
6. The relay permits or prevents the prototype DC motor from running.
7. The LCD displays the current system status.

**Important:** The MQ-3 threshold is an experimental sensor reading, not a calibrated blood-alcohol concentration. The ultrasonic sensor checks distance and does not independently confirm that a helmet is being worn.

## 6. Circuit Connections

### A. Helmet-Side Connections

| Component Pin | Arduino Nano Pin |
|---|---|
| MQ-3 AO | A0 |
| MQ-3 VCC | 5V |
| MQ-3 GND | GND |
| HC-SR04 TRIG | D4 |
| HC-SR04 ECHO | D5 |
| Buzzer Signal | D8 |
| RF Transmitter DATA | D12 |
| RF Transmitter VCC | As per module rating |
| RF Transmitter GND | GND |

### B. Bike-Side Connections

#### RF Receiver

| Receiver Pin | Arduino Nano Pin |
|---|---|
| DATA | D2 |
| VCC | 5V, if supported |
| GND | GND |

#### 16×2 LCD

| LCD Pin | Arduino Nano Connection |
|---|---|
| 1 - VSS | GND |
| 2 - VDD | 5V |
| 3 - VO | Contrast adjustment |
| 4 - RS | D3 |
| 5 - RW | GND |
| 6 - E | D4 |
| 7–10 - D0 to D3 | Not connected |
| 11 - D4 | D5 |
| 12 - D5 | D6 |
| 13 - D6 | D8 |
| 14 - D7 | D9 |
| 15 - LED A | As specified by the LCD module |
| 16 - LED K | GND |

#### Relay Module

| Relay Pin | Connection |
|---|---|
| IN | Arduino Nano D7 |
| VCC | 5V, if supported |
| GND | Arduino Nano GND |

### C. Battery, Switch and DC Motor

- Battery positive → ON/OFF switch → Arduino Nano VIN, within the board's input rating.
- Battery negative → Arduino Nano GND.
- Motor supply positive → Relay COM.
- Relay NO → Motor positive.
- Motor negative → Motor supply negative.
- Relay NC → Leave unconnected.

**Safety:** Never power the motor directly from an Arduino GPIO pin. Verify the relay's voltage and current ratings, including motor starting current. Test with a low-voltage DC motor prototype; do not connect this unvalidated circuit directly to a real motorcycle ignition.

## 7. RF Communication and Motor Control

The helmet-side Arduino sends the following messages.

| RF Message | Meaning | Bike-Side Action |
|---|---|---|
| `ALLOW` | Alcohol reading is below the programmed threshold and the distance condition is satisfied | Grants start permission, subject to other conditions |
| `NOHELMET` | The distance condition is not satisfied while alcohol is below the threshold | Keeps the motor OFF |
| `ALCOHOL` | Alcohol reading exceeds the programmed threshold | Keeps the motor OFF and activates lockout |

### Motor Start Conditions

The motor can start only when:

1. A recent, recognized RF message has been received.
2. An `ALLOW` message grants permission.
3. The lockout condition does not prevent starting.

The bike-side code uses a **2-second RF signal timeout**. If no recognized message is received within this period, the signal is considered stale and the motor remains OFF.

The configured alcohol lockout period is **30 seconds**. Start permission must still be available after the lockout expires.

## 8. LCD Status Display

| Condition | LCD Message |
|---|---|
| System starting | `Smart Helmet` / `Starting...` |
| Receiver ready | `Bike Receiver` / `Ready` |
| Start permitted | `Helmet: YES` / `Alcohol: SAFE` |
| Helmet distance condition not satisfied | `Helmet: NO` / `Motor: OFF` |
| Alcohol lockout active | `ALCOHOL DETECTED` / `Wait: XX sec` |
| RF signal lost | `RF Signal Lost` / `Motor: OFF` |

The exact messages depend on the conditions handled by the bike-side program.

## 9. Software and Code Files

The project contains two Arduino programs.

### `helmet_transmitter.ino`

This program:

- Reads the MQ-3 sensor on A0.
- Uses a programmed threshold of 25 for experimental testing.
- Reads the HC-SR04 ultrasonic sensor.
- Controls the buzzer.
- Sends `ALLOW`, `NOHELMET`, or `ALCOHOL` using RF communication.

### `bike_receiver.ino`

This program:

- Receives RF messages.
- Checks whether the signal is recent.
- Applies start-permission and lockout logic.
- Controls the relay using D7.
- Displays status on the 16×2 LCD.

**Threshold note:** The value 25 is a prototype testing value, not a universal or medically meaningful alcohol threshold. Sensor warm-up and calibration are required for meaningful testing.

## 10. How to Run the Project

1. Install the Arduino IDE.
2. Install the RadioHead library.
3. Connect the helmet-side components according to Section 6.
4. Upload `helmet_transmitter.ino` to the helmet-side Arduino Nano.
5. Connect the bike-side components according to Section 6.
6. Upload `bike_receiver.ino` to the bike-side Arduino Nano.
7. Power both units using suitable supplies.
8. Open the Serial Monitor at **9600 baud** to observe sensor readings and messages.
9. Test the RF communication, LCD, relay, and DC motor separately before testing the complete prototype.

## 11. Limitations

- The MQ-3 sensor requires warm-up and calibration.
- The ultrasonic sensor checks distance rather than directly confirming helmet wear.
- RF communication can be affected by distance and interference.
- The threshold is experimental and may vary with sensor conditions.
- The prototype has not been validated for controlling a real motorcycle.

## 12. Future Improvements

- Calibrate the alcohol sensor using an appropriate test procedure.
- Improve helmet-wearing detection.
- Improve RF communication reliability.
- Add a complete circuit diagram and wiring photographs.
- Record test results under different sensor and communication conditions.
- Conduct proper electrical and safety testing before considering real-vehicle integration.

## 13. Conclusion

The Smart Helmet – Alcohol Detection System demonstrates a prototype safety mechanism using two Arduino Nano boards, an MQ-3 alcohol sensor, an ultrasonic sensor, RF communication, a buzzer, an LCD, and a relay-controlled DC motor.

The project illustrates how sensor readings and wireless messages can be used to control motor-start permission under programmed conditions. Further calibration, testing, and safety validation are necessary before any real-vehicle application.

## 14. Repository Files

| File | Description |
|---|---|
| `helmet_transmitter.ino` | Helmet-side sensing and RF transmission |
| `bike_receiver.ino` | Bike-side RF reception, LCD, and relay control |
| `README.md` | Project overview, connections, and instructions |

## 15. Disclaimer

This project is an educational prototype. It is not a certified alcohol detection device or a validated motorcycle safety system. Test it only with a suitable low-voltage DC motor setup.
