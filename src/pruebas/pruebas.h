#ifndef PRUEBAS_H
#define PRUEBAS_H
#include <optional>
#include <map>
#include <string>


#include <Arduino.h>
#include "saveInjector/saveInjector.h"
 

// // Estructura para almacenar el resultado de una prueba
struct TestResult {
  std::optional<float> expectedValue;
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
void testResistencia();
void testFugas();
void testSonido();
void testCorriente(float nominal, float minVal, float maxVal, unsigned long sampleDuration);
void testTiempoRespuesta(unsigned long maxOpenTime, unsigned long maxCloseTime);
void testCaudal(float nominal, float tolerance, unsigned long duration);
void monitorTemperatura(float maxTemp);

// Declaraciones de funciones modulares que toman como base el inyector seleccionado
void runTestResistencia();
void runTestFugas();
void runTestSonido();
void runTestCorriente();
void runTestTiempoRespuesta();
void runTestCaudal();
void runTestTemperatura();

// Funciones para ejecutar las pruebas en modo manual y automático.
void setupbit();
void updateBuzzer();
void beep();
void mostrarResultado(const char *nombre, TestResult result);

extern std::map<std::string, TestResult> resultadosTests;
#endif  // PRUEBAS_H
