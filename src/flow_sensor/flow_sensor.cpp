#include "flow_sensor/flow_sensor.h"
#include <math.h>
#include "selectrpm/selectrpm.h"
#include "config.h"
#include "InjectorController/InjectorController.h"
#include "menu/Menu.h"

// Inicialización del puntero estático para la instancia.
// FlowSensor sensor(4);

FlowSensor *FlowSensor::_instance = nullptr;

FlowSensor::FlowSensor(uint8_t pin, float calibrationFactor,
                       unsigned long measurementInterval,
                       float leakThreshold, float alpha)
    : _pin(pin), _calibrationFactor(calibrationFactor),
      _measurementInterval(measurementInterval), _leakThreshold(leakThreshold),
      _alpha(alpha), _pulseCount(0), _lastMeasurementTime(0),
      _lastFlowRate(0.0), _filteredFlow(0.0)
{
  // Se guarda la referencia a la instancia (asume que habrá solo una)
  _instance = this;
}

void FlowSensor::begin()
{
  pinMode(_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(_pin), pulseISR, RISING);
  _lastMeasurementTime = millis();
}

void IRAM_ATTR FlowSensor::pulseISR()
{
  if (_instance != nullptr)
  {
    _instance->_pulseCount++;
  }
}

void FlowSensor::update()
{
  unsigned long currentTime = millis();
  // Verifica si ha transcurrido el intervalo configurado
  if (currentTime - _lastMeasurementTime >= _measurementInterval)
  {
    // Toma el total de pulsos acumulados y resetea el contador
    unsigned long pulses = _pulseCount;
    _pulseCount = 0;

    // Calcula los pulsos por segundo
    float pulsesPerSecond = pulses / (_measurementInterval / 1000.0);
    // Calcula el caudal en L/min según el factor de calibración
    _lastFlowRate = pulsesPerSecond / _calibrationFactor;
    // Actualiza el filtro exponencial (media móvil)
    _filteredFlow = _alpha * _lastFlowRate + (1 - _alpha) * _filteredFlow;
    _lastMeasurementTime = currentTime;
  }
}

float FlowSensor::getFlowRate()
{
  return _lastFlowRate;
}

float FlowSensor::getFilteredFlow()
{
  return _filteredFlow;
}

bool FlowSensor::isLeakDetected()
{
  return _filteredFlow > _leakThreshold;
}
// Calcula el volumen esperado en ml
// RPM: revoluciones por minuto del motor
// injPerRev: número de inyecciones por revolución
// testTimeSec: tiempo de prueba en segundos
// pulseWidthUs: ancho de pulso en microsegundos
// K: constante (ml/µs) que indica el volumen inyectado por µs de activación

// unsigned long calculateExpectedVolume(unsigned long RPM, unsigned long injPerRev, unsigned long testTimeSec, unsigned long pulseWidthUs, unsigned long K) {
//     // Frecuencia de inyección (inyecciones por segundo)
//     unsigned long frequency = (RPM / 60.0) * injPerRev;
// unsigned long    unsigned long volumePerInjection = K * pulseWidthUs;
//     // Volumen total esperado = (inyecciones/seg) * tiempo * volumen por inyección
//     unsigned long expectedVolume = frequency * testTimeSec * volumePerInjection;
//     return expectedVolume;
// }
// frecuencia = rpmValue / 120.0f;          // RPM → Hz (4 tiempos)
// anchoPulsoUs = pulseWidthValue * 1000; // ms → µs
// tiempoPruebaSec = testTimeValue / 1000; // ms → segundos

//  long calculateExpectedVolume(long frecuencia, unsigned long pulseWidthValue, unsigned long tiempoPruebaSec) {
//    long injPerRev = 2.0;          // Número de inyecciones por revolución
//   //  unsigned long testTimeSec = tiempoPruebaSec;        // Tiempo de prueba en segundos
//   //  unsigned long pulseWidthUs = anchoPulsoUs;    // Ancho de pulso en microsegundos
//  long K = 0.00001;            // Constante de calibración (ej.: 0.00001 ml/µs)

//    Serial.println("//////////////////// values calculateExpectedVolume injector//////////");
//    Serial.println(frecuencia);
//    Serial.println(anchoPulsoUs);
//    Serial.println(tiempoPruebaSec);
//    Serial.println("//////////////////// values calculateExpectedVolume injector end//////////");

//   // Frecuencia de inyección (inyecciones por segundo)
//    long frequency = (frecuencia / 60) * injPerRev;
//    long volumePerInjection = K * pulseWidthValue;
//   Serial.println("//////////////////// debud frecuence//////////");
//   Serial.println(frequency);
//   Serial.println(volumePerInjection);
//   Serial.println("////////////////////debug frecuence//////////");

//   // Volumen total esperado = (inyecciones/seg) * tiempo * volumen por inyección
//    long expectedVolume = frequency * tiempoPruebaSec * volumePerInjection;
//   return expectedVolume;
// }
double calculateExpectedVolume(long frecuenciaa, unsigned long anchoPulsoUss, unsigned long tiempoPruebaSecc)
{
  double injPerRev = 2.0; // Número de inyecciones por revolución
  double K = 0.00001;     // Constante de calibración en ml/µs

  // Frecuencia de inyección (inyecciones por segundo)
  double frequency = (frecuenciaa / 60.0) * injPerRev; // Asegurar división en punto flotante
  double volumePerInjection = K * anchoPulsoUss;

  // Volumen total esperado = (inyecciones/seg) * tiempo * volumen por inyección
  double expectedVolume = frequency * tiempoPruebaSecc * volumePerInjection;

  return expectedVolume;
}

// Convierte el caudal (L/min) obtenido por el sensor a volumen (ml) durante el tiempo de prueba
unsigned long calculateSensorVolume()
{
  float sensorFlowRateLmin = sensor.getFilteredFlow();
  // 1 L = 1000 ml, 1 min = 60 s
  float flowRate_ml_per_sec = (sensorFlowRateLmin * 1000.0) / 60.0;

  return flowRate_ml_per_sec * tiempoPruebaSec;
}

// Compara el volumen esperado con el volumen medido por el sensor
// tolerancePercent: tolerancia permitida en porcentaje respecto al valor esperado
bool compareVolumes(float expectedVolume, float sensorVolume, float tolerancePercent)
{
  float allowedDifference = expectedVolume * (tolerancePercent / 100.0);
  return (fabs(sensorVolume - expectedVolume) <= allowedDifference);
}

void setupvalues()
{
    InyectorParametros values = inyector.devolver();

    double expectedVolume = calculateExpectedVolume(values.frecuenciaa, values.anchoPulsoUss, values.tiempoPruebaSecc);
    unsigned long sensorVolume = calculateSensorVolume();



    // Definimos una tolerancia del 10%
    unsigned long tolerancePercent = 10;

    TestResult results;
    results.measuredValue = sensorVolume;
    results.expectedValue = expectedVolume;
    results.passed = compareVolumes(expectedVolume, sensorVolume, tolerancePercent);

    // Verificar si "Caudal" ya existe en resultadosTests antes de asignar
    if (resultadosTests.find("Caudal") == resultadosTests.end())
    {
        resultadosTests["Caudal"] = results;
    }
    else
    {
        resultadosTests["Caudal"].measuredValue += sensorVolume;
    }
}
