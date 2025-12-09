#ifndef API_HANDLERS_H
#define API_HANDLERS_H

#include <WebServer.h>

namespace APIHandlers
{
    void setupRoutes(WebServer &server);

    void handleGetSensors();
    void handleGetLight();
    void handleGetTemperature();

    void handleLinkRgbLightMode();
    void handleUnlinkRgbLightMode();
    void handleLinkRgbTempMode();
    void handleUnlinkRgbTempMode();

    void sendError(int code, const char *message);

    void handleGetThresholds();
    void handleUpdateThresholds();

    void handleLedColorJson();
    void handleLedOff();
}

#endif