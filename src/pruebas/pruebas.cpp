#include "pruebas.h"
#include <Arduino.h>
#include "saveInjector/saveInjector.h"
#include "ultrasonicsensor/ultrasonic_sensor.h"
#include "GlobalVarials/GlobalVarials.h"
#include "Menu/Menu.h"
#include "config.h"
#include "selectrpm/selectrpm.h"

#include "max6675.h"
#include <map>
#include <string>
int thermoDO = 11;
int thermoCS = 45;
int thermoCLK = 48;
unsigned long buzzerStarTime;
bool buzzerActive;
unsigned long buzzerStartTime;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
std::map<std::string, TestResult> resultadosTests;

// -----------------------------------------------------------------------------
// IMPLEMENTACIÓN DE LAS PRUEBAS (estas funciones pueden basarse en lecturas ADC, etc.)
// Los siguientes ejemplos son esquemáticos y deberán ajustarse según tus sensores y calibración.
// -----------------------------------------------------------------------------

// 1. Prueba de Resistencia Eléctrica
void testResistencia()
{
  // TestResult result;
  // // Simula lectura de ADC y cálculo de resistencia
  // float measuredResistance = nominal + random(-2, 3); // Ejemplo: valor medido
  // result.measuredValue = measuredResistance;
  // float lower = nominal * (1.0 - tolerance);
  // float upper = nominal * (1.0 + tolerance);
  // result.passed = (measuredResistance >= lower && measuredResistance <= upper);
  // delay(10000);
  // estadoActual = SUBMENU_MANUAL;

  // resultadosTests["Resistencia"] = result;
  // return result;
}

// 2. Prueba de Fugas en Reposo
void testFugas()
{
    //se llama para evaluar el valor medido
    float distance= ultrasonic_sensor.get_distance_fashion(20, false);
    TestResult results;
    results.expectedValue = distance;
    results.measuredValue= ultrasonic_sensor.distaceFashion;
    if (ultrasonic_sensor.distaceFashion == distance)
    {
      results.passed=true;
    }else
    {
      results.passed=false;
    }

    resultadosTests["fugas"]=results;
  

}

// 3. Prueba de Sonido de Activación
void testSonido()
{
  // TestResult result;
  // // Simula lectura de ADC del micrófono
  // // inyector.activate(50.0, 2500, 5);
  // int adcVal = analogRead(PIN_SONIDO); // Lee el valor ADC del sensor
  // result.measuredValue = adcVal;
  // result.passed = (adcVal >= threshold);
  // return result;
}

// 4. Prueba de Corriente de Activación
void testCorriente(float nominal, float minVal, float maxVal, unsigned long sampleDuration)
{
  TestResult result;
  // Simula la medición de corriente (en A)
  float measuredCurrent = nominal; // Ejemplo
  result.measuredValue = measuredCurrent;
  result.passed = (measuredCurrent >= minVal && measuredCurrent <= maxVal);
}

// 5. Prueba de Tiempo de Respuesta
void testTiempoRespuesta(unsigned long maxOpenTime, unsigned long maxCloseTime)
{
  TestResult result;
  // Simula la medición del tiempo de activación (en ms)
  unsigned long measuredTime = maxOpenTime - 1; // Ejemplo
  result.measuredValue = measuredTime;
  result.passed = (measuredTime < maxOpenTime);
  // return result;
}

// 6. Prueba de Caudal de Combustible
void testCaudal(float nominal, float tolerance, unsigned long duration)
{
  TestResult result;
  // Simula el cálculo de caudal en cc/min
  float measuredFlow = nominal; // Ejemplo
  result.measuredValue = measuredFlow;

  float lower = nominal * (1.0 - tolerance);
  float upper = nominal * (1.0 + tolerance);
  result.passed = (measuredFlow >= lower && measuredFlow <= upper);




  resultadosTests["Temperatura"] = result;
  // resultadosTests["Temperatura"].expectedValue;
  // resultadosTests["Temperatura"].measuredValue;
  // resultadosTests["Temperatura"].passed;
  // return result;
}

