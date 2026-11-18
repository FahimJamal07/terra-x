#include "web_server.h"

#include <WiFi.h>

#include "../../config.h"

void WebService::begin() {
	WiFi.mode(WIFI_AP);
	WiFi.softAP(config::wifi::ssid, config::wifi::password);
	server_.on("/favicon.ico", HTTP_GET, [this]() {
		logRequest();
		server_.send(204, "text/plain", "");
	});
	server_.onNotFound([this]() {
		logRequest();
		server_.send(404, "application/json", "{\"error\":\"not_found\"}");
	});
	server_.begin();
}

void WebService::update() {
	server_.handleClient();
}

void WebService::logRequest() {
	Serial.print("HTTP REQUEST: ");
	Serial.println(server_.uri());
}

::WebServer& WebService::server() {
	return server_;
}
