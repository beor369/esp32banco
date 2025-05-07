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
#include <EEPROM.h>
#include "SRF05.h"
#include "InjectorController/InjectorController.h"
 

// #define PIN_INYECTOR 4      // Control MOSFET
// #define PIN_SONIDO 7        // Sensor de sonido
// #define PIN_BUZZER 19       // Buzzer de feedback
// unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200;
// Número de muestras para moda de distancia
const int sampleCount = 10;
// Variables para la medición
unsigned long pulseStartTime = 0;
bool pulseSent = false;
bool measurementDone = false;
unsigned long activationTime = 0;

// Umbral de corriente en amperios para detectar el inicio de la eyección
float currentThreshold = 0.05; // Ajusta este valor según las características de tu inyector
// Variables
unsigned long tiempoInicio = 0;
float corriente = 0;
int lecturaSonido = 0;
// Número de muestras para la prueba
const int numSamples = 100;
const int numCycles = 5; // Número de ciclos a realizar
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
// Parámetros nominales
// Parámetros nominales y medición
float Q_inyector_cc_min = 144.0f;  // caudal nominal cc/min
// Variables globales definidas en tu sketch:
//   float frequency        → frecuencia de inyección (Hz)
//   unsigned long pulseWidthUs → ancho de pulso (µs)
//   float testDurationSec  → duración de la prueba (s)

// Área de la probeta (cm²)
const float areaProbeta_cm2 = 15.2f;

// Calcula combustible teórico (cc) usando frecuencia en Hz:
// combustible = (Q_cc_min/60) [cc/s] * freq_Hz [pulsos/s] * pulseWidth_us/1e6 [s] * testDuration_s [s]
float calcularCombustible(
  float Q_cc_min,
  float freq_Hz,
  float pulseWidth_us,
  float testTime_s
) {
  float Q_cc_s      = Q_cc_min / 60.0f;        // cc/s
  float pulseTime_s = pulseWidth_us / 1e6f;    // µs → s
  return Q_cc_s * freq_Hz * pulseTime_s * testTime_s;
}
// float calcularCombustible(float QccMin, float freqHz, float pulseWidthUs, float durationSec) {
//   if (QccMin <= 0 || freqHz <= 0 || pulseWidthUs <= 0 || durationSec <= 0) return 0.0f;

//   // Ejemplo de fórmula:
//   float pulsosTotales = freqHz * durationSec;
//   float dutyCycle = pulseWidthUs / 1000000.0f * freqHz;  // ciclo útil como fracción
//   float flujoPorPulso = QccMin / 60.0f / freqHz;         // cc por pulso ideal

//   return flujoPorPulso * pulsosTotales;
// }

// IMPLEMENTACIÓN DE LAS PRUEBAS (estas funciones pueden basarse en lecturas ADC, etc.)
// Los siguientes ejemplos son esquemáticos y deberán ajustarse según tus sensores y calibración.
// sonidos

void playTone(int freq, int duration)
{
  if (freq <= 0)
  {
    delay(duration);
    return;
  }
  // atacha y genera PWM
  ledcAttachPin(BUZZER_PIN, BUZZER_CHAN);
  ledcWriteTone(BUZZER_CHAN, freq);
  delay(duration);
  // corta y libera
  ledcWriteTone(BUZZER_CHAN, 0);
  ledcDetachPin(BUZZER_PIN);
  // opcional: alta impedancia para ruido cero
  pinMode(BUZZER_PIN, INPUT);
}
// sonido breve para desplazamiento
void playMoveSound()
{
  playTone(800, 50);
}

// sonido para clic
void playClickSound()
{
  playTone(1200, 100);
}

// la melodía de GOT
static const int melody[] = {
    NOTE_G4, NOTE_C5, NOTE_DS5, NOTE_F5, NOTE_G5,
    NOTE_G4, NOTE_C5, NOTE_DS5, NOTE_F5, NOTE_G5,
    NOTE_G4, NOTE_C5, NOTE_DS5, NOTE_F5, NOTE_G5,
    NOTE_F5, NOTE_G5, NOTE_AS5, NOTE_A5, NOTE_G5,
    NOTE_F5, NOTE_G4, 0, NOTE_G4, NOTE_C5};

static const int durations[] = {
    500, 500, 250, 250, 500,
    500, 250, 250, 500, 500,
    250, 250, 500, 500, 250,
    250, 250, 250, 250, 500,
    250, 250, 500, 500, 500};

