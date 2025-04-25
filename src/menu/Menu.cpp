#include "Menu.h"
#include "displayhandler/DisplayHandler.h"
#include "saveInjector/saveInjector.h"
#include "pruebas/pruebas.h"
#include "pruebas/activatebankofproof/activatebankofproof.h"
#include "selectrpm/selectrpm.h"
#include "ultrasonicsensor/ultrasonic_sensor.h"
#include "help/help.h"

#include "InjectorController/InjectorController.h"
// Declaración de variables globales (definidas en main.cpp)
extern int indiceMenu;
extern int indiceInyector;
extern int indiceSubSubMenu;
extern int indicefuncionamiento;
extern int indiceseleccionarmoto;
extern int indiceagregarborrarinjt;
extern Estado estadoActual;
bool automatico;
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

// Ejemplos de arreglos de imágenes para cada menú
static const char *menuPrincipalImagenes[] = {"iniciar_prueba", "agregarborrarinyector", "ayuda"};
static const int tamaniomenuprincipal = sizeof(menuPrincipalImagenes) / sizeof(menuPrincipalImagenes[0]);

static const char *submenuFuncionamientoImagenes[] = {"modo_manual", "modo_automatico", "atras"};
static const int tamaniosubMenuFuncionamiento = sizeof(submenuFuncionamientoImagenes) / sizeof(submenuFuncionamientoImagenes[0]);

static const char *submenuManualImagenes[] = {
    "prueba_medir_resistencia", "prueba_fugas", "prueba_medir_clic", "prueba_corriente_activacion",
    "prueba_tiempo_respuesta", "prueba_flujo", "prueba_temperatura", "resultados", "atrasitoo"};
static const int tamanioSubMenuManual = sizeof(submenuManualImagenes) / sizeof(submenuManualImagenes[0]);

static const char *agregarBorrarImagenes[] = {"AGREGAR_INJ", "BORRAR_INJ", "ATRASAGREGARINYECTOR_INJ"};
static const int tamanioagregarborrarinj = sizeof(agregarBorrarImagenes) / sizeof(agregarBorrarImagenes[0]);

static const char *seleccionarMotoImagenes[] = {"SIGUIENTEFUNCIONAMIENTO", "VER_CARACT", "ATRASMENU"};
static const int tamanioseleccionarmoto = sizeof(seleccionarMotoImagenes) / sizeof(seleccionarMotoImagenes[0]);

void controlBombaDurantePrueba(unsigned long tiempoBombaEncendidaS, unsigned long tiempoBombaApagadaS, long time_injector, Callback actualizaValores, EstadoInyector estadoInyector)
{
  unsigned long tiempoBombaEncendidaMs = tiempoBombaEncendidaS * 1000;
  unsigned long tiempoBombaApagadaMs = tiempoBombaApagadaS * 1000;
  unsigned long inicioCiclo = millis();
  bool bombaEncendida = true;

  switch (estadoInyector)
  {
  case estate_inyector::ACTIVA_INYECTOR_SIN_REVOLUCION:
    inyector.is_activate_max = true;
    inyector.is_activate_min = false;
    inyector.activarInyectorDesdeEncoder(time_injector);
    break;
  case estate_inyector::ACTIVA_INYECTOR_CON_REVOLUCION:
    inyector.activarInyectorDesdeEncoder(time_injector);
    break;
  case estate_inyector::INJECTOR_MIN:
    inyector.is_activate_min = true;
    inyector.is_activate_max = false;
    inyector.activarInyectorDesdeEncoder(time_injector);
    break;
  case estate_inyector::INYECTOR_DESACTIVADO:
  default:
    break;
  }

  // Si ambos tiempos son 0, la bomba no se activa
  if (tiempoBombaEncendidaMs == 0 && tiempoBombaApagadaMs == 0)
  {
    while (!(inyector.isActive && (millis() - inyector.startTime >= inyector.testDurationMs)))
    {
      actualizaValores(); // Sigue actualizando valores mientras se ejecuta la prueba
    }
    return; // Sale de la función sin activar la bomba
  }

  digitalWrite(BOMBA_PIN, LOW); // Enciende la bomba

  while (!(inyector.isActive && (millis() - inyector.startTime >= inyector.testDurationMs)))
  {
    unsigned long tiempoCiclo = millis() - inicioCiclo;

    if (bombaEncendida && (tiempoCiclo >= tiempoBombaEncendidaMs))
    {
      digitalWrite(BOMBA_PIN, HIGH);
      bombaEncendida = false;
      inicioCiclo = millis();
    }
    else if (!bombaEncendida && (tiempoCiclo >= tiempoBombaApagadaMs))
    {
      digitalWrite(BOMBA_PIN, LOW);
      bombaEncendida = true;
      inicioCiclo = millis();
    }

    actualizaValores();
  }

  digitalWrite(BOMBA_PIN, HIGH); // Apaga la bomba al final del ciclo
}
void devolver()
{
  encoder.clearCount();
  Serial.println("Esperando pulsación de botón...");
  // Espera a que se presione el botón
  while (digitalRead(botonPin) == LOW)
  {
    inyector.stop();
    delay(10);
  }
  Serial.println("Botón presionado, esperando liberación...");
  // Espera a que se suelte el botón
  while (digitalRead(botonPin) == HIGH)
  {
    inyector.stop();
    delay(100);
  }
  delay(400); // Delay para evitar rebotes
  u8g2.clearBuffer();
  // inyector.stop();
  dibujarImagen(submenuManualImagenes[indiceSubSubMenu]);
  estadoActual = SUBMENU_MANUAL;
  Serial.println("Cambio de estado a MENU_PRINCIPAL");
}
void setupvalues()
{
  InyectorParametros values = inyector.devolver();

  unsigned long tolerancePercent = 10;
  TestResult results;


}

