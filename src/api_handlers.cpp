#include "api_handlers.h"
#include "sensors.h"
#include "led_controller.h"
#include "config.h"
#include "pins.h"
#include <ArduinoJson.h>

extern WebServer server;

void APIHandlers::setupRoutes(WebServer &srv)
{
    srv.on("/api/sensors", HTTP_GET, handleGetSensors);
    srv.on("/api/photoresistance", HTTP_GET, handleGetLight);
    srv.on("/api/thermoresistance", HTTP_GET, handleGetTemperature);
    srv.on("/api/ledrgb/link_light", HTTP_PATCH, handleLinkRgbLightMode);
    srv.on("/api/ledrgb/unlink_light", HTTP_PATCH, handleUnlinkRgbLightMode);
    srv.on("/api/ledrgb/link_temp", HTTP_PATCH, handleLinkRgbTempMode);
    srv.on("/api/ledrgb/unlink_temp", HTTP_PATCH, handleUnlinkRgbTempMode);

    srv.on("/api/config/thresholds", HTTP_GET, handleGetThresholds);
    srv.on("/api/config/thresholds", HTTP_PATCH, handleUpdateThresholds);
    srv.on("/api/ledrgb/color", HTTP_PATCH, handleLedColorJson);
    srv.on("/api/ledrgb/off", HTTP_PATCH, handleLedOff);
    
}

void APIHandlers::sendError(int code, const char *message)
{
    StaticJsonDocument<200> doc;
    doc["status"] = "error";
    doc["message"] = message;
    doc["code"] = code;

    String response;
    serializeJson(doc, response);
    server.send(code, "application/json", response);
}

