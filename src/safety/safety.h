#pragma once

#include "../hardware/l298n/l298n.h"
#include "../hardware/tb6612/tb6612.h"

class Safety {
public:
    void begin(L298N& roverDriver, TB6612& actuatorDriver);
    void update();
    void refreshRover();
    void refreshActuator();
    void emergencyStop();
    void clearEmergencyStop();
    bool isEmergencyStopped() const;
    bool emergencyStopActive() const;
    bool roverTimedOut() const;
    bool actuatorTimedOut() const;

private:
    L298N* roverDriver_ = nullptr;
    TB6612* actuatorDriver_ = nullptr;
    bool emergencyStopActive_ = false;
    bool roverTimedOut_ = true;
    bool actuatorTimedOut_ = true;
    unsigned long lastRoverRefresh_ = 0;
    unsigned long lastActuatorRefresh_ = 0;
};
