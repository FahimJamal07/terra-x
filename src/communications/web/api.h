#pragma once

class System;
class WebService;

class Api {
public:
    void begin(WebService& webService, System& system);
    void update();

private:
    void registerRoutes();
    void handleStatus();
    void handleTelemetry();
    void handleDrive();
    void handleStop();
    void handleSpeed();
    void handleActuator();
    void handleEmergencyStop();
    void handleEmergencyClear();
    void handleMission();
    void handleAddWaypoint();
    void handleClearWaypoints();
    void handleListWaypoints();
    void handleLatestSample();

    WebService* webService_ = nullptr;
    System* system_ = nullptr;
};
