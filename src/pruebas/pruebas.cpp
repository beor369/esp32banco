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


// // Pines
// #define PIN_INYECTOR 4      // Control MOSFET
// #define PIN_SONIDO 7        // Sensor de sonido
// #define PIN_BUZZER 19       // Buzzer de feedback

// Variables para la medición
unsigned long pulseStartTime = 0;
bool pulseSent = false;
bool measurementDone = false;
unsigned long activationTime = 0;

// Umbral de corriente en amperios para detectar el inicio de la eyección
float currentThreshold = 0.05;  // Ajusta este valor según las características de tu inyector
// Variables
unsigned long tiempoInicio = 0;
float corriente = 0;
int lecturaSonido = 0;
// Número de muestras para la prueba
const int numSamples = 100;
const int numCycles  = 5;    // Número de ciclos a realizar
// Variables
unsigned long tiempoApertura = 0;
unsigned long tiempoCierre = 0;
int thermoDO = 11;
int thermoCS = 45;
int thermoCLK = 48;
unsigned long buzzerStarTime;
bool buzzerActive;
unsigned long buzzerStartTime;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
std::map<std::string, TestResult> resultadosTests;
// Parámetros del inyector para el cálculo
float Q_inyector = 200.0; // Caudal nominal en cc/min
int n = 4;                // Factor de inyección (ajústalo según tu motor)

// Área de la probeta en cm² para convertir la diferencia de altura en volumen (V = Δh × área)
float areaProbeta = 10.0; // Ejemplo: 10 cm²

// Función para calcular el combustible teórico (en cc) usando la fórmula adaptada a segundos:
// Fórmula original en minutos:
//    Combustible (cc) = ((Q * PW_ms) / 60000) * ((RPM * T_min) / n)
// Como T_min = T_sec/60, se tiene:
//    Combustible (cc) = ((Q * PW_ms) / 60000) * ((RPM * T_sec) / (n * 60))
float calcularCombustible(float Q, float pulseWidth_ms, float RPM, int n, float tiempoSec)
{
  return ((Q * pulseWidth_ms) / 60000.0) * ((RPM * tiempoSec) / (n * 60.0));
}
// IMPLEMENTACIÓN DE LAS PRUEBAS (estas funciones pueden basarse en lecturas ADC, etc.)
// Los siguientes ejemplos son esquemáticos y deberán ajustarse según tus sensores y calibración.

// 1. Prueba de Resistencia Eléctrica
void testResistencia()
{
  inyector.assess_ina();
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
  TestResult result;

  // se llama para evaluar el valor medido
  float distance = ultrasonic_sensor.get_distance_fashion(20, false);
  TestResult results;
  results.expectedValue = distance;
  results.measuredValue = ultrasonic_sensor.distaceFashion;
  Serial.println(ultrasonic_sensor.getDistance());
  Serial.println(distance);
  // Serial.println(results.expectedValue);
  Serial.println(results.measuredValue);
  if (ultrasonic_sensor.distaceFashion == distance)
  {
    u8g2.clearBuffer();
    Serial.println("¡ADVERTENCIA: FUGAS !");
    result.passed = false;
    // Aquí puedes agregar acciones adicionales, por ejemplo, detener el sistema o activar una alarma.
    u8g2.drawStr(0, 25, "¡ADVERTENCIA: FUGAS !");

    u8g2.sendBuffer();
    delay(100);
  }
  else
  {
    u8g2.clearBuffer();
    result.passed = true;
    Serial.println(" INYECTOR BIEN! .");
    u8g2.drawStr(0, 25, "¡INYECTOR OK!");
    u8g2.sendBuffer();
    delay(100);
  }

  delay(2000);

  resultadosTests["fugas"] = result;
}

