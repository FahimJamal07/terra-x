#pragma once

#include <WebServer.h>

class WebService {
public:
    void begin();
    void update();
    void logRequest();
    ::WebServer& server();

private:
    ::WebServer server_{80};
};
