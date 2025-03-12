#ifndef FLOW_SENSOR_H
#define FLOW_SENSOR_H

#include <Arduino.h>
unsigned long calculateSensorVolume();
void setupvalues();
class FlowSensor {
public:
    // Constructor: asigna el pin, factor de calibración (pulsos por L/min), 
    // intervalo de medición en ms, umbral de fuga y parámetro de filtrado.
    FlowSensor(uint8_t pin, float calibrationFactor = 7.5, 
               unsigned long measurementInterval = 5000, 
               float leakThreshold = 0.5, float alpha = 0.3);


    // Inicializa el sensor y la interrupción.
    void begin();

    // formula para saber la cantidad de combustible


    // Llama periódicamente a update() para procesar los pulsos acumulados.
    void update();

    // Devuelve el caudal calculado (L/min) en la última medición.
    float getFlowRate();

    // Devuelve el caudal filtrado.
    float getFilteredFlow();

    // Indica si se detecta una fuga (basado en el umbral configurado).
    bool isLeakDetected();



private:
    uint8_t _pin;
    float _calibrationFactor;
    unsigned long _measurementInterval; // Intervalo en milisegundos
    float _leakThreshold;
    float _alpha;                       // Factor del filtro exponencial

    volatile unsigned long _pulseCount; // Contador de pulsos (modificado en la ISR)
    unsigned long _lastMeasurementTime;
    float _lastFlowRate;
    float _filteredFlow;

    // Para utilizar la ISR dentro de la clase (se asume una única instancia)
    static FlowSensor* _instance;
    static void IRAM_ATTR pulseISR();
};
extern FlowSensor sensor; // Declaración externa

#endif
