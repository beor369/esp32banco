#ifndef PRUEBAS_H
#define PRUEBAS_H

#include <Arduino.h>
#include "saveInjector/saveInjector.h"
// // Estructura para almacenar el resultado de una prueba
struct TestResult {
  bool passed;          // true si la prueba se aprueba
  float measuredValue;  // Valor medido (resistencia, corriente, tiempo, etc.)
};

// Declaración de la estructura para los datos de un inyector
// struct InjectorData {
//   char modelo[30];
//   float resistencia;
//   float caudal;
//   float fugas;
//   float tiempoRespuesta;
//   float corrienteActivacion;
//   float temperaturaOperativa;
//   float sonidoActivacion;
// };

// Declaraciones de las funciones de pruebas (cada una utiliza los valores ideales del inyector)
TestResult testResistencia(float nominal, float tolerance);
TestResult testFugas(float maxFlow);
TestResult testSonido(float threshold);
TestResult testCorriente(float nominal, float minVal, float maxVal, unsigned long sampleDuration);
TestResult testTiempoRespuesta(unsigned long maxOpenTime, unsigned long maxCloseTime);
TestResult testCaudal(float nominal, float tolerance, unsigned long duration);
TestResult monitorTemperatura(float maxTemp);

// Declaraciones de funciones modulares que toman como base el inyector seleccionado
TestResult runTestResistencia(InjectorData selected);
TestResult runTestFugas(InjectorData selected);
TestResult runTestSonido(InjectorData selected);
TestResult runTestCorriente(InjectorData selected);
TestResult runTestTiempoRespuesta(InjectorData selected);
TestResult runTestCaudal(InjectorData selected);
TestResult runTestTemperatura(InjectorData selected);

// Funciones para ejecutar las pruebas en modo manual y automático.
void runTestsManual(InjectorData selected);
void runTestsAutomatic(InjectorData selected);

#endif  // PRUEBAS_H
