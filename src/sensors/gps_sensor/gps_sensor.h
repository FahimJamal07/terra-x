#pragma once

#include <stddef.h>
#include <stdint.h>
#include <TinyGPS++.h>

#include "../../hardware/gps/gps.h"
#include "../../types.h"

class GPSSensor {
public:
    void begin(GPSHardware& hardware);
    void update();
    GPSData getData() const;
    GPSData data() const;
    bool isHealthy() const;

private:
    void processByte(char value);
    bool hasFreshData() const;

    GPSHardware* hardware_ = nullptr;
    TinyGPSPlus gps_;
    GPSData data_;
    uint32_t lastValidUpdate_ = 0;
};
