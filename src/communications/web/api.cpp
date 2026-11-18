#include "api.h"

#include <Arduino.h>
#include <math.h>
#include <stdlib.h>

#include "../../config.h"
#include "../../core/system.h"
#include "../../types.h"
#include "web_server.h"

namespace {

const char* driveDirectionName(DriveDirection direction) {
	switch (direction) {
		case DriveDirection::Forward:
			return "forward";
		case DriveDirection::Reverse:
			return "reverse";
		case DriveDirection::Left:
			return "left";
		case DriveDirection::Right:
			return "right";
		default:
			return "stop";
	}
}

const char* actuatorStateName(ActuatorState state) {
	switch (state) {
		case ActuatorState::Up:
			return "up";
		case ActuatorState::Down:
			return "down";
		default:
			return "stop";
	}
}

const char* operatingModeName(OperatingMode mode) {
	switch (mode) {
		case OperatingMode::Autonomous:
			return "autonomous";
		case OperatingMode::Mission:
			return "mission";
		case OperatingMode::Emergency:
			return "emergency";
		default:
			return "manual";
	}
}

const char* navigationStateName(NavigationState state) {
	switch (state) {
		case NavigationState::MOVING:
			return "autonomous";
		case NavigationState::ARRIVED:
			return "arrived";
		case NavigationState::GPS_LOST:
			return "gps lost";
		case NavigationState::ERROR:
			return "error";
		default:
			return "manual";
	}
}

const char* samplingStateName(SamplingState state) {
	switch (state) {
		case SamplingState::LOWERING:
			return "lowering";
		case SamplingState::SETTLING:
			return "settling";
		case SamplingState::READING:
			return "reading";
		case SamplingState::RAISING:
			return "raising";
		case SamplingState::COMPLETE:
			return "complete";
		case SamplingState::FAILED:
			return "failed";
		default:
			return "idle";
	}
}

bool jsonString(const String& body, const char* key, String& value) {
	const String keyToken = String("\"") + key + "\"";
	const int keyPosition = body.indexOf(keyToken);
	if (keyPosition < 0) {
		return false;
	}
	const int colon = body.indexOf(':', keyPosition + keyToken.length());
	const int openingQuote = body.indexOf('"', colon + 1);
	if (colon < 0 || openingQuote < 0) {
		return false;
	}
	const int closingQuote = body.indexOf('"', openingQuote + 1);
	if (closingQuote < 0) {
		return false;
	}
	value = body.substring(openingQuote + 1, closingQuote);
	return true;
}

bool jsonNumber(const String& body, const char* key, int& value) {
	const String keyToken = String("\"") + key + "\"";
	const int keyPosition = body.indexOf(keyToken);
	if (keyPosition < 0) {
		return false;
	}
	const int colon = body.indexOf(':', keyPosition + keyToken.length());
	if (colon < 0) {
		return false;
	}
	int position = colon + 1;
	while (position < body.length() && body[position] == ' ') {
		++position;
	}
	bool negative = false;
	if (position < body.length() && body[position] == '-') {
		negative = true;
		++position;
	}
	if (position >= body.length() || body[position] < '0' || body[position] > '9') {
		return false;
	}
	int parsed = 0;
	while (position < body.length() && body[position] >= '0' && body[position] <= '9') {
		parsed = parsed * 10 + body[position] - '0';
		++position;
	}
	value = negative ? -parsed : parsed;
	return true;
}

bool jsonDouble(const String& body, const char* key, double& value) {
	const String keyToken = String("\"") + key + "\"";
	const int keyPosition = body.indexOf(keyToken);
	if (keyPosition < 0) {
		return false;
	}
	const int colon = body.indexOf(':', keyPosition + keyToken.length());
	if (colon < 0) {
		return false;
	}
	int end = body.indexOf(',', colon + 1);
	const int objectEnd = body.indexOf('}', colon + 1);
	if (end < 0 || (objectEnd >= 0 && objectEnd < end)) {
		end = objectEnd;
	}
	if (end < 0) {
		return false;
	}
	String number = body.substring(colon + 1, end);
	number.trim();
	char* parseEnd = nullptr;
	value = strtod(number.c_str(), &parseEnd);
	return parseEnd != number.c_str() && *parseEnd == '\0' && isfinite(value);
}

bool validSampleId(const String& sampleId) {
	if (sampleId.length() >= Waypoint::sampleIdCapacity) {
		return false;
	}
	for (size_t index = 0; index < sampleId.length(); ++index) {
		const char value = sampleId[index];
		if (!((value >= 'a' && value <= 'z') || (value >= 'A' && value <= 'Z') ||
			  (value >= '0' && value <= '9') || value == '-' || value == '_')) {
			return false;
		}
	}
	return true;
}

void sendJson(::WebServer& server, int code, const String& body) {
	server.send(code, "application/json", body);
}

void sendError(::WebServer& server, int code, const char* message) {
	sendJson(server, code, String("{\"ok\":false,\"error\":\"") + message + "\"}");
}

bool validSpeed(int speed) {
	return speed >= config::navigation::minimum_speed && speed <= config::rover::speed_maximum;
}

}  // namespace