void APIHandlers::handleGetSensors()
{
    if (!SensorManager::isInitialized())
    {
        sendError(503, "Sensors not initialized");
        return;
    }

    float tempValue = SensorManager::readTemperature();
    float lightValue = SensorManager::readLight();

    if (tempValue == TEMP_ERROR_VALUE || lightValue == LIGHT_ERROR_VALUE)
    {
        sendError(500, "Failed to read sensor data");
        return;
    }

    StaticJsonDocument<512> doc;
    JsonArray sensors = doc.createNestedArray("sensors");

    JsonObject temp = sensors.createNestedObject();
    temp["id"] = "temperature";
    temp["type"] = "thermistor";
    temp["unit"] = "celsius";
    temp["pin"] = Pins::THERMISTOR;
    temp["value"] = tempValue;

    JsonObject light = sensors.createNestedObject();
    light["id"] = "light";
    light["type"] = "ldr";
    light["unit"] = "percent";
    light["pin"] = Pins::LDR;
    light["value"] = lightValue;

    JsonObject led = sensors.createNestedObject();
    led["id"] = "led_indicator";
    led["type"] = "digital";
    led["pin"] = Pins::LED_INDICATOR;
    led["state"] = digitalRead(Pins::LED_INDICATOR);

    JsonObject rgb = sensors.createNestedObject();
    rgb["id"] = "rgb_led";
    rgb["type"] = "rgb";
    JsonObject pins = rgb.createNestedObject("pins");
    pins["r"] = Pins::RGB_RED;
    pins["g"] = Pins::RGB_GREEN;
    pins["b"] = Pins::RGB_BLUE;

    rgb["light_linked"] = LEDController::isLightLinked();
    rgb["temp_linked"]  = LEDController::isTempLinked();

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void APIHandlers::handleGetLight()
{
    if (!SensorManager::isInitialized())
    {
        sendError(503, "Light sensor not available or not initialized");
        return;
    }

    float lightValue = SensorManager::readLight();

    if (lightValue == LIGHT_ERROR_VALUE)
    {
        sendError(500, "Failed to read light sensor");
        return;
    }

    StaticJsonDocument<256> doc;
    doc["id"] = "light";
    doc["type"] = "ldr";
    doc["unit"] = "percent";
    doc["pin"] = Pins::LDR;
    doc["value"] = lightValue;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void APIHandlers::handleGetTemperature()
{
    if (!SensorManager::isInitialized())
    {
        sendError(503, "Temperature sensor not available or not initialized");
        return;
    }

    float tempValue = SensorManager::readTemperature();

    if (tempValue == TEMP_ERROR_VALUE)
    {
        sendError(500, "Failed to read temperature sensor");
        return;
    }

    StaticJsonDocument<256> doc;
    doc["id"] = "temperature";
    doc["type"] = "thermistor";
    doc["unit"] = "celsius";
    doc["pin"] = Pins::THERMISTOR;
    doc["value"] = tempValue;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void APIHandlers::handleLinkRgbLightMode()
{
    if (!SensorManager::isInitialized())
    {
        sendError(503, "Sensor or LED not available");
        return;
    }

    float testValue = SensorManager::readLight();
    if (testValue == LIGHT_ERROR_VALUE)
    {
        sendError(500, "Failed to link: light sensor error");
        return;
    }

    LEDController::linkRgbLightMode(true);
    server.send(200, "application/json", "{\"status\":\"ok\",\"link\":\"enabled\",\"color\":\"blue\"}");
}

void APIHandlers::handleUnlinkRgbLightMode()
{
    LEDController::linkRgbLightMode(false);
    LEDController::off();
    LEDController::disableAllModes();
    server.send(200, "application/json", "{\"status\":\"ok\",\"link\":\"disabled\"}");
}

void APIHandlers::handleLinkRgbTempMode()
{
    if (!SensorManager::isInitialized())
    {
        sendError(503, "Sensor or LED not available");
        return;
    }

    float testValue = SensorManager::readTemperature();
    if (testValue == TEMP_ERROR_VALUE)
    {
        sendError(500, "Failed to link: temperature sensor error");
        return;
    }

    LEDController::linkRgbTempMode(true);
    server.send(200, "application/json", "{\"status\":\"ok\",\"link\":\"enabled\",\"color\":\"red\"}");
}

void APIHandlers::handleUnlinkRgbTempMode()
{
    LEDController::linkRgbTempMode(false);
    LEDController::off();
    LEDController::disableAllModes();
    server.send(200, "application/json", "{\"status\":\"ok\",\"link\":\"disabled\"}");
}

void APIHandlers::handleGetThresholds()
{
    StaticJsonDocument<256> doc;
    doc["light_threshold"] = gLightThreshold;
    doc["temp_cold_threshold"] = gTempColdThreshold;
    doc["temp_hot_threshold"] = gTempHotThreshold;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void APIHandlers::handleUpdateThresholds()
{
    Serial.println("handleUpdateThresholds() called");

    if (!server.hasArg("plain")) {
        Serial.println("  -> ERROR: Missing JSON body");
        sendError(400, "Missing JSON body");
        return;
    }

    String body = server.arg("plain");
    Serial.print("  JSON body: ");
    Serial.println(body);

    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, body);

    if (err) {
        Serial.print("  -> ERROR: Invalid JSON: ");
        Serial.println(err.c_str());
        sendError(400, "Invalid JSON");
        return;
    }

    // On affiche les valeurs avant mise à jour
    Serial.print("  BEFORE: gLightThreshold=");
    Serial.print(gLightThreshold);
    Serial.print(", gTempColdThreshold=");
    Serial.print(gTempColdThreshold);
    Serial.print(", gTempHotThreshold=");
    Serial.println(gTempHotThreshold);

    if (doc.containsKey("light_threshold")) {
        float lt = doc["light_threshold"].as<float>();
        Serial.print("  Updating light_threshold -> ");
        Serial.println(lt);
        gLightThreshold = lt;
    }

    if (doc.containsKey("temp_cold_threshold")) {
        float tc = doc["temp_cold_threshold"].as<float>();
        Serial.print("  Updating temp_cold_threshold -> ");
        Serial.println(tc);
        gTempColdThreshold = tc;
    }

    if (doc.containsKey("temp_hot_threshold")) {
        float th = doc["temp_hot_threshold"].as<float>();
        Serial.print("  Updating temp_hot_threshold -> ");
        Serial.println(th);
        gTempHotThreshold = th;
    }

    // Sécurité: s'assurer que cold < hot
    if (gTempColdThreshold >= gTempHotThreshold)
    {
        float mid = (gTempColdThreshold + gTempHotThreshold) / 2.0f;
        gTempColdThreshold = mid - 1.0f;
        gTempHotThreshold  = mid + 1.0f;
        Serial.println("  Adjusted cold/hot thresholds to keep cold < hot");
    }

    // On affiche les valeurs après mise à jour
    Serial.print("  AFTER: gLightThreshold=");
    Serial.print(gLightThreshold);
    Serial.print(", gTempColdThreshold=");
    Serial.print(gTempColdThreshold);
    Serial.print(", gTempHotThreshold=");
    Serial.println(gTempHotThreshold);

    StaticJsonDocument<256> out;
    out["light_threshold"]       = gLightThreshold;
    out["temp_cold_threshold"]   = gTempColdThreshold;
    out["temp_hot_threshold"]    = gTempHotThreshold;

    String response;
    serializeJson(out, response);
    server.send(200, "application/json", response);
}

void APIHandlers::handleLedColorJson()
{
    if (!SensorManager::isInitialized())
    {
        sendError(503, "RGB LED not available or not initialized");
        return;
    }

    if (!server.hasArg("plain"))
    {
        sendError(400, "Missing JSON body");
        return;
    }

    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, server.arg("plain"));

    if (err)
    {
        sendError(400, "Invalid JSON");
        return;
    }

    if (!doc.containsKey("r") || !doc.containsKey("g") || !doc.containsKey("b"))
    {
        sendError(400, "Missing r/g/b fields");
        return;
    }

    int r = doc["r"];
    int g = doc["g"];
    int b = doc["b"];

    r = constrain(r, 0, 255);
    g = constrain(g, 0, 255);
    b = constrain(b, 0, 255);

    LEDController::setColor((uint8_t)r, (uint8_t)g, (uint8_t)b);

    StaticJsonDocument<128> out;
    out["status"] = "ok";
    out["r"] = r;
    out["g"] = g;
    out["b"] = b;

    String response;
    serializeJson(out, response);
    server.send(200, "application/json", response);
}

void APIHandlers::handleLedOff()
{
    LEDController::off();
    LEDController::disableAllModes();
    StaticJsonDocument<64> doc;
    doc["status"] = "ok";
    doc["color"]  = "off";

    String res;
    serializeJson(doc, res);
    server.send(200, "application/json", res);
}
