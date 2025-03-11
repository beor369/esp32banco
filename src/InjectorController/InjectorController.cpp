#include "InjectorController/InjectorController.h"
#include "config.h"
#include "menu/Menu.h"
#include "selectrpm/selectrpm.h"
#include <math.h>



void InjectorController::activarInyectorDesdeEncoder() {
  // Convertir a parámetros técnicos (asegurar unidades correctas)
  // float frecuencia = rpmValue / 120.0f;          // RPM → Hz (4 tiempos)
  // unsigned long anchoPulsoUs = pulseWidthValue * 1000; // ms → µs
  // unsigned long tiempoPruebaSec = testTimeValue / 1000; // ms → segundos
  frecuencia = rpmValue / 120.0f;          // RPM → Hz (4 tiempos)
  anchoPulsoUs = pulseWidthValue * 1000; // ms → µs
  tiempoPruebaSec = testTimeValue / 1000; // ms → segundos
 
  inyector.activate(frecuencia, anchoPulsoUs, tiempoPruebaSec);
}
InjectorController::InjectorController(uint8_t pin, uint8_t channel, uint8_t resolution) 
  : pin(pin), channel(channel), resolution(resolution), isActive(false) {}

void InjectorController::begin() {
  ledcSetup(channel, 1, resolution);
  ledcAttachPin(pin, channel);
}

void InjectorController::calculateDutyCycle() {
  uint64_t periodUs = 1000000 / frequency;
  if (pulseWidthUs > periodUs) {
    Serial.println("Error: Pulse width > Period");
    stop();
    return;
  }
  
  uint32_t duty = (pulseWidthUs * frequency * (1ULL << resolution)) / 1000000ULL;
  duty = min(duty, (uint32_t)((1ULL << resolution) - 1)); // ¡Corrección aquí!
  ledcWrite(channel, duty);
}

void InjectorController::activate(float freq, unsigned long pulseWidthUs, unsigned long testDurationSec) {
  frequency = freq;
  this->pulseWidthUs = pulseWidthUs;
  testDurationMs = testDurationSec * 1000;
  
  ledcSetup(channel, frequency, resolution);
  calculateDutyCycle();
  
  isActive = true;
  startTime = millis();
  
  Serial.printf("Inyector ACTIVADO - F: %.1fHz, PW: %luµs, T: %lus\n", 
               frequency, pulseWidthUs, testDurationSec);
}


// void InjectorController::activate(float freq, unsigned long pulseWidthUs, unsigned long testDurationSec) {
//   // Calcular la resolución mínima necesaria para que el divisor no supere el límite
//   // Usamos clock = 40e6 (en Hz) y max_divider = 1023
//   uint8_t min_resolution = ceil(log2(40000000.0 / (1023.0 * freq)));

//   // Limitar la resolución a un rango aceptable (por ejemplo, entre 8 y 15 bits)
//   if (min_resolution < 8) {
//     min_resolution = 8;
//   } else if (min_resolution > 15) {
//     min_resolution = 15;
//   }

//   // Configurar PWM con la resolución calculada
//   ledcSetup(channel, freq, min_resolution);

//   // Calcular el duty cycle según la resolución actual
//   uint64_t periodUs = 1000000 / freq;
//   uint32_t duty = (pulseWidthUs * (1ULL << min_resolution)) / periodUs;
//   duty = constrain(duty, 1, (1U << min_resolution) - 1);

//   // Resto del código...
//   ledcWrite(channel, duty);
//   Serial.printf("Resolución: %u bits | Duty: %u\n", min_resolution, duty);
// }


void InjectorController::update() {
  if (isActive && (millis() - startTime >= testDurationMs)) {
    stop();
  }
}

void InjectorController::stop() {
  ledcWrite(channel, 0);
  isActive = false;
  Serial.println("Inyector DESACTIVADO");
}

bool InjectorController::isInjectorActive() const {
  return isActive;
}