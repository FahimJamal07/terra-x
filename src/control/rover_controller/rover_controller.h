#pragma once

#include "../../hardware/l298n/l298n.h"
#include "../../safety/safety.h"
#include "../../types.h"

class RoverController {
public:
    void begin(L298N& driver, Safety& safety);
    void update();
    void stop();
    void forward(uint8_t speed);
    void reverse(uint8_t speed);
    void left(uint8_t speed);
    void right(uint8_t speed);
    void setSpeed(uint8_t speed);
    uint8_t getSpeed() const;
    DriveDirection getDirection() const;

private:
    uint8_t constrainedSpeed(uint8_t speed) const;

    L298N* driver_ = nullptr;
    Safety* safety_ = nullptr;
    uint8_t speed_ = 200;
    DriveDirection direction_ = DriveDirection::Stop;
};
