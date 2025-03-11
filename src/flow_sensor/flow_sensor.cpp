#include "flow_sensor/flow_sensor.h"
#include <math.h>
#include "selectrpm/selectrpm.h"
#include "config.h"
#include "InjectorController/InjectorController.h"
#include "menu/Menu.h"
// Inicialización del puntero estático para la instancia.
FlowSensor* FlowSensor::_instance = nullptr;

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

void FlowSensor::begin() {
  pinMode(_pin, INPUT);
  attachInterrupt(digitalPinToInterrupt(_pin), pulseISR, RISING);
  _lastMeasurementTime = millis();
}

void IRAM_ATTR FlowSensor::pulseISR() {
  if (_instance != nullptr) {
    _instance->_pulseCount++;
  }
}

void FlowSensor::update() {
  unsigned long currentTime = millis();
  // Verifica si ha transcurrido el intervalo configurado
  if (currentTime - _lastMeasurementTime >= _measurementInterval) {
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

float FlowSensor::getFlowRate() {
  return _lastFlowRate;
}

float FlowSensor::getFilteredFlow() {
  return _filteredFlow;
}

bool FlowSensor::isLeakDetected() {
  return _filteredFlow > _leakThreshold;
}
// Calcula el volumen esperado en ml
// RPM: revoluciones por minuto del motor
// injPerRev: número de inyecciones por revolución
// testTimeSec: tiempo de prueba en segundos
// pulseWidthUs: ancho de pulso en microsegundos
// K: constante (ml/µs) que indica el volumen inyectado por µs de activación
float calculateExpectedVolume(float RPM, float injPerRev, float testTimeSec, float pulseWidthUs, float K) {
    // Frecuencia de inyección (inyecciones por segundo)
    float frequency = (RPM / 60.0) * injPerRev;
    // Volumen por inyección en ml (suponiendo relación lineal)
    float volumePerInjection = K * pulseWidthUs;
    // Volumen total esperado = (inyecciones/seg) * tiempo * volumen por inyección
    float expectedVolume = frequency * testTimeSec * volumePerInjection;
    return expectedVolume;
}

// Convierte el caudal (L/min) obtenido por el sensor a volumen (ml) durante el tiempo de prueba
float calculateSensorVolume(float flowRateLmin, float testTimeSec) {
    // 1 L = 1000 ml, 1 min = 60 s
    float flowRate_ml_per_sec = (flowRateLmin * 1000.0) / 60.0;
    return flowRate_ml_per_sec * testTimeSec;
}

// Compara el volumen esperado con el volumen medido por el sensor
// tolerancePercent: tolerancia permitida en porcentaje respecto al valor esperado
bool compareVolumes(float expectedVolume, float sensorVolume, float tolerancePercent) {
    float allowedDifference = expectedVolume * (tolerancePercent / 100.0);
    return (fabs(sensorVolume - expectedVolume) <= allowedDifference);
}
void setupvalues() {
    
    // frecuencia = rpmValue / 120.0f;          // RPM → Hz (4 tiempos)
    // anchoPulsoUs = pulseWidthValue * 1000; // ms → µs
    // tiempoPruebaSec = testTimeValue / 1000; // ms → segundos
    // extern float frecuencia;
    // extern unsigned long anchoPulsoUs;
    // extern unsigned long tiempoPruebaSec;  
    // Parámetros de ejemplo (puedes editar estos valores según tu prueba)
    // float RPM = frecuencia;           // RPM del motor
    // float injPerRev = 2.0;          // Número de inyecciones por revolución
    // float testTimeSec = tiempoPruebaSec;        // Tiempo de prueba en segundos
    // float pulseWidthUs = anchoPulsoUs;    // Ancho de pulso en microsegundos
    // float K = 0.00001;            // Constante de calibración (ej.: 0.00001 ml/µs)
    
    float RPM = 3000.0;           // RPM del motor
    float injPerRev = 2.0;          // Número de inyecciones por revolución
    float testTimeSec = 5.0;        // Tiempo de prueba en segundos
    float pulseWidthUs = 1000.0;    // Ancho de pulso en microsegundos
    float K = 0.00001;            // Constante de calibración (ej.: 0.00001 ml/µs)
    
    // Volumen esperado calculado según la fórmula
    float expectedVolume = calculateExpectedVolume(RPM, injPerRev, testTimeSec, pulseWidthUs, K);
    
    // Supongamos que el sensor mide un caudal de 2.5 L/min
    float sensorFlowRateLmin = 2.5;
    float sensorVolume = calculateSensorVolume(sensorFlowRateLmin, testTimeSec);

    Serial.print("Volumen esperado: ");
    Serial.print(expectedVolume, 4);
    Serial.println(" ml");

    Serial.print("Volumen medido por sensor: ");
    Serial.print(sensorVolume, 4);
    Serial.println(" ml");

    // Definimos una tolerancia, por ejemplo, 10%
    float tolerancePercent = 10.0;
    if (compareVolumes(expectedVolume, sensorVolume, tolerancePercent)) {
        Serial.println("El volumen medido está dentro del rango de tolerancia.");
    } else {
        Serial.println("El volumen medido no coincide con el esperado.");
    }
}