void GameOfThrones()
{
  // conecta pin al canal 1
  ledcAttachPin(BUZZER_PIN, BUZZER_CHAN);
  int n = sizeof(melody) / sizeof(melody[0]);
  for (int i = 0; i < n; i++)
  {
    int note = melody[i];
    int dur = durations[i];
    if (note == 0)
    {
      // silencio
      ledcWriteTone(BUZZER_CHAN, 0);
      delay(dur);
    }
    else
    {
      // emite la frecuencia
      ledcWriteTone(BUZZER_CHAN, note);
      delay(dur);
    }
    // corta entre notas para que se aprecien
    ledcWriteTone(BUZZER_CHAN, 0);
    delay(50);
  }
}

// 1) Sonido al iniciar la prueba: “moneda” de Mario
void playTestStartSound()
{
  static const int melody[] = {NOTE_C5, NOTE_E5, NOTE_G5};
  static const int durations[] = {50, 50, 200};
  for (size_t i = 0; i < sizeof(melody) / sizeof(melody[0]); i++)
  {
    playTone(melody[i], durations[i]);
    delay(20);
  }
}

// 2) Sonido al terminar la prueba: “power-up”
void playTestEndSound()
{
  static const int melody[] = {NOTE_C5, NOTE_C6};
  static const int durations[] = {150, 300};
  for (size_t i = 0; i < sizeof(melody) / sizeof(melody[0]); i++)
  {
    playTone(melody[i], durations[i]);
    delay(30);
  }
}

