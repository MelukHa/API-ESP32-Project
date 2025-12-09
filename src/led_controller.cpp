#include "led_controller.h"
#include "sensors.h"
#include "config.h"
#include "pins.h"
#include <math.h>

bool LEDController::lightLinked = false;
bool LEDController::tempLinked = true;

void LEDController::initialize()
{
    pinMode(Pins::LED_INDICATOR, OUTPUT);
    pinMode(Pins::RGB_RED, OUTPUT);
    pinMode(Pins::RGB_GREEN, OUTPUT);
    pinMode(Pins::RGB_BLUE, OUTPUT);

    LEDController::off();

    // Éteindre la LED d’indication
    digitalWrite(Pins::LED_INDICATOR, LOW);

    // Par défaut : mode température ON, mode lumière OFF
    lightLinked = false;
    tempLinked  = true;
}

// Inversion pour cathode commune
inline uint8_t inv(uint8_t v) {
    return 255 - v;
}

void LEDController::setColor(uint8_t red, uint8_t green, uint8_t blue)
{
    lightLinked = false;
    tempLinked = false;

    analogWrite(Pins::RGB_RED,   inv(red));
    analogWrite(Pins::RGB_GREEN, inv(green));
    analogWrite(Pins::RGB_BLUE,  inv(blue));
}

void LEDController::off()
{
    // éteint la RGB
    analogWrite(Pins::RGB_RED,   255);
    analogWrite(Pins::RGB_GREEN, 255);
    analogWrite(Pins::RGB_BLUE,  255);

    // (optionnel) couper aussi la LED d’indication
    // digitalWrite(Pins::LED_INDICATOR, LOW);
}

void LEDController::disableAllModes()
{
    lightLinked = false;
    tempLinked  = false;
}

void LEDController::linkRgbLightMode(bool enable)
{
    if (enable)
    {
        // Mode lumière ON, température OFF
        lightLinked = true;
        tempLinked  = false;

        // On peut remettre la RGB à 0, sans toucher aux flags :
        LEDController::off();
    }
    else
    {
        lightLinked = false;
    }
}

void LEDController::linkRgbTempMode(bool enable)
{
    if (enable)
    {
        // Mode température ON, lumière OFF
        tempLinked  = true;
        lightLinked = false;

        LEDController::off();
    }
    else
    {
        tempLinked = false;
    }
}

bool LEDController::isLightLinked()
{
    return lightLinked;
}

bool LEDController::isTempLinked()
{
    return tempLinked;
}

void LEDController::updateLightLink()
{
    float lightPercent = SensorManager::readLight();

    if (lightPercent == LIGHT_ERROR_VALUE)
    {
        Serial.println("Warning: Invalid light reading");
        return;
    }

    float threshold = gLightThreshold;
    threshold = constrain(threshold, 0.0f, 100.0f);

    bool isDark = (lightPercent <= threshold);

    // On veut que plus il fait sombre (en dessous du seuil),
    // plus la LED bleue de la RGB est forte.
    float factor;

    if (isDark)
    {
        // lightPercent <= threshold
        // ratio: 0 (juste sous le seuil) -> 1 (0% lumière)
        float ratio = (threshold - lightPercent) / max(threshold, 1.0f);
        factor = 0.3f + 0.7f * ratio; // 0.3 .. 1.0
    }
    else
    {
        // lightPercent > threshold
        float ratio = (lightPercent - threshold) / max(100.0f - threshold, 1.0f);
        factor = max(0.0f, 1.0f - ratio); // 1 -> 0
    }

    int bluePwm = (int)(255 * pow(factor, 0.5f)); // gamma un peu doux
    bluePwm = constrain(bluePwm, 0, 255);

    // RGB = uniquement du BLEU en mode lumière
    analogWrite(Pins::RGB_RED,   255);
    analogWrite(Pins::RGB_GREEN, 255);
    analogWrite(Pins::RGB_BLUE,  inv(bluePwm));


    Serial.print("[LightLink] Light: ");
    Serial.print(lightPercent);
    Serial.print("% (thr=");
    Serial.print(threshold);
    Serial.print(") -> RGB(0,0,");
    Serial.print(bluePwm);
    Serial.println(")");
}

void LEDController::updateTempLink()
{
    float tempC = SensorManager::readTemperature();

    if (tempC == TEMP_ERROR_VALUE)
    {
        Serial.println("Warning: Invalid temperature reading");
        return;
    }

    float cold = gTempColdThreshold;
    float hot  = gTempHotThreshold;

    // Sécurité : si mal configuré, on retombe sur TEMP_MIN/TEMP_MAX
    if (cold >= hot)
    {
        cold = TEMP_MIN;
        hot  = TEMP_MAX;
    }

    uint8_t r, g, b;

    if (tempC <= cold)
    {
        r = 0;
        g = 0;
        b = 255;
    }
    else if (tempC >= hot)
    {
        r = 255;
        g = 0;
        b = 0;
    }
    else
    {
        // Entre les deux : dégradé bleu -> vert -> rouge
        float t = (tempC - cold) / (hot - cold);

        if (t < 0.5f)
        {
            // 0..0.5 => bleu -> vert
            float k = t / 0.5f;
            r = 0;
            g = (uint8_t)(255 * k);
            b = (uint8_t)(255 * (1.0f - k));
        }
        else
        {
            // 0.5..1 => vert -> rouge
            float k = (t - 0.5f) / 0.5f;
            r = (uint8_t)(255 * k);
            g = (uint8_t)(255 * (1.0f - k));
            b = 0;
        }
    }

    analogWrite(Pins::RGB_RED,   inv(r));
    analogWrite(Pins::RGB_GREEN, inv(g));
    analogWrite(Pins::RGB_BLUE,  inv(b));

    Serial.print("Temp: ");
    Serial.print(tempC);
    Serial.print("°C (cold=");
    Serial.print(cold);
    Serial.print(", hot=");
    Serial.print(hot);
    Serial.print(") -> RGB(");
    Serial.print(r);
    Serial.print(",");
    Serial.print(g);
    Serial.print(",");
    Serial.print(b);
    Serial.println(")");
}

void LEDController::updateLightThresholdIndicator()
{
    float lightPercent = SensorManager::readLight();

    if (lightPercent == LIGHT_ERROR_VALUE)
    {
        Serial.println("[Threshold] Invalid light reading");
        return;
    }

    float threshold = constrain(gLightThreshold, 0.0f, 100.0f);
    bool isDark = (lightPercent <= threshold);

    // LED rouge séparée = indicateur de seuil
    digitalWrite(Pins::LED_INDICATOR, isDark ? HIGH : LOW);
}

void LEDController::update()
{
    if (lightLinked)
    {
        updateLightLink();
    }

    if (tempLinked)
    {
        updateTempLink();
    }

    // check de la LED rouge de seuil (toujours active)
    updateLightThresholdIndicator();

    float lightPercent = SensorManager::readLight();
    float degrees = SensorManager::readTemperature();
    if (lightPercent >= 0)
    {
        Serial.print("Light : ");
        Serial.print(lightPercent);
        Serial.print("% ; Temperature : ");
        Serial.print(degrees);
        Serial.println("°C");
    }
}