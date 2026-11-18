#include "state.h"

State::State() = default;

const SystemStatus& State::status() const {
    return status_;
}

void State::setStatus(const SystemStatus& status) {
    status_ = status;
}