// 3) Sonido al mostrar resultados: “level clear” (primeros compases)
void playResultsSound()
{
  static const int melody[] = {
      NOTE_G5, NOTE_F5, NOTE_E5, NOTE_C5, NOTE_E5, NOTE_G5};
  static const int durations[] = {
      100, 100, 100, 200, 100, 300};
  for (size_t i = 0; i < sizeof(melody) / sizeof(melody[0]); i++)
  {
    playTone(melody[i], durations[i]);
    delay(20);
  }
}
// 1. Prueba de Resistencia Eléctrica
void testResistencia()
{
  //  inyector.assess_ina();
  // Lecturas del sensor INA219
  float shuntVoltage = ina219.getShuntVoltage_mV(); // en mV
  float busVoltage = ina219.getBusVoltage_V();      // en V
  float current = ina219.getCurrent_mA();           // en mA
  float power = ina219.getPower_mW();               // en mW

  // Imprimir valores medidos con 4 decimales (puedes ajustar el número según prefieras)
  Serial.print("Bus Voltage:   ");
  Serial.print(busVoltage, 4);
  Serial.println(" V");
  Serial.print("Shunt Voltage: ");
  Serial.print(shuntVoltage, 4);
  Serial.println(" mV");
  Serial.print("Current:       ");
  Serial.print(current, 4);
  Serial.println(" mA");
  Serial.print("Power:         ");
  Serial.print(power, 4);
  Serial.println(" mW");
  Serial.println("");
  // Convierte el shunt voltage de mV a V
  float v_shunt_V = shuntVoltage / 1000.0;
  // Calcula la corriente usando el valor del shunt resistor (0.1 Ω)
  float current_A = v_shunt_V / 0.1;
  // Calcula la resistencia del inyector usando el bus voltage
  float r_inyector = v_shunt_V / current_A;

  // Parámetros para ajuste de la medición
  // double current_A = current / 1000;
  Serial.print("current_A: ");
  Serial.println(current_A, 8); // 8 decimales
  Serial.print("r_inyector: ");
  Serial.println(r_inyector, 8); // 8 decimale
  double ohm = busVoltage / current_A;

  InjectorData inyec = globalInjector.getInjector();
  TestResult result;

  result.expectedValue = inyec.resistencia;
  result.measuredValue = ohm;
  Serial.println(current_A, 8); // 8 decimales
  Serial.print(ohm, 8);
  if (inyec.resistencia > (ohm - 2) && inyec.resistencia < (ohm + 2))
  {
    result.passed = true;
  }
  else
  {
    result.passed = false;
  }

  resultadosTests["resistencia"] = result;
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
  if (ultrasonic_sensor.distaceFashion != distance)
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
  float cvSum = 0.0;                 // Suma total de los CV de cada ciclo
  int abnormalCycles = 0;            // Ciclos con CV alto
  int clicsBajos = 0;                // Ciclos con clics muy bajos
  const float umbralCV = 2.0;        // Umbral de CV en porcentaje
  const float umbralClicBajo = 50.0; // Umbral mínimo de media de clics (ajustable)
  TestResult result;
  Serial.println("Inicio de ciclos de medición...");

  for (int cycle = 0; cycle < numCycles; cycle++)
  {
    int muestras[numSamples];
    unsigned long suma = 0;
    Serial.print("Ciclo ");
    Serial.println(cycle + 1);

    for (int i = 0; i < numSamples; i++)
    {
      muestras[i] = analogRead(PIN_SONIDO);
      suma += muestras[i];
      Serial.print("  Muestra ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(muestras[i]);
      delay(10);
    }

    float media = (float)suma / numSamples;
    float varianza = 0.0;

    for (int i = 0; i < numSamples; i++)
    {
      varianza += pow(muestras[i] - media, 2);
    }

    varianza /= numSamples;
    float desviacionEstandar = sqrt(varianza);
    float cycleCV = (media != 0) ? (desviacionEstandar / media) * 100.0 : 0.0;
    cvSum += cycleCV;

    Serial.print("  Media: ");
    Serial.println(media, 2);
    Serial.print("  Desviacion Std: ");
    Serial.println(desviacionEstandar, 2);
    Serial.print("  Coeficiente de Variacion: ");
    Serial.print(cycleCV, 2);
    Serial.println(" %");

    if (cycleCV > umbralCV)
    {
      Serial.println("  Estado: INYECTOR MAL (CV alto)");
      abnormalCycles++;
    }
    else if (media < umbralClicBajo)
    {
      Serial.println("  Estado: INYECTOR MAL (clics bajos)");
      clicsBajos++;
    }
    else
    {
      Serial.println("  Estado: INYECTOR OK");
    }

    Serial.println("-----------------------------");
    delay(1000);
  }

  float averageCV = cvSum / numCycles;
  Serial.println("Resultados Globales:");
  Serial.print("  Promedio de CV: ");
  Serial.print(averageCV, 2);
  Serial.println(" %");
  Serial.print("  Ciclos con CV alto: ");
  Serial.println(abnormalCycles);
  Serial.print("  Ciclos con clics bajos: ");
  Serial.println(clicsBajos);

  String finalResult;
  if (abnormalCycles > numCycles / 2 || clicsBajos > numCycles / 2)
  {
    finalResult = "INYECTOR: MAL";
    result.passed = false;
  }
  else
  {
    finalResult = "INYECTOR: OK";
    result.passed = true;
  }

  Serial.println(finalResult);

  u8g2.clearBuffer();
  u8g2.setCursor(0, 12);
  u8g2.print("CV Prom: ");
  u8g2.print(averageCV, 2);
  u8g2.print(" %");
  u8g2.setCursor(0, 30);
  u8g2.print("CV Mal: ");
  u8g2.print(abnormalCycles);
  u8g2.print(" CB: ");
  u8g2.print(clicsBajos);
  u8g2.setCursor(0, 48);
  u8g2.print(finalResult);
  u8g2.sendBuffer();

  delay(1000);
  resultadosTests["Sonido"] = result;
}

void testCorriente()
{
  TestResult result;
  float current = ina219.getCurrent_mA();           // en mA
  float shuntVoltage = ina219.getShuntVoltage_mV(); // en mV
  // Imprimir valores medidos con 4 decimales (puedes ajustar el número según prefieras)
  Serial.print("Shunt Voltage: ");
  Serial.print(shuntVoltage, 4);
  Serial.println(" mV");
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
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(0, 10, "Prueba tmpo respuesta");
  TestResult result;
  Serial.println("Enviando pulso...");

  unsigned long tiempoInicio = micros();
  digitalWrite(INYECTOR_PIN, HIGH);

  // Espera hasta que la corriente supere el umbral
  while (ina219.getCurrent_mA() < currentThreshold)
  {
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
  const unsigned long tiempoMin = 1000; // 1 ms
  const unsigned long tiempoMax = 2000; // 2 ms
  const char *resultado;

  if (tiempoActivacion >= tiempoMin && tiempoActivacion <= tiempoMax)
  {
    resultado = "Inyector BUENO";
    result.passed = true;
  }
  else
  {
    resultado = "Inyector MALO";
    result.passed = false;
  }

  u8g2.drawStr(0, 48, resultado);

  resultadosTests["tiemporespuesta"] = result;
  u8g2.sendBuffer();
  delay(5000);
}

// 6. Prueba de Caudal de Combustibles
void testCaudal() {
  TestResult result;
  
  ultrasonic_sensor.printDistance();

  // 1) Medición con ultrasonidos
  float distIni_cm = ultrasonic_sensor.get_distance_fashion(10, true);
  delay(1000);
 
  // 2) Ejecutar prueba (usa testDurationSec internamente en ms si es necesario)
  controlBombaDurantePrueba(
    3, 30,
    testTimeValolue,
    setupvalues,
    estate_inyector::ACTIVA_INYECTOR_CON_REVOLUCION
  );
  inyector.stop();
 
    delay(1000);
  // 3) Medir distancia final
  float distFin_cm = ultrasonic_sensor.get_distance_fashion(10, false);

  // 4) Volumen real en cc
  float deltaH_cm     = distIni_cm - distFin_cm;
  float volumenMedido = deltaH_cm * areaProbeta_cm2;
  Serial.print(distIni_cm);
  Serial.print(distFin_cm);

  Serial.print(deltaH_cm);
  Serial.print(areaProbeta_cm2);
  Serial.print(volumenMedido);
  // 5) Leer parámetros globales
  float freq_Hz       = frecuencia;        // Hz
  float pulseWidth_us = anchoPulsoUs;     // µs
  float testTime_s    = tiempoPruebaSec;  // s
  // frecuenciaa = frecuencia ;     // RPM → Hz (4 tiempos)
  // anchoPulsoUss =anchoPulsoUs;  // ms → µs
  // tiempoPruebaSecc = tiempoPruebaSec; // ms → segundos
  // 6) Cálculo teórico
  float combustibleTeorico = calcularCombustible(
    Q_inyector_cc_min,
    freq_Hz,
    pulseWidth_us,
    testTime_s
  );
  ultrasonic_sensor.printDistance();
  // 7) Factor de corrección
  float calFactor = volumenMedido / combustibleTeorico;
  float combustibleAjustado = combustibleTeorico * calFactor;

  // 8) Evaluación ±5%
  result.measuredValue = volumenMedido;
  result.expectedValue = combustibleAjustado;
  result.passed        = (
    volumenMedido >= combustibleAjustado * 0.95 &&
    volumenMedido <= combustibleAjustado * 1.05
  );
  resultadosTests["caudal"] = result;

  // 9) Salida Serial para depurar
  Serial.println("--- Test Caudal ---");
  Serial.print("Vol medido cc:     "); Serial.println(volumenMedido,      4);
  Serial.print("Vol teorico cc:    "); Serial.println(combustibleTeorico, 4);
  Serial.print("Factor correcion:  "); Serial.println(calFactor,           4);
  Serial.print("Vol ajustado cc:   "); Serial.println(combustibleAjustado, 4);
  Serial.print("Resultado:         "); Serial.println(result.passed ? "PASA" : "FALLA");
  Serial.println("---------------------");
}


// 7. prueba de Temperatura
void testTemperatura()
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

void runTestResistencia()
{
  playTestStartSound();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawStr(0, 10, "Prueba de resistencia");
  delay(10);
  u8g2.sendBuffer();
  controlBombaDurantePrueba(3, 40, 1500, testResistencia, estate_inyector::ACTIVA_INYECTOR_SIN_REVOLUCION);
  // Convertir valores a string antes de pasarlos a drawStr()
  std::string measuredValueStr = std::to_string(resultadosTests["resistencia"].measuredValue);
  std::string passedStr = resultadosTests["resistencia"].passed ? "OK" : "FAILED";
  Serial.print("imprimir measuredvalue antes de conversion:   ");
  Serial.print(resultadosTests["Resistencia"].measuredValue);
  Serial.println(" ohm");
  // Dibujar títulos y etiquetas
  u8g2.drawStr(0, 64, "Pres boton salir");
  const char *label = "Resistencia:";
  const char *estadoLabel = "Estado:";
  int labelWidth = u8g2.getStrWidth(label);
  int estadoWidth = u8g2.getStrWidth(estadoLabel);
  int espacio = 5;
  int variableX = labelWidth + espacio;
  int valorX = estadoWidth + espacio;
  // Dibujar las etiquetas y sus valores
  u8g2.drawStr(0, 25, label);
  u8g2.drawStr(variableX, 25, measuredValueStr.c_str());
  u8g2.drawStr(0, 45, estadoLabel);
  u8g2.drawStr(valorX, 45, passedStr.c_str());
  u8g2.sendBuffer();
  delay(5000);
  playTestEndSound();
  Serial.println("desalojamos liquido");
  controlBombaDurantePrueba(0, 0, 3000, []()
                            { Callback(); }, estate_inyector::ACTIVA_INYECTOR_SIN_REVOLUCION);
  playTestEndSound();
}

void runTestFugas()
{
  playTestStartSound();

  Serial.println(ultrasonic_sensor.getDistance());
  ultrasonic_sensor.get_distance_fashion(20, true);
  Serial.println("activamos la bomba");
  controlBombaDurantePrueba(3, 40, 4000, []()
                            { Callback(); }, estate_inyector::INJECTOR_MIN);
  u8g2.clearBuffer();
  testFugas();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  // Convertir valores a string antes de pasarlos a drawStr()
  std::string measuredValueStr = std::to_string(resultadosTests["fugas"].measuredValue);
  std::string passedStr = resultadosTests["fugas"].passed ? "PASSED" : "FAILED";
  // Dibujar títulos y etiquetas
  u8g2.drawStr(0, 10, "Prueba de fugas");
  u8g2.drawStr(0, 64, "Pres boton salir");
  const char *label = "fugas:";
  const char *estadoLabel = "Estado:";
  int labelWidth = u8g2.getStrWidth(label);
  int estadoWidth = u8g2.getStrWidth(estadoLabel);
  int espacio = 5;
  int variableX = labelWidth + espacio;
  int valorX = estadoWidth + espacio;
  // Dibujar las etiquetas y sus valores
  u8g2.drawStr(0, 25, label);
  u8g2.drawStr(variableX, 25, measuredValueStr.c_str());
  u8g2.drawStr(0, 45, estadoLabel);
  u8g2.drawStr(valorX, 45, passedStr.c_str());
  u8g2.sendBuffer();
  delay(5000);
  Serial.println("desalojamos liquido");
  controlBombaDurantePrueba(0, 0, 4000, []()
                            { Callback(); }, estate_inyector::ACTIVA_INYECTOR_SIN_REVOLUCION);
  playTestEndSound();
}

void runTestSonido()
{
  playTestStartSound();
  // controlBombaDurantePrueba(3, 30, testTimeValue, setupvalues, estate_inyector::ACTIVA_INYECTOR_CON_REVOLUCION);
  // inyector.stop();
  digitalWrite(BOMBA_PIN, LOW);
  delay(1000);
  digitalWrite(BOMBA_PIN, HIGH);
  controlBombaDurantePrueba(0, 0, 7000, testSonido, estate_inyector::ACTIVA_INYECTOR_CON_REVOLUCION);

  controlBombaDurantePrueba(0, 0, 3000, []()
                            { Callback(); }, estate_inyector::ACTIVA_INYECTOR_SIN_REVOLUCION);
  playTestEndSound();
}

void runTestCorriente()
{
  playTestStartSound();
  controlBombaDurantePrueba(3, 10, 5000, testCorriente, estate_inyector::ACTIVA_INYECTOR_SIN_REVOLUCION);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  // Convertir valores a string antes de pasarlos a drawStr()
  std::string measuredValueStr = std::to_string(resultadosTests["corrienteActivacion"].measuredValue);
  std::string passedStr = resultadosTests["corrienteActivacion"].passed ? "PASSED" : "FAILED";
  Serial.print("imprimir measuredvalue antes de conversion:   ");
  Serial.print(resultadosTests["corrienteActivacion"].measuredValue);
  Serial.println(" mA");
  // Dibujar títulos y etiquetas
  u8g2.drawStr(0, 10, "Prueba de corriente");
  u8g2.drawStr(0, 64, "Pres boton salir");
  const char *label = "Corriente:";
  const char *estadoLabel = "Estado:";
  int labelWidth = u8g2.getStrWidth(label);
  int estadoWidth = u8g2.getStrWidth(estadoLabel);
  int espacio = 5;
  int variableX = labelWidth + espacio;
  int valorX = estadoWidth + espacio;
  // Dibujar las etiquetas y sus valores
  u8g2.drawStr(0, 25, label);
  u8g2.drawStr(variableX, 25, measuredValueStr.c_str());
  u8g2.drawStr(0, 45, estadoLabel);
  u8g2.drawStr(valorX, 45, passedStr.c_str());
  u8g2.sendBuffer();
  delay(1000);
  playTestEndSound();
  Serial.println("desalojamos liquido");
  controlBombaDurantePrueba(0, 0, 3000, []()
                            { Callback(); }, estate_inyector::ACTIVA_INYECTOR_SIN_REVOLUCION);
  playTestEndSound();
}

void runTestTiempoRespuesta()
{
  playTestStartSound();

  testTiempoRespuesta();

  playTestEndSound();
}

void runTestCaudal()
{
  playTestStartSound();

  testCaudal(); // return testCaudal(selected.caudal, 0.05, 60000); // tolerancia del 5%, prueba de 1 minuto
  Serial.println("///////////////los resultados de la prueba////////////");
  // Serial.println(vol);
  Serial.println(resultadosTests["caudal"].measuredValue);
  Serial.println("////////////////////////////////////////////");
  u8g2.setFont(u8g2_font_6x12_tf);
  // Convertir valores a string antes de pasarlos a drawStr()
  std::string measuredValueStr = std::to_string(resultadosTests["caudal"].measuredValue);
  std::string passedStr = resultadosTests["caudal"].passed ? "PASSED" : "FAILED";
  // Dibujar títulos y etiquetas
  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "Prueba de flujo");
  u8g2.drawStr(0, 64, "Pres boton salir");
  const char *label = "flujo:";
  const char *estadoLabel = "Estado:";
  int labelWidth = u8g2.getStrWidth(label);
  int estadoWidth = u8g2.getStrWidth(estadoLabel);
  int espacio = 5;
  int variableX = labelWidth + espacio;
  int valorX = estadoWidth + espacio;
  // Dibujar las etiquetas y sus valores
  u8g2.drawStr(0, 25, label);
  u8g2.drawStr(variableX, 25, measuredValueStr.c_str());
  u8g2.drawStr(0, 45, estadoLabel);
  u8g2.drawStr(valorX, 45, passedStr.c_str());
  u8g2.sendBuffer();
  delay(1000);
  controlBombaDurantePrueba(0, 0, 3000, []()
                            { Callback(); }, estate_inyector::ACTIVA_INYECTOR_SIN_REVOLUCION);
  playTestEndSound();
}

void runTestTemperatura()
{
  playTestStartSound();

  inyector.rpmValue_tem = 6500;
  controlBombaDurantePrueba(3, 40, 12000, testTemperatura, estate_inyector::ACTIVA_INYECTOR_CON_REVOLUCION);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);
  // Convertir valores a string antes de pasarlos a drawStr()
  std::string measuredValueStr = std::to_string(resultadosTests["temperatura"].measuredValue);
  std::string passedStr = resultadosTests["temperatura"].passed ? "PASSED" : "FAILED";

  // Serial.print("imprimir measuredvalue despues de conversion:   "); Serial.print(measuredValueStr); Serial.println(" V");
  // Dibujar títulos y etiquetas
  u8g2.drawStr(0, 10, "Prueba de temperatura");
  u8g2.drawStr(0, 64, "Pres boton salir");
  const char *label = "Temperatura:";
  const char *estadoLabel = "Estado:";
  int labelWidth = u8g2.getStrWidth(label);
  int estadoWidth = u8g2.getStrWidth(estadoLabel);
  int espacio = 5;
  int variableX = labelWidth + espacio;
  int valorX = estadoWidth + espacio;
  // Dibujar las etiquetas y sus valores
  u8g2.drawStr(0, 25, label);
  u8g2.drawStr(variableX, 25, measuredValueStr.c_str());
  u8g2.drawStr(0, 45, estadoLabel);
  u8g2.drawStr(valorX, 45, passedStr.c_str());
  u8g2.sendBuffer();
  delay(1000);
  controlBombaDurantePrueba(0, 0, 3000, []()
                            { Callback(); }, estate_inyector::ACTIVA_INYECTOR_SIN_REVOLUCION);
  playTestEndSound();
}

