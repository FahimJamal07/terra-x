#pragma once

#include <HardwareSerial.h>

#include "../../types.h"

class GPSHardware {
public:
    void begin();
    void update();
    int available();
    int read();
    size_t read(uint8_t* buffer, size_t length);
    HardwareSerial& serial();
    GPSData data() const;

private:
    HardwareSerial serial_{2};
    GPSData data_;
};
