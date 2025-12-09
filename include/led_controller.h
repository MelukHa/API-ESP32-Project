#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>

class LEDController
{
public:
    static void initialize();

    static void setColor(uint8_t red, uint8_t green, uint8_t blue);
    static void off();
    static void disableAllModes();

    static void linkRgbLightMode(bool enable);
    static void linkRgbTempMode(bool enable);

    static bool isLightLinked();
    static bool isTempLinked();

    static void update();

private:
    static bool lightLinked;
    static bool tempLinked;

    static void updateLightLink();
    static void updateTempLink();
    static void updateLightThresholdIndicator();
};

#endif