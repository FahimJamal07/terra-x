#pragma once

class WebService;

class Dashboard {
public:
    void begin(WebService& webService);
    void update();
};