void Api::begin(WebService& webService, System& system) {
	webService_ = &webService;
	system_ = &system;
	registerRoutes();
}

void Api::update() {
}

void Api::registerRoutes() {
	::WebServer& server = webService_->server();
	server.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
	server.on("/api/telemetry", HTTP_GET, [this]() { handleTelemetry(); });
	server.on("/api/drive", HTTP_POST, [this]() { handleDrive(); });
	server.on("/api/stop", HTTP_POST, [this]() { handleStop(); });
	server.on("/api/speed", HTTP_POST, [this]() { handleSpeed(); });
	server.on("/api/actuator", HTTP_POST, [this]() { handleActuator(); });
	server.on("/api/emergency-stop", HTTP_POST, [this]() { handleEmergencyStop(); });
	server.on("/api/emergency-clear", HTTP_POST, [this]() { handleEmergencyClear(); });
	server.on("/api/mission", HTTP_GET, [this]() { handleMission(); });
	server.on("/api/waypoints", HTTP_POST, [this]() { handleAddWaypoint(); });
	server.on("/api/waypoints", HTTP_GET, [this]() { handleListWaypoints(); });
	server.on("/api/waypoints/clear", HTTP_POST, [this]() { handleClearWaypoints(); });
	server.on("/api/sample/latest", HTTP_GET, [this]() { handleLatestSample(); });
}

void Api::handleStatus() {
	webService_->logRequest();
	const SystemStatus& status = system_->state().status();
	String response = "{\"operatingMode\":\"";
	response += operatingModeName(status.operatingMode);
	response += "\",\"roverState\":\"";
	response += driveDirectionName(status.driveDirection);
	response += "\",\"actuatorState\":\"";
	response += actuatorStateName(status.actuatorState);
	response += "\",\"speed\":";
	response += String(status.roverSpeed);
	response += ",\"sensorHealth\":{\"gps\":";
	response += status.gpsStatus ? "true" : "false";
	response += ",\"soil\":";
	response += status.soilStatus ? "true" : "false";
	response += "},\"emergency\":";
	response += status.emergencyStop ? "true" : "false";
	response += ",\"navigationState\":\"";
	response += navigationStateName(status.navigationState);
	response += "\"";
	response += "}";
	sendJson(webService_->server(), 200, response);
}

void Api::handleTelemetry() {
	webService_->logRequest();
	const GPSData gps = system_->gpsSensor().getData();
	const SoilData soil = system_->soilSensor().getData();
	const SystemStatus& status = system_->state().status();
	String response = "{\"gps\":{\"latitude\":";
	response += String(gps.latitude, 6);
	response += ",\"longitude\":";
	response += String(gps.longitude, 6);
	response += ",\"fix\":";
	response += gps.fixValid ? "true" : "false";
	response += ",\"satellites\":";
	response += String(gps.satellites);
	response += ",\"speedKmph\":";
	response += String(gps.speedKmph, 2);
	response += ",\"altitudeMeters\":";
	response += String(gps.altitudeMeters, 2);
	response += "},\"soil\":{\"moisture\":";
	response += String(soil.moisture, 2);
	response += ",\"temperature\":";
	response += String(soil.temperature, 2);
	response += ",\"conductivity\":";
	response += String(soil.conductivity, 2);
	response += ",\"pH\":";
	response += String(soil.pH, 2);
	response += ",\"status\":\"";
	response += system_->soilSensor().status();
	response += "\"";
	response += ",\"valid\":";
	response += soil.valid ? "true" : "false";
	response += "},\"system\":{\"operatingMode\":\"";
	response += operatingModeName(status.operatingMode);
	response += "\",\"roverState\":\"";
	response += driveDirectionName(status.driveDirection);
	response += "\",\"actuatorState\":\"";
	response += actuatorStateName(status.actuatorState);
	response += "\",\"speed\":";
	response += String(status.roverSpeed);
	response += ",\"sensorHealth\":{\"gps\":";
	response += status.gpsStatus ? "true" : "false";
	response += ",\"soil\":";
	response += status.soilStatus ? "true" : "false";
	response += "},\"emergency\":";
	response += status.emergencyStop ? "true" : "false";
	response += ",\"navigationState\":\"";
	response += navigationStateName(status.navigationState);
	response += "\"";
	response += "}}";
	sendJson(webService_->server(), 200, response);
}