// 3. Prueba de Sonido de Activación
void testSonido()
{
  float cvSum = 0.0;       // Suma total de los CV de cada ciclo
  int abnormalCycles = 0;  // Contador de ciclos que superan el umbral
  const float umbralAlto = 20.0; // Umbral de CV en porcentaje (20% en este ejemplo)
  
  Serial.println("Inicio de ciclos de medición...");

  // Realiza "numCycles" ciclos de medición
  for (int cycle = 0; cycle < numCycles; cycle++) {
    int muestras[numSamples];
    unsigned long suma = 0;
    Serial.print("Ciclo ");
    Serial.println(cycle + 1);

    // Captura de muestras para este ciclo
    for (int i = 0; i < numSamples; i++) {
      muestras[i] = analogRead(PIN_SONIDO);
      suma += muestras[i];
      Serial.print("  Muestra ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(muestras[i]);
      delay(10); // Ajusta el retardo según la frecuencia deseada
    }

    // Cálculo de la media de este ciclo
    float media = (float)suma / numSamples;
    
    // Cálculo de la varianza y la desviación estándar
    float varianza = 0.0;
    for (int i = 0; i < numSamples; i++) {
      varianza += pow(muestras[i] - media, 2);
    }
    varianza /= numSamples;
    float desviacionEstandar = sqrt(varianza);
    
    // Cálculo del coeficiente de variación (CV)
    float cycleCV = (media != 0) ? (desviacionEstandar / media) * 100.0 : 0.0;
    cvSum += cycleCV;
    
    // Imprimir resultados del ciclo en el Monitor Serial
    Serial.print("  Media: ");
    Serial.println(media, 2);
    Serial.print("  Desviacion Std: ");
    Serial.println(desviacionEstandar, 2);
    Serial.print("  Coeficiente de Variacion: ");
    Serial.print(cycleCV, 2);
    Serial.println(" %");
    
    if (cycleCV > umbralAlto) {
      Serial.println("  Estado: INYECTOR MAL");
      abnormalCycles++;
    } else {
      Serial.println("  Estado: INYECTOR OK");
    }
    Serial.println("-----------------------------");
    delay(1000); // Breve pausa entre ciclos
  }

  // Calcular resultados globales
  float averageCV = cvSum / numCycles;
  Serial.println("Resultados Globales:");
  Serial.print("  Promedio de CV: ");
  Serial.print(averageCV, 2);
  Serial.println(" %");
  Serial.print("  Ciclos Anormales: ");
  Serial.print(abnormalCycles);
  Serial.print(" de ");
  Serial.println(numCycles);
  
  // Definir criterio final:
  // Por ejemplo, si más de la mitad de los ciclos indican fallo, el inyector se considera malo.
  String finalResult;
  if (abnormalCycles > numCycles / 2) {
    finalResult = "INYECTOR: MAL";
  } else {
    finalResult = "INYECTOR: OK";
  }
  
  Serial.println(finalResult);
  
  // Mostrar resultados en la pantalla OLED
  u8g2.clearBuffer();
  u8g2.setCursor(0, 12);
  u8g2.print("CV Promedio: ");
  u8g2.print(averageCV, 2);
  u8g2.print(" %");
  u8g2.setCursor(0, 30);
  u8g2.print("Ciclos mal: ");
  u8g2.print(abnormalCycles);
  u8g2.print("/");
  u8g2.print(numCycles);
  u8g2.setCursor(0, 48);
  u8g2.print(finalResult);
  u8g2.sendBuffer();

  // Espera antes de repetir la serie de ciclos
  delay(10000);

}
// 4. Prueba de Corriente de Activación
void testCorriente()
{
  TestResult result;
  float current = ina219.getCurrent_mA(); // en mA
  float shuntVoltage = ina219.getShuntVoltage_mV(); // en mV
  // Imprimir valores medidos con 4 decimales (puedes ajustar el número según prefieras)
  Serial.print("Shunt Voltage: "); Serial.print(shuntVoltage, 4); Serial.println(" mV");
  // Convierte el shunt voltage de mV a V
  float v_shunt_V = shuntVoltage / 1000.0;
  Serial.print("Current:       ");
  Serial.print(current, 4);
  Serial.println(" mA");
  // Calcula la corriente usando el valor del shunt resistor (0.1 Ω)
  float current_A = v_shunt_V / 0.1;
  Serial.print("current_A: ");
  Serial.println(current_A, 8); // 8 decimales
  InjectorData inyec = globalInjector.getInjector();
  result.expectedValue = inyec.corrienteActivacion;
  result.measuredValue = current_A;
  Serial.println(current_A, 8); // 8 decimales
  if (inyec.corrienteActivacion > (current_A - 0.08) && inyec.corrienteActivacion < (current_A + 0.08))
  {
    result.passed = true;
  }
  else
  {
    result.passed = false;
  }

  resultadosTests["corrienteActivacion"] = result;
}


// 5. Prueba de Tiempo de Respuesta
void testTiempoRespuesta()
{
  Serial.println("Enviando pulso...");
  
  unsigned long tiempoInicio = micros();
  digitalWrite(INYECTOR_PIN, HIGH);
  
  // Espera hasta que la corriente supere el umbral
  while (ina219.getCurrent_mA() < currentThreshold) {
    // espera activa
  }
  
  unsigned long tiempoActivacion = micros() - tiempoInicio;
  Serial.print("Tiempo de activación: ");
  Serial.print(tiempoActivacion);
  Serial.println(" us");
  
  digitalWrite(INYECTOR_PIN, LOW);
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  
  u8g2.drawStr(0, 15, "Tiempo de activacion:");
  char buffer[20];
  sprintf(buffer, "%lu us", tiempoActivacion);
  u8g2.drawStr(0, 35, buffer);
  // Evaluamos si el inyector esta bueno
  const unsigned long tiempoMin = 1000;  // 1 ms
  const unsigned long tiempoMax = 2000;  // 2 ms
  const char* resultado;

  if (tiempoActivacion >= tiempoMin && tiempoActivacion <= tiempoMax) {
    resultado = "Inyector BUENO";
  } else {
    resultado = "Inyector MALO";
  }

  u8g2.drawStr(0, 48, resultado);
  u8g2.sendBuffer();

}

// 6. Prueba de Caudal de Combustibles
void testCaudal()
{
  TestResult result;

  // Medición inicial del nivel (distancia en cm) en la probeta
  float distanciaInicial = ultrasonic_sensor.get_distance_fashion(5, true);
  Serial.print("Distancia inicial: ");
  Serial.print(distanciaInicial);
  Serial.println(" cm");

  inyector.rpmValue_tem = 0;
  controlBombaDurantePrueba(3, 10, testTimeValue, setupvalues, estate_inyector::ACTIVA_INYECTOR_CON_REVOLUCION);

  inyector.stop();
  u8g2.clearBuffer();
  // Medición final del nivel (después de la inyección)
  float distanciaFinal = ultrasonic_sensor.get_distance_fashion(5, false);
  Serial.print("Distancia final: ");
  Serial.print(distanciaFinal);
  Serial.println(" cm");

  // Cálculo del volumen medido (cc) a partir de la diferencia de altura
  float deltaAltura = distanciaInicial - distanciaFinal;
  float volumenMedido = deltaAltura * areaProbeta;
  Serial.print("Volumen medido (cc): ");
  Serial.println(volumenMedido);

  // Conversión de los valores externos para el cálculo:
  // pulseWidthValue: base de 10 ms → valor real en ms:
  float pulseWidth_ms = pulseWidthValue * 10.0;
  // rpmValue: base de 1000 RPM → valor real:
  float RPM_real = rpmValue * 1000.0;
  // testTimeValue: base de 10,000 ms → tiempo real en segundos:
  float tiempoSec = (testTimeValue * 10000.0) / 1000.0; // Esto equivale a testTimeValue * 10

  // Cálculo del combustible teórico inyectado (cc)
  float combustibleCalculado = calcularCombustible(Q_inyector, pulseWidth_ms, RPM_real, n, tiempoSec);
  Serial.print("Combustible calculado (cc): ");
  Serial.println(combustibleCalculado);
  float measuredFlow = combustibleCalculado; // Ejemplo
  result.measuredValue = volumenMedido;
  float tolerance = 0.05;
  float lower = combustibleCalculado * (1.0 - tolerance);
  float upper = combustibleCalculado * (1.0 + tolerance);
  result.passed = (measuredFlow >= lower && measuredFlow <= upper);
  resultadosTests["caudal"] = result;
}

// 7. Monitoreo de Temperatura
void monitorTemperatura(float maxTemp)
{
  char outputi[50];
  float UMBRAL_TEMP = 32.0;
  TestResult result;
  // Lee la temperatura en grados Celsius
  float temperatura = thermocouple.readCelsius();
  result.measuredValue = temperatura;
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  // Verifica si la temperatura supera el umbral
  if (temperatura >= UMBRAL_TEMP)
  {
    u8g2.clearBuffer();
    Serial.println("¡ADVERTENCIA: INYECTOR SOBRECALENTADO!");
    result.passed = false;
    // Aquí puedes agregar acciones adicionales, por ejemplo, detener el sistema o activar una alarma.
    u8g2.drawStr(0, 18, "¡ADVERTENCIA: INYECTOR SOBRECALENTADO!");
    u8g2.sendBuffer();
    delay(100);
  }
  else
  {
    u8g2.clearBuffer();
    result.passed = true;
    Serial.println(" INYECTOR bien! .");
    u8g2.drawStr(0, 18, "¡INYECTOR OK!");
    u8g2.sendBuffer();
    delay(100);
  }
  sprintf(outputi, "temperatura: %.2f %s", thermocouple.readCelsius());

  u8g2.clearBuffer();
  u8g2.drawStr(0, 18, outputi);
  u8g2.sendBuffer();
  delay(1000);

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
  testCorriente();
}

void runTestTiempoRespuesta()
{
  testTiempoRespuesta();
  // Se usa el valor ideal para apertura (y cierre, para simplificar)
  // testTiempoRespuesta(selected.tiempoRespuesta, selected.tiempoRespuesta);
}

void runTestCaudal()
{
  testCaudal(); // return testCaudal(selected.caudal, 0.05, 60000); // tolerancia del 5%, prueba de 1 minuto
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