// Muestra el menú según el estado actual
void mostrarMenu()
{
  switch (estadoActual)
  {
  case MENU_PRINCIPAL:
    if (indiceMenu >= 0 && indiceMenu < tamaniomenuprincipal)
    {
      dibujarImagen(menuPrincipalImagenes[indiceMenu]);
    }
    break;
  case MENU_SELECCION_INYECTOR:
    // beep();
    
    handleTestInjector();
    break;
  case SELECCIONAR_RPM_TIME:
    loopselectrpm();
    break;
  case SUBMENU_FUNCIONAMIENTO:
    if (indicefuncionamiento >= 0 && indicefuncionamiento < tamaniosubMenuFuncionamiento)
    {
      dibujarImagen(submenuFuncionamientoImagenes[indicefuncionamiento]);
    }
    break;
  case SUBMENU_MANUAL:
    if (indiceSubSubMenu >= 0 && indiceSubSubMenu < tamanioSubMenuManual)
    {
      dibujarImagen(submenuManualImagenes[indiceSubSubMenu]);
    }
    break;
  case AGREGAR_BORRAR_INJ:
    if (indiceagregarborrarinjt >= 0 && indiceagregarborrarinjt < tamanioagregarborrarinj)
    {
      dibujarImagen(agregarBorrarImagenes[indiceagregarborrarinjt]);
    }
    break;
  case SELECCIONAR_MOTO:
    if (indiceseleccionarmoto >= 0 && indiceseleccionarmoto < tamanioseleccionarmoto)
    {
      dibujarImagen(seleccionarMotoImagenes[indiceseleccionarmoto]);
    }
    break;

  case MENU_CARACTERISTICAS:

    handleShowInjector();

    break;
  case AGREGAR:
    Serial.println("hola voy agregar");
    handleAddInjector();
    break;
  case BORRAR:
    Serial.println("hola voy borrar");
    handleDeleteData();
    break;
  case SUBMENU_MID_RES:

  {
    // beep();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 10, "Prueba de resistencia");
    u8g2.sendBuffer();
    runTestResistencia();

    if (automatico) {
      estadoActual = SUBMENU_FUGAS;   
    } else {
      devolver(); 
    }                    
    break;
       }
  
  case SUBMENU_FUGAS:
  { // Test de Fugas
    // beep();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 10, "Prueba de fugas");
    u8g2.sendBuffer();
    runTestFugas();
    Serial.println(automatico);
    if (automatico) {
      estadoActual = SUBMENU_CLICK;   // avanza al siguiente
    } else {
      devolver();                     // modo manual: vuelves al menú
    }
    break;
  }
  case SUBMENU_CLICK:

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 10, "Prueba de Sonido");
    u8g2.sendBuffer();
    runTestSonido();
    if (automatico) {
      estadoActual = SUBMENU_CORRIENTE_ACTIVACION;   // avanza al siguiente
    } else {
      devolver();                     // modo manual: vuelves al menú
    }
    break;
  case SUBMENU_CORRIENTE_ACTIVACION:
  {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 10, "Prueba corrte de activacion");
    u8g2.sendBuffer();
    delay(10);
    runTestCorriente();
    if (automatico) {
      estadoActual = SUBMENU_TIEMPO_RESPUESTA;   // avanza al siguiente
    } else {
      devolver();                     // modo manual: vuelves al menú
    }

    break;
  }
  case SUBMENU_TIEMPO_RESPUESTA:
  {

    // u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 10, "Prueba tmpo respuesta");
    u8g2.sendBuffer();
    delay(10);
    runTestTiempoRespuesta();
    if (automatico) {
      estadoActual = SUBMENU_FLUJO;   // avanza al siguiente
    } else {
      devolver();                     // modo manual: vuelves al menú
    }
    break;
  }
  case SUBMENU_FLUJO:
  {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 10, "Prueba de caudal");
    u8g2.sendBuffer();
    delay(10);
    runTestCaudal();
    if (automatico) {
      estadoActual = SUBMENU_TEMPERATURA;   // avanza al siguiente
    } else {
      devolver();                     // modo manual: vuelves al menú
    }

    break;
  }
  case SUBMENU_TEMPERATURA:
  {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 10, "Prueba temperatura");
    u8g2.sendBuffer();
    delay(10);
    runTestTemperatura();
    if (automatico) {
      estadoActual = SUBMENU_RESULTADOS;
    } else {
      devolver();
    }
    break; 
  }
  case SUBMENU_RESULTADOS:
  {
    delay (200);
    u8g2.clearBuffer();
    automatico = false;   // desactivamos el modo automático
    mostrarResultadosFinales();
    mostrarSoluciones();
       
    break;
  }
 
  case ATRASITO:
    estadoActual = SUBMENU_FUNCIONAMIENTO;
    break;

  case AYUDA:

    mostrarAyuda();
    estadoActual = MENU_PRINCIPAL;
    break;
  default:
    break;
  }
}

