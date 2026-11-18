#pragma once

#include <stdint.h>

enum class DriveDirection : uint8_t {
    Stop,
    Forward,
    Reverse,
    Left,
    Right
};

enum class ActuatorState : uint8_t {
    Stopped,
    Up,
    Down
};

enum class OperatingMode : uint8_t {
    Manual,
    Autonomous,
    Mission,
    Emergency
};

enum class NavigationState : uint8_t {
    IDLE,
    MOVING,
    ARRIVED,
    GPS_LOST,
    ERROR
};

enum class SamplingState : uint8_t {
    IDLE,
    LOWERING,
    SETTLING,
    READING,
    RAISING,
    COMPLETE,
    FAILED
};

struct GPSData {
    double latitude = 0.0;
    double longitude = 0.0;
    double speedKmph = 0.0;
    double altitudeMeters = 0.0;
    uint8_t satellites = 0;
    bool fixValid = false;
    uint32_t timestamp = 0;
};

struct SoilData {
    float moisture = 0.0F;
    float temperature = 0.0F;
    float conductivity = 0.0F;
    float pH = 0.0F;
    bool valid = false;
    uint32_t timestamp = 0;
};

struct SystemStatus {
    OperatingMode operatingMode = OperatingMode::Manual;
    DriveDirection driveDirection = DriveDirection::Stop;
    ActuatorState actuatorState = ActuatorState::Stopped;
    uint8_t roverSpeed = 0;
    bool gpsStatus = false;
    bool soilStatus = false;
    bool emergencyStop = false;
    NavigationState navigationState = NavigationState::IDLE;
};
