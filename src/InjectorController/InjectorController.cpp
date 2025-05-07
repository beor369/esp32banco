#include "InjectorController/InjectorController.h"
#include "config.h"
#include "menu/Menu.h"
#include "selectrpm/selectrpm.h"
#include "GlobalVarials/GlobalVarials.h"
#include <math.h>
long frecuencia;
unsigned long anchoPulsoUs;
unsigned long tiempoPruebaSec;
unsigned long DurationSec;
 long frecuenciaa;
unsigned long anchoPulsoUss;
unsigned long tiempoPruebaSecc;

// definición de las variables globales
float frequency = 0.0f;               // Hz
unsigned long pulseWidthUs = 0;       // µs
unsigned long testDurationSec = 0;    // s
InjectorController inyector(INYECTOR_PIN, 0, 12);

InyectorParametros InjectorController::activarInyectorDesdeEncoder(long test_time)
{

  if (this->rpmValue_tem==0)
  {
    frecuencia = rpmValue / 120.0f;  
  }else
  {
    frecuencia = this->rpmValue_tem / 120.0f;  
  }

  
  anchoPulsoUs = pulseWidthValue * 1000;  // ms → µs

  tiempoPruebaSec = test_time / 1000; // ms → segundos
  DurationSec= tiempoPruebaSec;

  inyector.activate(frecuencia, anchoPulsoUs, tiempoPruebaSec);

  return {frecuencia, anchoPulsoUs, tiempoPruebaSec};
}


void InjectorController::activar_injector_sin_revolucion(float time)
{

}



InyectorParametros InjectorController::devolver()
{

  frecuenciaa = frecuencia ;     // RPM → Hz (4 tiempos)
  anchoPulsoUss =anchoPulsoUs;  // ms → µs
  tiempoPruebaSecc = tiempoPruebaSec; // ms → segundos

  

  return {frecuenciaa, anchoPulsoUss, tiempoPruebaSecc};
}
InjectorController::InjectorController(uint8_t pin, uint8_t channel, uint8_t resolution)
    : pin(pin), channel(channel), resolution(resolution), isActive(false) {}

void InjectorController::begin()
{
  ledcSetup(channel, 1, resolution);
  ledcAttachPin(pin, channel);
}

void InjectorController::assess_ina() {
//   // Lecturas del sensor INA219
//   float shuntVoltage = ina219.getShuntVoltage_mV(); // en mV
//   float busVoltage   = ina219.getBusVoltage_V();      // en V
//   float current      = ina219.getCurrent_mA();         // en mA
//   float power        = ina219.getPower_mW();           // en mW

//   // Imprimir valores medidos con 4 decimales (puedes ajustar el número según prefieras)
//   Serial.print("Bus Voltage:   "); Serial.print(busVoltage, 4); Serial.println(" V");
//   Serial.print("Shunt Voltage: "); Serial.print(shuntVoltage, 4); Serial.println(" mV");
//   Serial.print("Current:       "); Serial.print(current, 4); Serial.println(" mA");
//   Serial.print("Power:         "); Serial.print(power, 4); Serial.println(" mW");
//   Serial.println("");
//   // Convierte el shunt voltage de mV a V
// float v_shunt_V = shuntVoltage / 1000.0;
// // Calcula la corriente usando el valor del shunt resistor (0.1 Ω)
// float current_A = v_shunt_V / 0.1;  
// // Calcula la resistencia del inyector usando el bus voltage
// float r_inyector = v_shunt_V / current_A;

//   // Parámetros para ajuste de la medición
//   // double current_A = current / 1000;
//   Serial.print("current_A: "); Serial.println(current_A, 8);  // 8 decimales
//   Serial.print("r_inyector: "); Serial.println(r_inyector, 8);  // 8 decimale
//   double ohm= busVoltage / current_A;

//   InjectorData inyec = globalInjector.getInjector();
//   TestResult result;

//   result.expectedValue= inyec.resistencia;
//   result.measuredValue= ohm;
//   Serial.println(current_A, 8);  // 8 decimales
//   Serial.print(ohm,8);
//   if (inyec.resistencia > (ohm - 2) && inyec.resistencia < (ohm + 2)){
//       result.passed= true;
//   } else{
//       result.passed= false;
//   }
  
//   resultadosTests["Resistencia"] = result;

}




void InjectorController::calculateDutyCycle()
{

  uint64_t periodUs;
  periodUs = 1000000 / this->frequency;

  if (this->pulseWidthUs > periodUs)
  {
    Serial.println("Error: Pulse width > Period");
    stop();
    return;
  }
  
  uint32_t duty = (this->pulseWidthUs * this->frequency * (1ULL << this->resolution)) / 1000000ULL;
  duty = min(duty, (uint32_t)((1ULL << this->resolution) - 1)); // ¡Corrección aquí!
  ledcWrite(this->channel, duty);

}

void InjectorController::activate(float freq, unsigned long pulseWidthUs, unsigned long testDurationSec)
{
  this->frequency = freq;
  this->pulseWidthUs = pulseWidthUs;
  this->testDurationMs = testDurationSec * 1000;

  // Configuramos el PWM
  ledcSetup(this->channel, this->frequency, this->resolution);

  uint32_t maxDuty = (1ULL << this->resolution) - 1;

  // Si está en 0% de ciclo de trabajo (desactivado)
  if (this->is_activate_min)
  {
    ledcWrite(this->channel, 0);  // PWM en 0%
    Serial.println("Inyector NO activado (PWM con ciclo de trabajo del 0%).");
    isActive = true;
    startTime = millis();  // Registrar el tiempo aunque esté desactivado
    return;
  }

  // Si está en 100% de ciclo de trabajo (máximo activado)
  if (this->is_activate_max)
  {
    ledcWrite(this->channel, maxDuty);  // PWM en 100%
    Serial.println("Inyector ACTIVADO al máximo (PWM con ciclo de trabajo del 100%).");
  }
  else
  {
    // Configuración normal: se calcula el ciclo de trabajo según el ancho de pulso
    calculateDutyCycle();
    Serial.printf("Inyector ACTIVADO - F: %.1fHz, PW: %luµs, T: %lus\n",
                  frequency, pulseWidthUs, testDurationSec);
  }

  Serial.println("hola estoy aquui");

  this->isActive = true;
  this->startTime = millis();  // Registrar el tiempo de activación
  return;
}



void InjectorController::stop_bomba()
{
  digitalWrite(BOMBA_PIN, HIGH);
}

void InjectorController::begin_bomba()
{
  digitalWrite(BOMBA_PIN, HIGH);
}


void InjectorController::update()
{
  if (isActive && (millis() - startTime >= testDurationMs))
  {
    stop();
  }
}

void InjectorController::stop()
{
  this->is_activate_max=false;
  this->is_activate_min= false;
  ledcWrite(channel, 0);
  isActive = false;
  // Serial.println("Inyector DESACTIVADO");
}

bool InjectorController::isInjectorActive() const
{
  return isActive;
}
