#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

namespace Display
{
    void initialize();
    void showSensors(float tempC, float lightPercent);
}

#endif
