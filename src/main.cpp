#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include "config.h"
#include "pins.h"
#include "sensors.h"
#include "led_controller.h"
#include "api_handlers.h"
#include "display.h"

float gLightThreshold    = 30.0f;
float gTempColdThreshold = 15.0f;
float gTempHotThreshold  = 25.0f;

static unsigned long gLastDisplayUpdate = 0;

const char *WIFI_SSID = "Bbox-51B4C1FF"; //"HAARP_Anthena"; //"Livebox-74F0";
const char *WIFI_PASSWORD = "aWLWpsPRQqPz576AQ5"; //"1357908642"; //"Aaqy2gtC9AivP2uGgC";

WebServer server(SERVER_PORT);

void setupWiFi()
{
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupOTA()
{
  ArduinoOTA.setHostname("ESP32-Sensors");

  ArduinoOTA.onStart([]()
                     {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else
      type = "filesystem";
    Serial.println("Start updating " + type); });

  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });

  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();
  Serial.println("OTA Ready");
}

void setup()
{
  Serial.begin(115200);

  LEDController::initialize();
  SensorManager::initialize();
  Display::initialize();

  setupWiFi();
  setupOTA();

  APIHandlers::setupRoutes(server);

  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  ArduinoOTA.handle();
  server.handleClient();
  LEDController::update();

  float tempC    = SensorManager::readTemperature();
  float lightPct = SensorManager::readLight();
  Display::showSensors(tempC, lightPct);

  delay(SENSOR_READ_DELAY);
}