// Cambia el estado del sistema según la selección del usuario
void manejarEstado()
{
  switch (estadoActual)
  {
  case MENU_PRINCIPAL:
    if (indiceMenu == 0)
    {
     
      estadoActual = MENU_SELECCION_INYECTOR;
    }
    else if (indiceMenu == 1)
    {

      estadoActual = AGREGAR_BORRAR_INJ;
    }
    else if (indiceMenu == 2)
    {

      estadoActual = AYUDA;
    }
    break;
  case AGREGAR_BORRAR_INJ:
    if (indiceagregarborrarinjt == 0)
    {
      Serial.println("vamos agregar");
      estadoActual = AGREGAR;
    }
    else if (indiceagregarborrarinjt == 1)
    {
      Serial.println("vamos borrar");
      estadoActual = BORRAR;
    }
    else if (indiceagregarborrarinjt == 2)
    {
      Serial.println("vamos menu principal");
      estadoActual = MENU_PRINCIPAL;
    }
    break;
  case MENU_SELECCION_INYECTOR:
    // Ejemplo: al seleccionar un inyector, pasar al menú de selección de motocicleta
    estadoActual = SELECCIONAR_MOTO;
    // currentState = SELECT_RPM;
    break;
  case SELECCIONAR_RPM_TIME:

    break;
  case SELECCIONAR_MOTO:
    // delay(50);
    if (indiceseleccionarmoto == 0)
    {
      estadoActual = SUBMENU_FUNCIONAMIENTO;
    }
    else if (indiceseleccionarmoto == 1)
    {
      estadoActual = MENU_CARACTERISTICAS;
    }
    else if (indiceseleccionarmoto == 2)
    {
      // lastEncoderMoveTime = 0;
      // encoder.clearCount();
      // delay(500);
      estadoActual = MENU_PRINCIPAL;
    }
    break;
  case SUBMENU_FUNCIONAMIENTO:
    automatico = false;
    Serial.print("  automatico=");
    Serial.println(automatico);
    if (indicefuncionamiento == 0)
    {
      estadoActual = SUBMENU_MANUAL;
    }
    else if (indicefuncionamiento == 1)
    {
      estadoActual = SUBMENU_AUTOMATICO;
    }
    else if (indicefuncionamiento == 2)
    {
      estadoActual = SELECCIONAR_MOTO;
    }
    break;
  case SUBMENU_MANUAL:
    if (indiceSubSubMenu == 0)
    {
      estadoActual = SUBMENU_MID_RES;
    }
    else if (indiceSubSubMenu == 1)
    {
      estadoActual = SUBMENU_FUGAS;
    }
    else if (indiceSubSubMenu == 2)
    {
      estadoActual = SUBMENU_CLICK;
    }
    else if (indiceSubSubMenu == 3)
    {
      estadoActual = SUBMENU_CORRIENTE_ACTIVACION;
    }
    else if (indiceSubSubMenu == 4)
    {
      estadoActual = SUBMENU_TIEMPO_RESPUESTA;
    }
    else if (indiceSubSubMenu == 5)
    {
      estadoActual = SUBMENU_FLUJO;
    }
    else if (indiceSubSubMenu == 6)
    {
      estadoActual = SUBMENU_TEMPERATURA;
    }
    else if (indiceSubSubMenu == 7)
    {
      estadoActual = SUBMENU_RESULTADOS;
    }
    else if (indiceSubSubMenu == 8)
    {
      // lastEncoderMoveTime = 0;
      // encoder.clearCount();
      // delay(500);
      estadoActual = SUBMENU_FUNCIONAMIENTO;
    }
    break;
    case SUBMENU_AUTOMATICO :
    {
      // runAllTests();
      automatico    = true;
      estadoActual = SUBMENU_MID_RES; 
    }
    break;
  default:
    break;
  }
}

