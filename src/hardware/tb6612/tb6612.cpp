#include "tb6612.h"

#include <Arduino.h>

#include "../../config.h"

void TB6612::begin() {
    pinMode(config::actuator::ain1, OUTPUT);
    pinMode(config::actuator::ain2, OUTPUT);
    pinMode(config::actuator::pwma, OUTPUT);
    pinMode(config::actuator::stby, OUTPUT);
    digitalWrite(config::actuator::stby, LOW);
    stop();
}

void TB6612::update() {
}

void TB6612::up(uint8_t speed) {
    digitalWrite(config::actuator::ain1, HIGH);
    digitalWrite(config::actuator::ain2, LOW);
    digitalWrite(config::actuator::stby, HIGH);
    setSpeed(speed);
}

void TB6612::down(uint8_t speed) {
    digitalWrite(config::actuator::ain1, LOW);
    digitalWrite(config::actuator::ain2, HIGH);
    digitalWrite(config::actuator::stby, HIGH);
    setSpeed(speed);
}

void TB6612::stop() {
    analogWrite(config::actuator::pwma, 0);
    digitalWrite(config::actuator::ain1, LOW);
    digitalWrite(config::actuator::ain2, LOW);
    digitalWrite(config::actuator::stby, LOW);
    speed_ = 0;
}

void TB6612::setSpeed(uint8_t speed) {
    speed_ = constrainedSpeed(speed);
    analogWrite(config::actuator::pwma, speed_);
}

uint8_t TB6612::constrainedSpeed(uint8_t speed) const {
    if (speed == 0) {
        return 0;
    }
    return constrain(speed, config::rover::speed_minimum, config::rover::speed_maximum);
}
