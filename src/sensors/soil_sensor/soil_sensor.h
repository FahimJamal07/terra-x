#pragma once

#include "../../hardware/rs485/rs485.h"
#include "../../types.h"

class SoilSensor {
public:
    void begin(SoilRs485& transport);
    void update();
    void deactivate();
    void activate();
    SoilData getData() const;
    SoilData data() const;
    bool isHealthy() const;
    const char* status() const;

private:
    void startRequest();
    void processResponse();

    static constexpr size_t responseCapacity = 256;
    SoilRs485* transport_ = nullptr;
    SoilData data_;
    const char* status_ = "NO_RESPONSE";
    uint32_t lastRequestAt_ = 0;
    uint32_t responseStartedAt_ = 0;
    size_t responseLength_ = 0;
    uint8_t response_[responseCapacity] = {};
    bool requested_ = false;
    bool requestInProgress_ = false;
    bool active_ = false;
    uint32_t activatedAt_ = 0;
};
