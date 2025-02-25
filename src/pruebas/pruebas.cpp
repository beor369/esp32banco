#include "pruebas.h"
#include <Arduino.h>
 #include "saveInjector/saveInjector.h"
 #include "Menu/Menu.h"


// -----------------------------------------------------------------------------
// IMPLEMENTACIÓN DE LAS PRUEBAS (estas funciones pueden basarse en lecturas ADC, etc.)
// Los siguientes ejemplos son esquemáticos y deberán ajustarse según tus sensores y calibración.
// -----------------------------------------------------------------------------

// 1. Prueba de Resistencia Eléctrica  
TestResult testResistencia(float nominal, float tolerance) {
  TestResult result;
  // Simula lectura de ADC y cálculo de resistencia
  float measuredResistance = nominal + random(-2, 3);  // Ejemplo: valor medido
  result.measuredValue = measuredResistance;
  float lower = nominal * (1.0 - tolerance);
  float upper = nominal * (1.0 + tolerance);
  result.passed = (measuredResistance >= lower && measuredResistance <= upper);
  delay(10000);
  estadoActual = SUBMENU_MANUAL;

  return result;
}

// 2. Prueba de Fugas en Reposo  
TestResult testFugas(float maxFlow) {
  TestResult result;
  // Simula la medición de flujo en cc/min
  float measuredFlow = maxFlow - 0.05;  // Ejemplo
  result.measuredValue = measuredFlow;
  result.passed = (measuredFlow < maxFlow);
  return result;
}

// 3. Prueba de Sonido de Activación  
TestResult testSonido(float threshold) {
  TestResult result;
  // Simula lectura de ADC del micrófono
  int adcVal = random(0, 1024);
  result.measuredValue = adcVal;
  result.passed = (adcVal >= threshold);
  return result;
}

// 4. Prueba de Corriente de Activación  
TestResult testCorriente(float nominal, float minVal, float maxVal, unsigned long sampleDuration) {
  TestResult result;
  // Simula la medición de corriente (en A)
  float measuredCurrent = nominal; // Ejemplo
  result.measuredValue = measuredCurrent;
  result.passed = (measuredCurrent >= minVal && measuredCurrent <= maxVal);
  return result;
}

// 5. Prueba de Tiempo de Respuesta  
TestResult testTiempoRespuesta(unsigned long maxOpenTime, unsigned long maxCloseTime) {
  TestResult result;
  // Simula la medición del tiempo de activación (en ms)
  unsigned long measuredTime = maxOpenTime - 1;  // Ejemplo
  result.measuredValue = measuredTime;
  result.passed = (measuredTime < maxOpenTime);
  return result;
}

// 6. Prueba de Caudal de Combustible  
TestResult testCaudal(float nominal, float tolerance, unsigned long duration) {
  TestResult result;
  // Simula el cálculo de caudal en cc/min
  float measuredFlow = nominal; // Ejemplo
  result.measuredValue = measuredFlow;
  float lower = nominal * (1.0 - tolerance);
  float upper = nominal * (1.0 + tolerance);
  result.passed = (measuredFlow >= lower && measuredFlow <= upper);
  return result;
}

// 7. Monitoreo de Temperatura  
TestResult monitorTemperatura(float maxTemp) {
  TestResult result;
  // Simula lectura de temperatura en °C
  float measuredTemp = maxTemp - 5; // Ejemplo
  result.measuredValue = measuredTemp;
  result.passed = (measuredTemp < maxTemp);
  return result;
}

// -----------------------------------------------------------------------------
// Funciones modulares que usan los datos del inyector seleccionado como base.
// -----------------------------------------------------------------------------

TestResult runTestResistencia(InjectorData selected) {
  return testResistencia(selected.resistencia, 0.10); // ±10%

}

TestResult runTestFugas(InjectorData selected) {
  return testFugas(selected.fugas);  // Se asume que selected.fugas define el umbral
}

TestResult runTestSonido(InjectorData selected) {
  return testSonido(selected.sonidoActivacion); // Se usa el valor ideal como umbral
}