// 7. Monitoreo de Temperatura
void monitorTemperatura(float maxTemp)
{
  char outputi[50];
  float UMBRAL_TEMP = 32.0;
  TestResult result;
  // Lee la temperatura en grados Celsius
  float temperatura = thermocouple.readCelsius();
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  // Verifica si la temperatura supera el umbral
  if (temperatura >= UMBRAL_TEMP)
  {
    Serial.println("¡ADVERTENCIA: INYECTOR SOBRECALENTADO!");
    result.passed = false;
    // Aquí puedes agregar acciones adicionales, por ejemplo, detener el sistema o activar una alarma.
  }
  else
  {
    result.passed = true;
    Serial.println(" INYECTOR bien! .");
  }
  sprintf(outputi, "Fugas: %.2f %s", thermocouple.readCelsius());
  // Test de Resistencia
  u8g2.clearBuffer();
  u8g2.drawStr(0, 18, outputi);
  u8g2.sendBuffer();
  delay(1000);

  // float measuredTemp = maxTemp - 5; // Ejemplo
  // result.measuredValue = measuredTemp;
  // result.passed = (measuredTemp < maxTemp);
  resultadosTests["temperatura"] = result;
}

// -----------------------------------------------------------------------------
// Funciones modulares que usan los datos del inyector seleccionado como base.
// -----------------------------------------------------------------------------

void runTestResistencia()
{
  return testResistencia(); // ±10%
}

void runTestFugas()
{
  testFugas(); // Se asume que selected.fugas define el umbral
}

void runTestSonido()
{

  return testSonido(); // Se usa el valor ideal como umbral
}

void runTestCorriente()
{
  // float minCorr = selected.corrienteActivacion * 0.8;
  // float maxCorr = selected.corrienteActivacion * 1.2;
  // return testCorriente(selected.corrienteActivacion, minCorr, maxCorr, 100);
}

void runTestTiempoRespuesta()
{
  // Se usa el valor ideal para apertura (y cierre, para simplificar)
  //testTiempoRespuesta(selected.tiempoRespuesta, selected.tiempoRespuesta);
}

void runTestCaudal()
{
  // return testCaudal(selected.caudal, 0.05, 60000); // tolerancia del 5%, prueba de 1 minuto
}

void runTestTemperatura()
{
    monitorTemperatura(80.0); // Umbral fijo de 80°C
}

void activarBuzzer()
{
  // Código para activar el buzzer
  // tone(8, 1000, 500); // Ejemplo: Sonido en pin 8, 1000 Hz por 500ms
}
// Modo Automático: se ejecutan todas las pruebas en secuencia.
void mostrarResultado(const char *nombre, TestResult result)
{
  char output[50];
  snprintf(output, sizeof(output), "%s: %.2f %s", nombre, result.measuredValue, result.passed ? "OK" : "FALLA");

  u8g2.clearBuffer();
  u8g2.drawStr(0, 12, output);
  u8g2.sendBuffer();
  delay(1000);

  if (!result.passed)
  {
    activarBuzzer(); // Llamada a la función del buzzer en caso de falla
  }
}


void beep()
{
  if (!buzzerActive)
  {
    digitalWrite(BUZZER_PIN, HIGH); // Activa el buzzer
    buzzerStartTime = millis();     // Guarda el tiempo de inicio
    buzzerActive = true;            // Marca como activo
  }
}

// Función para actualizar el estado del buzzer (llamar en loop())
void updateBuzzer()
{
  if (buzzerActive && (millis() - buzzerStartTime >= 1000))
  {
    digitalWrite(BUZZER_PIN, LOW); // Apaga el buzzer después de 1 segundo
    buzzerActive = false;          // Marca como inactivo
  }
}

void setupbit()
{
  pinMode(BUZZER_PIN, OUTPUT);   // Configura el pin como salida
  digitalWrite(BUZZER_PIN, LOW); // Asegura que el buzzer inicia apagado
}
