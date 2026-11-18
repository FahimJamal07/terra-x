#pragma once

#include <HardwareSerial.h>

#include "../../types.h"

class SoilRs485 {
public:
    void begin();
    void update();
    int available();
    int read();
    size_t read(uint8_t* buffer, size_t length);
    size_t write(const uint8_t* buffer, size_t length);
    void clearInput();
    HardwareSerial& serial();
    SoilData data() const;

private:
    HardwareSerial serial_{1};
    SoilData data_;
};
