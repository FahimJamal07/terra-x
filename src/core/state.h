#pragma once

#include "../types.h"

class State {
public:
    State();

    const SystemStatus& status() const;
    void setStatus(const SystemStatus& status);

private:
    SystemStatus status_;
};
