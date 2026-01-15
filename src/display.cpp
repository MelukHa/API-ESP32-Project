#include "display.h"

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#include "config.h"
#include "sensors.h"

// -----------------------------------------------------------------------------
// Pins typiques du TTGO T-Display (v1) pour l'écran ST7789
// Vérifie avec ton schéma/carton si nécessaire, mais cette config est la plus
// courante :
//   TFT_MOSI = 19
//   TFT_SCLK = 18
//   TFT_CS   = 5
//   TFT_DC   = 16
//   TFT_RST  = 23
//   TFT_BL   = 4 (backlight)
// -----------------------------------------------------------------------------
static const int TFT_MOSI = 19;
static const int TFT_SCLK = 18;
static const int TFT_CS   = 5;
static const int TFT_DC   = 16;
static const int TFT_RST  = 23;
static const int TFT_BL   = 4;   // rétro-éclairage (souvent 4 sur T-Display)

static Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

void Display::initialize()
{
    // Activer le rétro-éclairage
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    // Initialisation SPI pour l’écran
    SPI.begin(TFT_SCLK, -1, TFT_MOSI); // MISO non utilisé (-1)

    // Écran 135x240 ST7789 (T-Display)
    tft.init(135, 240);   // hauteur, largeur
    tft.setRotation(1);   // paysage, ajuste si besoin (0..3)

    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Boot TTGO");
}

void Display::showSensors(float tempC, float lightPercent)
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);

    if (tempC == TEMP_ERROR_VALUE || lightPercent == LIGHT_ERROR_VALUE)
    {
        tft.setCursor(10, 20);
        tft.print("Capteurs");
        tft.setCursor(10, 45);
        tft.print("en erreur");
        return;
    }

    char buf[32];

    tft.setCursor(10, 20);
    snprintf(buf, sizeof(buf), "Temp: %.1f C", tempC);
    tft.print(buf);

    tft.setCursor(10, 50);
    snprintf(buf, sizeof(buf), "Lumiere: %.1f %%", lightPercent);
    tft.print(buf);
}
