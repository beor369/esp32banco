#include <Arduino.h>
#include "Config.h"
#include "displayhandler/DisplayHandler.h"
#include "encoderhadler/EncoderHandler.h"
#include "menu/Menu.h"
#include "tecladovirtual/TecladoVirtual.h"
#include "saveInjector/saveInjector.h"
#include "pruebas/activatebankofproof/activatebankofproof.h"
#include "selectrpm/selectrpm.h"
#include "InjectorController/InjectorController.h"
#include <Wire.h>
#include "SRF05.h"
#include "ultrasonicsensor/ultrasonic_sensor.h"
// #include "max6675.h"
#include <max6675.h>
#include <Adafruit_INA219.h>
#include "pruebas/pruebas.h"
#include "Notes.h"
// Define los pines que se utilizarán (ajusta según tu conexión)
// #define PIN_CLK 45    // Pin de reloj SPI
// #define PIN_CS  3   // Pin de selección (Chip Select)
// #define PIN_DO  8    // Pin de datos (MISO)
// Instancia SRF05 (pinTrigger, pinEcho)
// Instancia SRF05 (pinTrigger, pinEcho)
// #define ECHO_PIN 4  
// #define TRIG_PIN 2  


// Crea la instancia del objeto MAX6675
// MAX6675 termocupla(PIN_CLK, PIN_CS, PIN_DO);
//MAX6675 termocupla(PIN_CLK, PIN_CS, PIN_DO);
// Crea una instancia del sensor en el pin 4 (ajusta según tu conexión)
// FlowSensor sensor(4);
// Instanciar objetos globalmente
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 20, 21);
ESP32Encoder encoder;
///////////////////

TwoWire ina219Wire(1);

// Crea la instancia del sensor usando la instancia personalizada de TwoWire
Adafruit_INA219 ina219(0x40);





// Variables globales de estado (puedes inicializarlas aquí)
Estado estadoActual = MENU_PRINCIPAL;
int indiceMenu = 0;
int indiceInyector = 0;
int indiceSubMenu = 0;
int indiceSubSubMenu = 0;
int indicefuncionamiento = 0;
int indiceseleccionarmoto = 0;
int indiceMotoSeleccionada = 0;
int indiceagregarborrarinjt = 0;
unsigned long lastEncoderMoveTime = 0;
// definición de los pines
// const int trigPin = 2;
// const int echoPin = 4;

// definición del objeto global
// SRF05 SRF(trigPin, echoPin);
// función genérica que atacha, suena, y libera el buzzer
void setup()
{
 // configura canal 1: frecuencia inicial neutra (1 kHz), resolución 8 bits
 ledcSetup(BUZZER_CHAN, 1000, PWM_RESOLUTION);
//  SRF.setCorrectionFactor(1.035);
  Serial.begin(115200);
  //Serial2.begin(115200, SERIAL_8N1, UART_CAM_RX, UART_CAM_TX);
  EEPROM.begin(EEPROM_SIZE);
  initDisplay();
  u8g2.enableUTF8Print();
  setupEncoder();
  mostrarMenu(); // Muestra el menú inicial
  cargarDatos();
  // setupbit(); 
  setupmic(); 
  setupselectrpm();
  inyector.begin();
  // ultrasonic_sensor.begin();
  // sensor.begin();

  ina219Wire.begin(8, 9);
  ina219.setCalibration_16V_400mA();

  // // Calibración personalizada
  // uint32_t currentDivider_mA = 10; // Ajusta según tu resistencia shunt y rango de corriente
  // ina219.setCalibration_32V_2A(); // Usa una función de calibración predefinida o personalizada
  // Inicializa el sensor INA219 utilizando el bus I2C alternativo
  if (!ina219.begin(&ina219Wire)) {
    Serial.println("No se encontró el chip INA219");
    while (1) { delay(10); }
  }
  Serial.println("INA219 detectado correctamente");
 }

