#include <Arduino.h>

#include "config.h"
#include "core/system.h"

#ifndef TERRA_X_ENABLE_DEMO
#define TERRA_X_ENABLE_DEMO 0
#endif

System applicationSystem;

void setup() {
    Serial.begin(115200);
    applicationSystem.begin();
}

void loop() {
#if TERRA_X_ENABLE_DEMO
    applicationSystem.demoUpdate();
#else
    applicationSystem.update();
#endif
}