TestResult runTestCorriente(InjectorData selected) {
  float minCorr = selected.corrienteActivacion * 0.8;
  float maxCorr = selected.corrienteActivacion * 1.2;
  return testCorriente(selected.corrienteActivacion, minCorr, maxCorr, 100);
}

TestResult runTestTiempoRespuesta(InjectorData selected) {
  // Se usa el valor ideal para apertura (y cierre, para simplificar)
  return testTiempoRespuesta(selected.tiempoRespuesta, selected.tiempoRespuesta);
}

TestResult runTestCaudal(InjectorData selected) {
  return testCaudal(selected.caudal, 0.05, 60000); // tolerancia del 5%, prueba de 1 minuto
}

TestResult runTestTemperatura(InjectorData selected) {
  return monitorTemperatura(80.0);  // Umbral fijo de 80°C
}

// -----------------------------------------------------------------------------
// Funciones para ejecutar las pruebas en modos manual y automático.
// -----------------------------------------------------------------------------

//Modo Manual: el usuario elige qué prueba ejecutar, en el orden que desee.
//  void runTestsManual(InjectorData selected) {
//   const char* testOptions[] = {
//     "Resistencia",
//     "Fugas",
//     "Sonido",
//     "Corriente",
//     "Tiempo Resp.",
//     "Caudal",
//     "Temperatura",
//     "Salir"
//   };
//   const int numTestOptions = 8;
//   bool exitMenu = false;
  
//   while (!exitMenu) {
//     int sel = selectMenuOption("Test Manual", testOptions, numTestOptions);
//     TestResult result;
//     switch(sel) {
//       case 0:
//         result = runTestResistencia(selected);
//         break;
//       case 1:
//         result = runTestFugas(selected);
//         break;
//       case 2:
//         result = runTestSonido(selected);
//         break;
//       case 3:
//         result = runTestCorriente(selected);
//         break;
//       case 4:
//         result = runTestTiempoRespuesta(selected);
//         break;
//       case 5:
//         result = runTestCaudal(selected);
//         break;
//       case 6:
//         result = runTestTemperatura(selected);
//         break;
//       case 7:
//         exitMenu = true;
//         continue;
//     }
//     // Mostrar resultados en el monitor serial (puedes adaptarlo a OLED)
//     Serial.print(testOptions[sel]);
//     Serial.print(": Valor = ");
//     Serial.print(result.measuredValue);
//     Serial.println(result.passed ? " OK" : " FALLA");
//     delay(2000);
//   }
// }

// Modo Automático: se ejecutan todas las pruebas en secuencia.
void runTestsAutomatic(InjectorData selected) {
  TestResult result;
  
  result = runTestResistencia(selected);
  Serial.print("Resistencia: ");
  Serial.print(result.measuredValue);
  Serial.println(result.passed ? " OK" : " FALLA");
  delay(1000);
  
  result = runTestFugas(selected);
  Serial.print("Fugas: ");
  Serial.print(result.measuredValue);
  Serial.println(result.passed ? " OK" : " FALLA");
  delay(1000);
  
  result = runTestSonido(selected);
  Serial.print("Sonido: ");
  Serial.print(result.measuredValue);
  Serial.println(result.passed ? " OK" : " FALLA");
  delay(1000);
  
  result = runTestCorriente(selected);
  Serial.print("Corriente: ");
  Serial.print(result.measuredValue);
  Serial.println(result.passed ? " OK" : " FALLA");
  delay(1000);
  
  result = runTestTiempoRespuesta(selected);
  Serial.print("Tiempo Resp.: ");
  Serial.print(result.measuredValue);
  Serial.println(result.passed ? " OK" : " FALLA");
  delay(1000);
  
  result = runTestCaudal(selected);
  Serial.print("Caudal: ");
  Serial.print(result.measuredValue);
  Serial.println(result.passed ? " OK" : " FALLA");
  delay(1000);
  
  result = runTestTemperatura(selected);
  Serial.print("Temperatura: ");
  Serial.print(result.measuredValue);
  Serial.println(result.passed ? " OK" : " FALLA");
  delay(1000);
}