void Api::handleDrive() {
	::WebServer& server = webService_->server();
	webService_->logRequest();
	if (!server.hasArg("plain")) {
		sendError(server, 400, "JSON body required");
		return;
	}
	String direction;
	if (!jsonString(server.arg("plain"), "direction", direction)) {
		sendError(server, 400, "direction required");
		return;
	}
	int speed = 0;
	const bool hasSpeed = jsonNumber(server.arg("plain"), "speed", speed);
	if ((direction != "stop" && !hasSpeed) || (hasSpeed && !validSpeed(speed))) {
		sendError(server, 400, "speed must be between 171 and 255");
		return;
	}
	if (direction != "stop" &&
		system_->actuatorController().getState() == ActuatorState::Down) {
		sendError(server, 409, "stop actuator before driving");
		return;
	}
	RoverController& rover = system_->roverController();
	if (direction == "forward") {
		rover.forward(static_cast<uint8_t>(speed));
	} else if (direction == "reverse") {
		rover.reverse(static_cast<uint8_t>(speed));
	} else if (direction == "left") {
		rover.left(static_cast<uint8_t>(speed));
	} else if (direction == "right") {
		rover.right(static_cast<uint8_t>(speed));
	} else if (direction == "stop") {
		rover.stop();
	} else {
		sendError(server, 400, "invalid drive direction");
		return;
	}
	sendJson(server, 200, "{\"ok\":true}");
}

void Api::handleStop() {
	webService_->logRequest();
	system_->roverController().stop();
	sendJson(webService_->server(), 200, "{\"ok\":true,\"state\":\"stop\"}");
}

void Api::handleSpeed() {
	::WebServer& server = webService_->server();
	webService_->logRequest();
	if (!server.hasArg("plain")) {
		sendError(server, 400, "JSON body required");
		return;
	}
	int speed = 0;
	if (!jsonNumber(server.arg("plain"), "speed", speed) || !validSpeed(speed)) {
		sendError(server, 400, "speed must be between 171 and 255");
		return;
	}
	system_->roverController().setSpeed(static_cast<uint8_t>(speed));
	sendJson(server, 200, "{\"ok\":true}");
}

void Api::handleActuator() {
	::WebServer& server = webService_->server();
	webService_->logRequest();
	if (!server.hasArg("plain")) {
		sendError(server, 400, "JSON body required");
		return;
	}
	const String body = server.arg("plain");
	String action;
	if (!jsonString(body, "action", action)) {
		sendError(server, 400, "action required");
		return;
	}
	if (action != "up" && action != "down" && action != "stop") {
		sendError(server, 400, "invalid actuator action");
		return;
	}
	int speed = 0;
	if (jsonNumber(body, "speed", speed)) {
		if (!validSpeed(speed)) {
			sendError(server, 400, "speed must be between 171 and 255");
			return;
		}
		system_->actuatorController().setSpeed(static_cast<uint8_t>(speed));
	}
	ActuatorController& actuator = system_->actuatorController();
	if (action == "up") {
		actuator.up();
	} else if (action == "down") {
		system_->roverController().stop();
		actuator.down();
	} else if (action == "stop") {
		actuator.stop();
	}
	sendJson(server, 200, "{\"ok\":true}");
}

