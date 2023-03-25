#ifndef __MAIN_H
#define __MAIN_H

#include <FastLED.h>

#define UDP_PORT 4210
#define WIFI_SSID  "SSID"
#define WIFI_PASS  "PASS"

#define LEDS_PWM_ON  "600001"
#define LEDS_PWM_OFF  "600002"
#define LEDS_PWM_COLOR  "600003"
#define GET_TEMP        "600004"
#define GET_HUMIDITY        "600005"
#define MOVEMENTSENSORON   "600006"
#define MOVEMENTSENSOROFF   "600007"
#define TIME_LED_TRANSACTION    3000

#define LED_PIN     D5
#define NUM_LEDS    60
#define BRIGHTNESS  255
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define UPDATES_PER_SECOND 30

#define MOVEMENT_SENS_TIME  15000

#define JSONLENGHT 500



void SetCurrentColor(CRGB color);
void FadeLed(uint8_t val);
void PaletteModes(int val);
void ExecCmd(const char* cmd);
float GetTemperature();
float GetHumidity();
bool GetMovementSensorState();


#endif /* __MAIN_H */