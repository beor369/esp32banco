#ifndef INJECTOR_CONTROLLER_H
#define INJECTOR_CONTROLLER_H

#include <Arduino.h>
#include "config.h"
#include "selectrpm/selectrpm.h"
#include "menu/Menu.h"

// parámetros de la prueba de inyector
struct InyectorParametros {
  long frecuencia;               // Hz
  unsigned long anchoPulsoUs;    // µs
  unsigned long tiempoPruebaSec; // s
};

class InjectorController {
public:
  uint8_t pin;
  uint8_t channel;
  uint8_t resolution;

  // variables internas del controlador
  float frequency;               // Hz
  float rpmValue_tem = 0;
  unsigned long pulseWidthUs;    // µs
  unsigned long testDurationMs;  // ms
  bool is_activate_max = false;
  bool is_activate_min = false;
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

// instancia externa
extern InjectorController inyector;

// variables globales de configuración de prueba
extern float frequency;               // Hz
extern unsigned long pulseWidthUs;    // µs
extern unsigned long testDurationSec; // s
extern long frecuencia;
extern unsigned long anchoPulsoUs;
extern unsigned long tiempoPruebaSec;
#endif // INJECTOR_CONTROLLER_H
