#pragma once

#include <stdint.h>

class L298N {
public:
    void begin();
    void update();
    void forward(uint8_t speed);
    void reverse(uint8_t speed);
    void left(uint8_t speed);
    void right(uint8_t speed);
    void stop();
    void setSpeed(uint8_t speed);

private:
    uint8_t constrainedSpeed(uint8_t speed) const;
    void setDirection(bool leftFirst, bool leftSecond, bool rightFirst, bool rightSecond);

    uint8_t speed_ = 0;
};