// ——— Variables globales para timing ———
// unsigned long lastEncoderMoveTime = 0;
// unsigned long lastButtonPress     = 0;
unsigned long lastScreenDraw      = 0;

// const unsigned long encoderMoveDelay = 100; // ms para debounce de giro
const unsigned long buttonDebounce   = 50;  // ms para rebote de botón
const unsigned long idlePeriod       = 500; // ms sin mover → refresco
const unsigned long drawPeriod       = 200; // ms mínimos entre redraws

// Dentro de file-scope (antes de setup)
static int          btnRawState       = HIGH;     // última lectura cruda
static int          btnStableState    = HIGH;     // estado filtrado
static unsigned long btnLastChangeTime = 0;       // cuando cambió btnRawState
static bool         btnClickEvent     = false;    // marca un clic nuevo

// Debounce en ms
//const unsigned long buttonDebounce = 50;  // ajusta entre 20–100 según tu botón
// ——— 1) Solo encoder: actualiza índice o estado pero NO dibuja ———
// Esto corre siempre en loop(), antes o después de processEncoderInput
void updateButton() {
  int reading = digitalRead(botonPin);
  unsigned long now = millis();

  // 1) Si cambió la lectura bruta, reinicia el timer
  if (reading != btnRawState) {
    btnRawState       = reading;
    btnLastChangeTime = now;
  }

  // 2) Si ha estado estable más tiempo que el debounce, aceptamos el cambio
  if (now - btnLastChangeTime > buttonDebounce) {
    if (btnStableState != btnRawState) {
      btnStableState = btnRawState;
      // 3) Si acabamos de caer a LOW, es un nuevo clic
      if (btnStableState == LOW) {
        btnClickEvent = true;
      }
    }
  }
}

// inicias un clic leyendo btnClickEvent y luego reseteándolo
bool consumeClick() {
  if (btnClickEvent) {
    btnClickEvent = false;
    return true;
  }
  return false;
}

// processEncoderInput revisitado
void processEncoderInput(
  void (*callbackIndice)(bool),
  void (*callbackManejarEstado)()
) {
  // a) primero actualiza el botón
  updateButton();

  // b) procesar giro idéntico a antes
  long cnt = encoder.getCount();
  if (millis() - lastEncoderMoveTime > encoderMoveDelay && cnt != 0) {
    bool incremento = cnt > 0;
    encoder.clearCount();
    lastEncoderMoveTime = millis();
    callbackIndice(incremento);
    playMoveSound();            // sonido al girar
  }

  // c) ahora procesa el clic filtrado
  if (consumeClick()) {
    playClickSound();           // sonido al clicar
    callbackManejarEstado();
  }
}

// refreshScreenIfNeeded idéntica a antes
void refreshScreenIfNeeded(void (*callbackMostrarMenu)()) {
  static unsigned long lastScreenDraw = 0;
  unsigned long now = millis();

  if (automatico) {
    if (now - lastScreenDraw > drawPeriod) {
      callbackMostrarMenu();
      manejarEstado();
      lastScreenDraw = now;
    }
    return;
  }

  // si giraste recientemente, dibuja inmediatamente
  if (now - lastEncoderMoveTime < idlePeriod && lastScreenDraw < lastEncoderMoveTime) {
    callbackMostrarMenu();
    lastScreenDraw = now;
    return;
  }

  // si llevas idle, refresca cada drawPeriod
  if (now - lastEncoderMoveTime >= idlePeriod && now - lastScreenDraw > drawPeriod) {
    callbackMostrarMenu();
    lastScreenDraw = now;
  }
}



// ——— 3) loop() simplificado ———
void loop() {
  // primero procesa INPUT
  processEncoderInput(actualizarIndice, manejarEstado);
  refreshScreenIfNeeded(mostrarMenu);

  // Sensor.update();
  inyector.update(); // Actualiza el estado (no bloqueante)
  // updateBuzzer();
  // luego refresca pantalla si toca
  // delay(1000);
  // buzzerr.noTone();
  

}
