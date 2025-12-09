#ifndef CONFIG_H
#define CONFIG_H

// Configuration WiFi (extern pour éviter les définitions multiples)
extern const char *WIFI_SSID;
extern const char *WIFI_PASSWORD;

// Configuration serveur
constexpr int SERVER_PORT = 80;

// Valeurs d'erreur des capteurs
constexpr float TEMP_ERROR_VALUE = -999.0f;
constexpr float LIGHT_ERROR_VALUE = -1.0f;

// Plages de normalisation
constexpr float TEMP_MIN = 0.0f;
constexpr float TEMP_MAX = 50.0f;

// Constantes thermistance
constexpr float THERM_REFERENCE_TEMP = 298.15f;
constexpr float THERM_BETA = 3950.0f;
constexpr float THERM_REFERENCE_RES = 10000.0f;

// Timing
constexpr int SENSOR_READ_DELAY = 500;
constexpr int SENSOR_INIT_DELAY = 100;

// thresholds sensors
extern float gLightThreshold;
extern float gTempColdThreshold;
extern float gTempHotThreshold;
#endif