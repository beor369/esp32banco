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
#include "ultrasonicsensor/ultrasonic_sensor.h"
// #include "max6675.h"
#include <max6675.h>
#include <Adafruit_INA219.h>
#include "pruebas/pruebas.h"
// Define los pines que se utilizarán (ajusta según tu conexión)
// #define PIN_CLK 45    // Pin de reloj SPI
// #define PIN_CS  3   // Pin de selección (Chip Select)
// #define PIN_DO  8    // Pin de datos (MISO)




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


void setup()
{
  
  Serial.begin(115200);
  //Serial2.begin(115200, SERIAL_8N1, UART_CAM_RX, UART_CAM_TX);
  EEPROM.begin(EEPROM_SIZE);
  pinMode(pinraro, INPUT_PULLUP);
  digitalWrite(pinraro,LOW);
  initDisplay();
  setupEncoder();
  mostrarMenu(); // Muestra el menú inicial
  cargarDatos();
  setupbit(); 
  setupmic(); 
  setupselectrpm();
  inyector.begin();
  ultrasonic_sensor.begin();
  // sensor.begin();

  // ina219Wire.begin(8, 9);

  // // Inicializa el sensor INA219 utilizando el bus I2C alternativo
  // if (!ina219.begin(&ina219Wire)) {
  //   Serial.println("No se encontró el chip INA219");
  //   while (1) { delay(10); }
  // }
  // Serial.println("INA219 detectado correctamente");
 }

void loop()
{
  // sensor.update();
  inyector.update(); // Actualiza el estado (no bloqueante)
  updateBuzzer();
  displayEncoderPosition();
  // Serial.print("C = "); 
  // Serial.println(thermocouple.readCelsius());
  // Serial.print("F = ");
  // Serial.println(thermocouple.readFahrenheit());

  // Lecturas del sensor INA219

  
// Espera 2 segundos entre lecturas

}
