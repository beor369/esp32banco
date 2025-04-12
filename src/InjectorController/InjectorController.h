#ifndef INJECTOR_CONTROLLER_H
#define INJECTOR_CONTROLLER_H

#include <Arduino.h>
#include "config.h"
#include "selectrpm/selectrpm.h"
#include "menu/Menu.h"
struct InyectorParametros {
  long frecuencia;
  unsigned long anchoPulsoUs;
  unsigned long tiempoPruebaSec;
  long frecuenciaa;
  unsigned long anchoPulsoUss;
  unsigned long tiempoPruebaSecc;
};

class InjectorController {
  public:
    uint8_t pin;
    uint8_t channel;
    uint8_t resolution;
    float frequency;
    float rpmValue_tem=0;
    unsigned long pulseWidthUs;
    unsigned long testDurationMs;
    bool is_activate_max= false;
    bool is_activate_min= false;
    bool isActive;
    unsigned long startTime;
    void calculateDutyCycle();

  public:
    InjectorController(uint8_t pin, uint8_t channel, uint8_t resolution = 12);
    InyectorParametros activarInyectorDesdeEncoder(long test_time);
    InyectorParametros devolver();
    void begin();
    void assess_ina();
    void activate(float freq, unsigned long pulseWidthUs, unsigned long testDurationSec);
    // void activateFromRPM(unsigned int rpm, float percentage, unsigned long testDurationSec); // Nueva función
    void update();
    void stop();
    void stop_bomba();
    void begin_bomba();
    void activar_injector_sin_revolucion(float time);
    bool isInjectorActive() const;
};
extern InjectorController inyector; // Declaración externa
extern long frecuencia;
extern unsigned long anchoPulsoUs;
extern unsigned long tiempoPruebaSec;
extern long frecuenciaa;
extern unsigned long anchoPulsoUss;
extern unsigned long tiempoPruebaSecc;

#endif