void activarBuzzer()
{
  // Código para activar el buzzer
  // tone(8, 1000, 500); // Ejemplo: Sonido en pin 8, 1000 Hz por 500ms
}
// Modo Automático: se ejecutan todas las pruebas en secuencia.
void mostrarResultado(const char *nombre, const TestResult &result)
{
  char output[64];
  if (result.expectedValue.has_value())
  {
    snprintf(output, sizeof(output), "%s: %.2f/%.2f %s",
             nombre,
             result.measuredValue,
             *result.expectedValue,
             result.passed ? "OK" : "FALLA");
  }
  else
  {
    snprintf(output, sizeof(output), "%s: %.2f %s",
             nombre,
             result.measuredValue,
             result.passed ? "OK" : "FALLA");
  }
  u8g2.clearBuffer();
  u8g2.drawStr(0, 12, output);
  u8g2.sendBuffer();
  delay(1000);
  if (!result.passed)
    activarBuzzer();
}

// void updateBuzzer()
// {
//   if (buzzerActive && (millis() - buzzerStartTime >= 1000))
//   {
//     digitalWrite(BUZZER_PIN, LOW); // Apaga el buzzer después de 1 segundo
//     buzzerActive = false;          // Marca como inactivo
//   }
// }

void setupbit()
{
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Asegura estado inicial bajo
}

