# Smart Poultry V3 - ESP32 IoT Environment Monitoring & Control System

![ESP32](https://img.shields.io/badge/ESP32-Ready-blue) ![ESP-IDF](https://img.shields.io/badge/ESP-IDF-v5.x-green) ![License](https://img.shields.io/badge/License-MIT-yellow)

Smart Poultry V3 is an advanced ESP32-based IoT system designed for Tunisian Mediterranean climate conditions. The system provides real-time environmental monitoring and automated control for poultry houses, ensuring optimal conditions for bird health and productivity.

## Table of Contents

- [Features](#features)
- [Tunisian Mediterranean Climate Standards](#tunisian-mediterranean-climate-standards)
- [Hardware Requirements](#hardware-requirements)
- [Installation & Building](#installation--building)
- [Configuration](#configuration)
- [Project Structure](#project-structure)
- [License](#license)

## Features

### Sensors
- **Temperature & Humidity**: DHT22 sensor for ambient environment monitoring
- **Water Temperature**: DS18B20 waterproof temperature sensor
- **Air Quality**: MQ135 (NH3, NOx, CO2), MQ2 (smoke/LPG), MQ7 (CO) gas sensors
- **Light Intensity**: LDR (Light Dependent Resistor) for ambient light measurement
- **Water Level**: Ultrasonic water level sensor for drinker monitoring
- **Sound Level**: Sound sensor for noise monitoring and distress detection

### Actuators
- **Climate Control**: Heater, Cooling Pad, Exhaust Fan
- **Feeding System**: Automated Feeder
- **Water System**: Water Pump
- **Lighting**: LED Light system with dimming control
- **Alert System**: Alarm for emergency notifications

### Communication
- **WiFi Manager**: Connect to local WiFi networks
- **MQTT Client**: Publish sensor data and receive control commands
- **OTA Updates**: Over-the-Air firmware updates

### Control Systems
- **Decision Algorithm**: Intelligent environment control based on thresholds
- **PID Controller**: Precise temperature control
- **Rules Engine**: Configurable automation rules
- **Emergency Protocol**: Automatic response to critical conditions
- **Energy Optimizer**: Efficient power management

## Tunisian Mediterranean Climate Standards

This project is specifically designed for Tunisian Mediterranean climate conditions:

| Parameter | Summer (May-Oct) | Winter (Nov-Apr) | Optimal for Poultry |
|-----------|-----------------|------------------|---------------------|
| Temperature | 25-40°C | 5-20°C | 18-24°C |
| Humidity | 40-70% | 60-85% | 50-70% |
| Ammonia (NH3) | <25 ppm | <25 ppm | <10 ppm |
| CO2 | <3000 ppm | <3000 ppm | <1000 ppm |

### Climate Zones Supported
- **Coastal (Sfax, Sousse, Tunis)**: High humidity, moderate temperatures
- **Interior (Kairouan, Sbeitla)**: Extreme temperatures, low humidity
- **South (Gabès, Douz)**: Hot arid conditions

### Default Thresholds
```
Temperature:    Min: 18°C, Max: 28°C, Optimal: 22°C
Humidity:       Min: 40%, Max: 75%, Optimal: 60%
NH3 (MQ135):    Max: 25 ppm
CO (MQ7):       Max: 50 ppm
Smoke (MQ2):    Max: 100 ppm
Light:          Min: 10 lux (night), Max: 500 lux (day)
Water Level:    Min: 20%, Max: 90%
Sound:          Max: 85 dB
```

## Hardware Requirements

### Main Board
- ESP32-WROOM-32 or ESP32-WROOM-32U (Recommended: 4MB Flash)

### Sensors
| Sensor | Pin | Notes |
|--------|-----|-------|
| DHT22 | GPIO4 | Temperature & Humidity |
| DS18B20 | GPIO5 | Water temperature (needs 4.7kΩ pull-up) |
| MQ135 | ADC1_CH0 (GPIO36) | Air quality |
| MQ2 | ADC1_CH1 (GPIO39) | Smoke detection |
| MQ7 | ADC1_CH2 (GPIO34) | Carbon monoxide |
| LDR | ADC1_CH3 (GPIO35) | Light intensity |
| HC-SR04 Trig | GPIO12 | Water level |
| HC-SR04 Echo | GPIO14 | Water level |
| Sound Sensor | ADC1_CH4 (GPIO32) | Noise level |

### Actuators
| Actuator | Pin | Notes |
|----------|-----|-------|
| Heater | GPIO25 | Relay controlled |
| Cooling Pad | GPIO26 | Relay controlled |
| Exhaust Fan | GPIO27 | Relay controlled |
| Feeder | GPIO18 | Servo motor |
| Water Pump | GPIO19 | Relay controlled |
| LED Light | GPIO21 | PWM controlled |
| Alarm | GPIO22 | Buzzer/Alarm |

### Power Requirements
- 5V/2A power supply recommended
- Consider solar + battery backup for remote installations

## Installation & Building

### Prerequisites

1. **ESP-IDF Environment**
   ```bash
   # Install ESP-IDF (v5.x recommended)
   # Follow: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html
   
   # On Windows, after installation:
   %userprofile%\esp\esp-idf\install.bat
   %userprofile%\esp\esp-idf\export.bat
   ```

2. **Git** (for version control)

### Build Steps

```bash
# Clone the repository
git clone https://github.com/your-repo/smart-poultry-v3.git
cd smart-poultry-v3

# Set up ESP-IDF environment
# Windows:
%userprofile%\esp\esp-idf\export.bat
# Linux/Mac:
. $HOME/esp/esp-idf/export.sh

# Configure the project
idf.py menuconfig

# Build the project
idf.py build

# Flash to ESP32
idf.py -p /dev/ttyUSB0 flash monitor
```

### Menuconfig Options

In `idf.py menuconfig`, configure:
- **Smart Poultry Config**: WiFi credentials, MQTT broker, sensor pins
- **Serial flasher config**: Flash size, baud rate

## Configuration

### WiFi Configuration
```c
// In menuconfig or NVS storage
#define CONFIG_WIFI_SSID "YourSSID"
#define CONFIG_WIFI_PASSWORD "YourPassword"
```

### MQTT Configuration
```c
#define CONFIG_MQTT_BROKER_URL "mqtt://broker.hivemq.com:1883"
#define CONFIG_MQTT_CLIENT_ID "smart_poultry_esp32"
#define CONFIG_MQTT_TOPIC_PREFIX "smart-poultry"
```

### Sensor Thresholds (menuconfig)
- Temperature min/max/optimal
- Humidity min/max/optimal
- Gas sensor thresholds
- Light level thresholds

### NVS Storage
The system uses NVS (Non-Volatile Storage) for:
- WiFi credentials
- MQTT settings
- Sensor calibration
- Custom thresholds
- System state

## Project Structure

```
smart-poultry-v3/
├── .github/
│   └── workflows/
│       ├── build.yml          # CI build workflow
│       └── release.yml        # Release workflow
├── components/
│   ├── actuators/             # Actuator drivers
│   │   ├── actuator_manager.c
│   │   ├── alarm.c
│   │   ├── cooling_pad.c
│   │   ├── exhaust_fan.c
│   │   ├── feeder.c
│   │   ├── heater.c
│   │   ├── led_light.c
│   │   └── water_pump.c
│   ├── communication/         # WiFi, MQTT, OTA
│   │   ├── comm_manager.c
│   │   ├── mqtt_client.c
│   │   ├── ota_update.c
│   │   └── wifi_manager.c
│   ├── control/               # Control algorithms
│   │   ├── control_manager.c
│   │   ├── decision_algorithm.c
│   │   ├── emergency_protocol.c
│   │   ├── energy_optimizer.c
│   │   ├── pid_controller.c
│   │   └── rules_engine.c
│   ├── sensors/              # Sensor drivers
│   │   ├── dht22.c
│   │   ├── ds18b20.c
│   │   ├── ldr.c
│   │   ├── mq2.c
│   │   ├── mq7.c
│   │   ├── mq135.c
│   │   ├── sensor_manager.c
│   │   ├── sound_sensor.c
│   │   └── water_level.c
│   └── storage/              # NVS storage
│       ├── nvs_storage.c
│       └── storage_manager.c
├── main/
│   ├── main.c                # Application entry point
│   ├── CMakeLists.txt
│   └── include/main.h
├── CMakeLists.txt
├── partitions.csv
├── sdkconfig
└── README.md
```

## MQTT Topics

| Topic | Direction | Description |
|-------|-----------|-------------|
| `smart-poultry/sensors` | Out | All sensor readings JSON |
| `smart-poultry/sensors/temperature` | Out | Temperature data |
| `smart-poultry/sensors/humidity` | Out | Humidity data |
| `smart-poultry/sensors/gas/mq135` | Out | Air quality data |
| `smart-poultry/sensors/gas/mq7` | Out | CO level data |
| `smart-poultry/sensors/gas/mq2` | Out | Smoke level data |
| `smart-poultry/sensors/water-level` | Out | Water level data |
| `smart-poultry/sensors/light` | Out | Light intensity |
| `smart-poultry/sensors/sound` | Out | Sound level |
| `smart-poultry/actuators/status` | Out | Actuator states |
| `smart-poultry/control/set` | In | Set actuator state |
| `smart-poultry/config/set` | In | Update configuration |

## Development

### Adding New Sensors
1. Create sensor driver in `components/sensors/`
2. Add to `sensor_manager.c`
3. Configure pins in `menuconfig`

### Adding New Actuators
1. Create actuator driver in `components/actuators/`
2. Add to `actuator_manager.c`
3. Configure pins in `menuconfig`

### Testing
```bash
# Run unit tests (if implemented)
idf.py test

# Monitor serial output
idf.py monitor
```

## License

MIT License

Copyright (c) 2024 Smart Poultry Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---

**Note**: This project is designed for educational and research purposes. Always validate sensor readings and implement proper safety measures for production use.
