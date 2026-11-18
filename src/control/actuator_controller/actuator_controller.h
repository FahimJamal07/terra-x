#pragma once

#include "../../hardware/tb6612/tb6612.h"
#include "../../safety/safety.h"
#include "../../types.h"

class ActuatorController {
public:
    void begin(TB6612& driver, Safety& safety);
    void update();
    void up();
    void down();
    void stop();
    void setSpeed(uint8_t speed);
    ActuatorState getState() const;

private:
    uint8_t constrainedSpeed(uint8_t speed) const;

    TB6612* driver_ = nullptr;
    Safety* safety_ = nullptr;
    uint8_t speed_ = 200;
    ActuatorState state_ = ActuatorState::Stopped;
};