// Identificadores de pruebas
enum EPrueba
{
  RESISTENCIA,
  FUGAS,
  SONIDO,
  CORRIENTE,
  TIEMPO_RESP,
  CAUDAL,
  TEMPERATURA
};

// Etiquetas y soluciones para cada prueba
static const char *pruebaLabels[] = {
    "Resistencia", "Fugas", "Sonido", "Corriente", "Tiempo Resp.", "Caudal", "Temperatura"};
static const char *pruebaSoluciones[] = {
    "Revise conexiones;si continua igual,cambie inyector",
    "Tina ultrasonica;si continua igual,cambie inyector",
    "Tina ultrasonica;si continua igual,cambie inyector",
    "Revise conexiones;si continua igual,cambie inyector",
    "Revise conexiones;si continua igual,cambie inyector",
    "Tina ultrasonica;si continua igual,cambie inyector",
    "Espere un momento;si continua igual,cambie inyector"};

// Mapa global de resultados
std::map<EPrueba, TestResult> resultados;

// ————— Mostrar correcciones una a una —————
void mostrarSoluciones()
{
  // Prueba de Resistencia
  {
    auto it = resultadosTests.find("resistencia");
    if (it != resultadosTests.end() && !it->second.passed)
    {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x12_tf);
      u8g2.drawStr(0, 5, "Prueba de Resistencia");
      u8g2.setFont(u8g2_font_5x8_tf);
      u8g2.drawStr(0, 20, "Revise conexiones;");
      u8g2.drawStr(0, 30, "si continua igual,");
      u8g2.drawStr(0, 40, "cambie inyector");
      u8g2.drawStr(0, 56, "Presione boton");
      u8g2.sendBuffer();

      while (digitalRead(botonPin) == HIGH)
        delay(50);
      delay(debounceDelay);
    }
    playClickSound();
  }

  // Prueba de Fugas
  {
    auto it = resultadosTests.find("fugas");
    if (it != resultadosTests.end() && !it->second.passed)
    {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x12_tf);
      u8g2.drawStr(0, 5, "Prueba de Fugas");
      u8g2.setFont(u8g2_font_5x8_tf);
      u8g2.drawStr(0, 20, "Paselo por tina");
      u8g2.drawStr(0, 30, "ultrasonica;");
      u8g2.drawStr(0, 40, "cambie inyector");
      u8g2.drawStr(0, 56, "Presione boton");
      u8g2.sendBuffer();
      while (digitalRead(botonPin) == HIGH)
        delay(50);
      delay(debounceDelay);
    }
    playClickSound();
  }

  // Prueba de Sonido
  {
    auto it = resultadosTests.find("Sonido");
    if (it != resultadosTests.end() && !it->second.passed)
    {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x12_tf);
      u8g2.drawStr(0, 5, "Prueba de Sonido");
      u8g2.setFont(u8g2_font_5x8_tf);
      u8g2.drawStr(0, 20, "Paselo por tina");
      u8g2.drawStr(0, 30, "ultrasonica;");
      u8g2.drawStr(0, 40, "cambie inyector");
      u8g2.drawStr(0, 56, "Presione boton");
      u8g2.sendBuffer();
      while (digitalRead(botonPin) == HIGH)
        delay(50);
      delay(debounceDelay);
    }
    playClickSound();
  }

  // Prueba de Corriente de Activación
  {
    auto it = resultadosTests.find("corrienteActivacion");
    if (it != resultadosTests.end() && !it->second.passed)
    {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x12_tf);
      u8g2.drawStr(0, 5, "Prueba Corriente");
      u8g2.setFont(u8g2_font_5x8_tf);
      u8g2.drawStr(0, 20, "Revise conexiones;");
      u8g2.drawStr(0, 30, "cambie inyector");
      u8g2.drawStr(0, 56, "Presione boton");
      u8g2.sendBuffer();
      while (digitalRead(botonPin) == HIGH)
        delay(50);
      delay(debounceDelay);
    }
    playClickSound();
  }

  // Prueba de Tiempo de Respuesta
  {
    auto it = resultadosTests.find("tiempoRespuesta");
    if (it != resultadosTests.end() && !it->second.passed)
    {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x12_tf);
      u8g2.drawStr(0, 5, "Prueba Tiempo Resp.");
      u8g2.setFont(u8g2_font_5x8_tf);
      u8g2.drawStr(0, 20, "Revise conexiones;");
      u8g2.drawStr(0, 30, "cambie inyector");
      u8g2.drawStr(0, 56, "Presione boton");
      u8g2.sendBuffer();
      while (digitalRead(botonPin) == HIGH)
        delay(50);
      delay(debounceDelay);
    }
    playClickSound();
  }

  // Prueba de Caudal
  {
    auto it = resultadosTests.find("caudal");
    if (it != resultadosTests.end() && !it->second.passed)
    {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x12_tf);
      u8g2.drawStr(0, 5, "Prueba de Caudal");
      u8g2.setFont(u8g2_font_5x8_tf);
      u8g2.drawStr(0, 20, "tina ultrasonica");
      u8g2.drawStr(0, 30, "si sigue igual;");
      u8g2.drawStr(0, 40, "cambie inyector");
      u8g2.drawStr(0, 56, "Presione boton");
      u8g2.sendBuffer();
      while (digitalRead(botonPin) == HIGH)
        delay(50);
      delay(debounceDelay);
    }
    playClickSound();
  }

  // Prueba de Temperatura
  {
    auto it = resultadosTests.find("temperatura");
    if (it != resultadosTests.end() && !it->second.passed)
    {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x12_tf);
      u8g2.drawStr(0, 5, "Prueba Temperatura");
      u8g2.setFont(u8g2_font_5x8_tf);
      u8g2.drawStr(0, 20, "Revise conexiones y");
      u8g2.drawStr(0, 30, "espere un momento;");
      u8g2.drawStr(0, 40, "cambie inyector");
      u8g2.drawStr(0, 56, "Presione boton");
      u8g2.sendBuffer();
      while (digitalRead(botonPin) == HIGH)
        delay(50);
      delay(debounceDelay);
    }
    playClickSound();
  }

  // Al finalizar todas las correcciones…
  // devolver();
  u8g2.clearBuffer();
  delay(10);

  estadoActual = ATRASITO;
}
// ————— Mostrar resumen de resultados —————
void mostrarResultadosFinales()
{
  playResultsSound();
  u8g2.setFont(u8g2_font_5x8_tf);
  u8g2.drawStr(0, 0, "RESULTADOS");

  {
    int y = 10; // posición vertical inicial
    char output[64];
    for (auto &p : pruebaInfo)
    {
      auto it = resultadosTests.find(p.key);
      if (it != resultadosTests.end())
      {
        const TestResult &r = it->second;
        const char *estado = r.passed ? "OK" : "FALLA";
        // Mostrar solo estado de la prueba
        snprintf(output, sizeof(output), "%s: %s", p.label, estado);
      }
      else
      {
        snprintf(output, sizeof(output), "%s: --", p.label);
      }
      u8g2.drawStr(0, y, output);
      y += 8; // desplazamiento para siguiente línea
    }
  }

  u8g2.sendBuffer();
  delay(300);
  while (digitalRead(botonPin) == HIGH)
    delay(100);
  delay(debounceDelay);
  delay(200);
  mostrarSoluciones();
}
// Ejecuta todas las pruebas en modo automático con opción a parar
