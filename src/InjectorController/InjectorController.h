#ifndef INJECTOR_CONTROLLER_H
#define INJECTOR_CONTROLLER_H

#include <Arduino.h>
#include "config.h"
#include "selectrpm/selectrpm.h"
#include "menu/Menu.h"
class InjectorController {
  private:
    uint8_t pin;
    uint8_t channel;
    uint8_t resolution;
    float frequency;
    unsigned long pulseWidthUs;
    unsigned long testDurationMs;
    bool isActive;
    unsigned long startTime;
    void calculateDutyCycle();

  public:
    InjectorController(uint8_t pin, uint8_t channel, uint8_t resolution = 12);
   void activarInyectorDesdeEncoder();
    void begin();
    void activate(float freq, unsigned long pulseWidthUs, unsigned long testDurationSec);
    // void activateFromRPM(unsigned int rpm, float percentage, unsigned long testDurationSec); // Nueva función
    void update();
    void stop();
    bool isInjectorActive() const;
};
extern InjectorController inyector; // Declaración externa
extern float frecuencia;
extern unsigned long anchoPulsoUs;
extern unsigned long tiempoPruebaSec;
#endif