void Api::handleEmergencyStop() {
	webService_->logRequest();
	system_->safety().emergencyStop();
	sendJson(webService_->server(), 200, "{\"ok\":true,\"emergency\":true}");
}

void Api::handleEmergencyClear() {
	webService_->logRequest();
	system_->safety().clearEmergencyStop();
	system_->roverController().stop();
	system_->actuatorController().stop();
	sendJson(webService_->server(), 200, "{\"ok\":true,\"emergency\":false}");
}

void Api::handleMission() {
	webService_->logRequest();
	sendJson(webService_->server(), 200, "{\"implemented\":false,\"message\":\"not implemented\"}");
}

void Api::handleAddWaypoint() {
	::WebServer& server = webService_->server();
	webService_->logRequest();
	if (!server.hasArg("plain")) {
		sendError(server, 400, "JSON body required");
		return;
	}
	const String body = server.arg("plain");
	Waypoint waypoint;
	if (!jsonDouble(body, "latitude", waypoint.latitude) ||
		!jsonDouble(body, "longitude", waypoint.longitude)) {
		sendError(server, 400, "latitude and longitude required");
		return;
	}
	if (jsonDouble(body, "arrivalRadius", waypoint.arrivalRadius) && waypoint.arrivalRadius < 0.0) {
		sendError(server, 400, "arrivalRadius must be non-negative");
		return;
	}
	String sampleId;
	if (jsonString(body, "sampleId", sampleId)) {
		if (!validSampleId(sampleId)) {
			sendError(server, 400, "sampleId contains invalid characters");
			return;
		}
		sampleId.toCharArray(waypoint.sampleId, sizeof(waypoint.sampleId));
	}
	if (!system_->waypointManager().addWaypoint(waypoint)) {
		sendError(server, 400, "invalid waypoint or waypoint capacity reached");
		return;
	}
	sendJson(server, 200, "{\"ok\":true}");
}

void Api::handleClearWaypoints() {
	webService_->logRequest();
	system_->waypointManager().clearWaypoints();
	system_->navigationController().clearTarget();
	sendJson(webService_->server(), 200, "{\"ok\":true,\"waypoints\":[]}");
}

void Api::handleListWaypoints() {
	webService_->logRequest();
	const WaypointManager& manager = system_->waypointManager();
	String response = "{\"current\":";
	response += String(manager.getCurrentWaypointIndex());
	response += ",\"complete\":";
	response += manager.isMissionComplete() ? "true" : "false";
	response += ",\"waypoints\":[";
	for (size_t index = 0; index < manager.getWaypointCount(); ++index) {
		Waypoint waypoint;
		if (index > 0) {
			response += ',';
		}
		manager.getWaypoint(index, waypoint);
		response += "{\"latitude\":";
		response += String(waypoint.latitude, 7);
		response += ",\"longitude\":";
		response += String(waypoint.longitude, 7);
		response += ",\"arrivalRadius\":";
		response += String(waypoint.arrivalRadius, 2);
		response += ",\"sampleId\":\"";
		response += waypoint.sampleId;
		response += "\"}";
	}
	response += "]}";
	sendJson(webService_->server(), 200, response);
}

void Api::handleLatestSample() {
	webService_->logRequest();
	const SamplingController& sampling = system_->samplingController();
	String response = "{\"state\":\"";
	response += samplingStateName(sampling.getState());
	response += "\",\"available\":";
	response += sampling.hasLatestSample() ? "true" : "false";
	if (sampling.hasLatestSample()) {
		const SampleRecord& sample = sampling.getLatestSample();
		response += ",\"sample\":{\"waypointId\":\"";
		response += sample.waypointId;
		response += "\",\"latitude\":";
		response += String(sample.latitude, 7);
		response += ",\"longitude\":";
		response += String(sample.longitude, 7);
		response += ",\"timestamp\":";
		response += String(sample.timestamp);
		response += ",\"moisture\":";
		response += String(sample.moisture, 2);
		response += ",\"temperature\":";
		response += String(sample.temperature, 2);
		response += ",\"EC\":";
		response += String(sample.conductivity, 2);
		response += ",\"pH\":";
		response += String(sample.pH, 2);
		response += ",\"valid\":";
		response += sample.valid ? "true" : "false";
		response += "}";
	}
	response += "}";
	sendJson(webService_->server(), 200, response);
}