// Actualiza los índices de selección según la dirección del giro del encoder
void actualizarIndice(bool incremento)
{
  switch (estadoActual)
  {
  case MENU_PRINCIPAL:
    if (incremento)
    {
      indiceMenu = (indiceMenu + 1) % tamaniomenuprincipal;
    }
    else
    {
      indiceMenu = (indiceMenu - 1 + tamaniomenuprincipal) % tamaniomenuprincipal;
    }

    break;
  case SELECCIONAR_MOTO:
    if (incremento)
    {
      indiceseleccionarmoto = (indiceseleccionarmoto + 1) % tamanioseleccionarmoto;
    }
    else
    {
      indiceseleccionarmoto = (indiceseleccionarmoto - 1 + tamanioseleccionarmoto) % tamanioseleccionarmoto;
    }
    break;
  case AGREGAR_BORRAR_INJ:
    if (incremento)
    {
      indiceagregarborrarinjt = (indiceagregarborrarinjt + 1) % tamanioagregarborrarinj;
    }
    else
    {
      indiceagregarborrarinjt = (indiceagregarborrarinjt - 1 + tamanioagregarborrarinj) % tamanioagregarborrarinj;
    }
    break;
  case SUBMENU_FUNCIONAMIENTO:
    if (incremento)
    {
      indicefuncionamiento = (indicefuncionamiento + 1) % tamaniosubMenuFuncionamiento;
    }
    else
    {
      indicefuncionamiento = (indicefuncionamiento - 1 + tamaniosubMenuFuncionamiento) % tamaniosubMenuFuncionamiento;
    }
    break;
  case SUBMENU_MANUAL:
    if (incremento)
    {
      indiceSubSubMenu = (indiceSubSubMenu + 1) % tamanioSubMenuManual;
    }
    else
    {
      indiceSubSubMenu = (indiceSubSubMenu - 1 + tamanioSubMenuManual) % tamanioSubMenuManual;
    }
    break;
  case AGREGAR:
    if (incremento)
    {
      indiceSubSubMenu = (indiceSubSubMenu + 1) % tamanioSubMenuManual;
    }
    else
    {
      indiceSubSubMenu = (indiceSubSubMenu - 1 + tamanioSubMenuManual) % tamanioSubMenuManual;
    }
    break;

  // Agrega otros casos si es necesario
  default:
    break;
  }
}
