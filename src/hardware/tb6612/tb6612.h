#pragma once

#include <stdint.h>

class TB6612 {
public:
    void begin();
    void update();
    void up(uint8_t speed);
    void down(uint8_t speed);
    void stop();
    void setSpeed(uint8_t speed);

private:
    uint8_t constrainedSpeed(uint8_t speed) const;

    uint8_t speed_ = 0;
};
