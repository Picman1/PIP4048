# Copilot Instructions for PIP4048

## Project Overview

PIP4048 is an **ESP32-based controller** for the PIP4048 inverter. It monitors inverter status, manages power prioritization based on time of day and battery state, and exposes control via MQTT. The project uses **PlatformIO** (not Arduino IDE).

## Build, Test, and Run

### Building and Uploading
- **Build**: `pio run` (or `platformio run`)
- **Upload to ESP32**: `pio run -t upload`
- **Monitor serial output**: `pio device monitor --baud 115200`
- **Build + Upload in one command**: `pio run -t upload && pio device monitor --baud 115200`

### PlatformIO Configuration
- **platformio.ini** defines the build environment (esp32dev, 921600 upload speed, 115200 monitor speed)
- **Board**: ESP32 Development Module
- **Key dependencies**: PubSubClient (MQTT), ArduinoJson, esp_ping

## Architecture

### Data Flow
1. **ESP32 connects to WiFi** (static IP 192.168.1.204) during setup
2. **Connects to MQTT broker** (192.168.1.203:1883)
3. **Periodically queries inverter** via UART/Serial2 (every 10 seconds) with CRC-validated commands
4. **Publishes inverter data and logs** to MQTT topics
5. **Listens for commands** on MQTT subscribe topic

### Communication Layers

#### Serial2 Protocol (Inverter Communication)
- **Baud rate**: 2400
- **RX pin**: GPIO16 | **TX pin**: GPIO17
- **Message format**: ASCII commands with CRC16-XMODEM checksum
- **All commands** are predefined as String constants in `Inverter.h` with embedded CRC
- **Example**: `QPIGS` (Query status) → response like `(236.8 49.8 230.0...` → parse fields by space

#### CRC16-XMODEM
- **Implementation**: `CRCService.h::crcXmodem()`
- **Validation**: `isValidStringMessageWithCRC()` checks last 2 bytes before `\r`
- **Every command** sent via `sendCommand()` adds CRC and expects validated response

#### MQTT Topics
- **Publish**: `inverter/data` (query responses), `inverter/log` (status messages)
- **Subscribe**: `inverter/cmd` (incoming commands like "QPIGS", "POP00", "Restart")
- **Functions**: `mqtt_data()` and `mqtt_log()` handle publishing with fallback to Serial

### Time-Based Logic
- **NTP Sync**: Every hour at minute 0:00, or after WiFi reconnect (via `syncNtpTime()`)
- **Daytime Flag**: Determined by sunrise/sunset API call; updated hourly and daily
- **Daily Logic**:
  - Sunrise/sunset fetched once per day via HTTP API call to sunrise-sunset.org
  - Battery charge management rules based on:
    - Battery percentage (QPIGS field 10)
    - Current mode (QMOD: "B"=Solar/Battery, "L"=Utility)
    - Daytime status (isDaytime flag)
  - Daily restart at midnight (00:00)

### Power Prioritization Rules
Located in `main.cpp::sendAndReceive()`:
- **Battery ≤70%** + Solar/Battery mode + Grid voltage available → switch to utility first (POP00)
- **Battery 100%** + Utility mode + Daytime → switch to solar first (POP01)
- **High load (>5000VA)** → switch to utility first (POP00)

### Global State Variables (Settings.h)
- `lastSerialSendTime`, `serialSendInterval` (10 sec) → periodic inverter queries
- `lastApiCallDay`, `lastCheckedHour_daytimeFlag` → track daily/hourly updates
- `lastRestartDay` → ensure daily restart only once per day
- `isDaytime` (extern) → updated by `updateDaytimeFlag()`
- WiFi static IP config, NTP servers, timezone (South Africa: SAST-2)

## Key Conventions

### Command Structure (Inverter.h)
- **Predefined commands** are stored as String constants: `QPIGS`, `QMOD`, `POP00`, `POP01`, etc.
- Each has binary representation with embedded CRC (e.g., `"\x51\x50\x49\x47\x53\xB7\xA9\x0D"`)
- **Never construct commands directly**—always use the predefined constants
- **Command format**: `sendCommand(String cmd)` → adds CRC, sends via Serial2, reads response with timeout

### Response Parsing
- **Field-based extraction**: `readField(response, fieldIndex)` splits by space and returns int
- **Direct substring**: For simple responses like QMOD `(B` or `(L`, check first 2 chars
- **Always validate**: Check response length, CRC validity, and (NAK failures before parsing

### Error Handling Patterns
- **Serial timeout**: 2 seconds default in `readSerial2Response()`
- **CRC validation failure** → return empty string, don't process
- **Response length checks** → prevent buffer overruns and malformed data
- **Log via MQTT**: Use `mqtt_log()` for warnings; errors are broadcast to the broker

### Header Organization
- **Inverter.h**: All inverter command definitions and QPIGS field breakdown
- **CRCService.h**: CRC calculation and validation; includes debugging output
- **MQTT.h**: Publish helpers; reconnection logic in `reconnectMQTT()`
- **Settings.h**: All configuration (WiFi, MQTT, NTP, timings, geographic coords)
- **Setups.h**: WiFi connection routine with timeout and restart on failure
- **TimerService.h**: Time formatting, NTP sync, HTTP API calls, daytime logic

### Code Style
- **String + char concatenation** is heavily used for logging
- **Emoji in logs** for visual feedback (✅, ❌, ⚠️)
- **Serial.println() + mqtt_log()** pattern for dual logging (Serial + MQTT)
- **Guard against nullptr** and buffer overflows in parsing
- **Comments include response examples** (e.g., QPIGS breakdown in Inverter.h)

### Testing & Debugging
- **Serial monitor**: Always run `pio device monitor` to watch commands and responses
- **CRC debugging**: `crcXmodem()` prints calculated CRC to Serial in HEX
- **MQTT testing**: Use `mosquitto_pub/sub` or any MQTT client to test topics
- **Manual command test**: Send "QPIGS" or other commands via MQTT `inverter/cmd` topic

## Secrets and Configuration
- **Secrets.h**: Defines `ssid`, `password`, `mqtt_user`, `mqtt_password` (create locally, not in git)
- **Settings.h**: MQTT server IP, NTP servers, timezone, geographic coordinates
- **platformio.ini**: Upload/monitor speeds, serial port (may need update for your machine)

## Common Tasks

### Adding a New Inverter Command
1. Define command in `Inverter.h` with CRC: `String NEWCMD = "..."`
2. Document response format as comment (see QPIGS example)
3. Use `sendCommand(NEWCMD)` in main logic
4. Parse response with `readField()` or substring extraction
5. Publish results via `mqtt_data()`

### Modifying Power Prioritization Logic
- Edit `sendAndReceive()` in `main.cpp` for the decision tree
- Test with battery percentage and time-of-day edge cases
- Use MQTT to monitor behavior: subscribe to `inverter/log` and `inverter/data`

### Changing Timing or Intervals
- `serialSendInterval` (Settings.h): Inverter query frequency (default 10 sec)
- `ntpSyncInterval`: NTP re-sync period (default 1 hour)
- Daily API call: Triggered when `tm_mday` changes
- Daily restart: Triggered when hour is 0 and day has changed
