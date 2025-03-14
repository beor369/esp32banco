#include "Menu.h"
#include "displayhandler/DisplayHandler.h"
#include "saveInjector/saveInjector.h"
#include "pruebas/pruebas.h"
#include "pruebas/activatebankofproof/activatebankofproof.h"
#include "selectrpm/selectrpm.h"
#include "flow_sensor/flow_sensor.h"

#include "InjectorController/InjectorController.h"
// Declaración de variables globales (definidas en main.cpp)
extern int indiceMenu;
extern int indiceInyector;
extern int indiceSubSubMenu;
extern int indicefuncionamiento;
extern int indiceseleccionarmoto;
extern int indiceagregarborrarinjt;
extern Estado estadoActual;

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

typedef void (*Callback)();

void controlBombaDurantePrueba(unsigned long tiempoBombaEncendidaS, unsigned long tiempoBombaApagadaS, Callback actualizaValores)
{
  unsigned long tiempoBombaEncendidaMs = tiempoBombaEncendidaS * 1000;
  unsigned long tiempoBombaApagadaMs = tiempoBombaApagadaS * 1000;
  unsigned long inicioCiclo = millis();
  bool bombaEncendida = true;

  digitalWrite(BOMBA_PIN, LOW); // Enciende la bomba
  inyector.activarInyectorDesdeEncoder();

  while (!(inyector.isActive && (millis() - inyector.startTime >= inyector.testDurationMs)))
  {

    // inyector.assess_ina();
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

  digitalWrite(BOMBA_PIN, HIGH);
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
    beep();
      
    delay(1000);

    break;
  case SUBMENU_FUGAS:
    // Test de Fugas
    char outputt[50];
    runTestFugas(selected);

    u8g2.clearBuffer();
    u8g2.drawStr(0, 12, outputt);
    u8g2.sendBuffer();
    delay(5000);

    estadoActual = SUBMENU_MANUAL;

    break;
  case SUBMENU_CLICK:

    inyector.activate(50.0, 2500, 5);

    while (!(inyector.isActive && (millis() - inyector.startTime >= inyector.testDurationMs)))
    {
      char output[50];
      u8g2.clearBuffer();
      Serial.println("SUBMENU CLICK");
      // pruebaClic();

       runTestSonido(selected);
      // sprintf(output, "Sonido: %.2f %s", result.measuredValue, result.passed ? "OK" : "FALLA");
      u8g2.clearBuffer();
      u8g2.drawStr(0, 12, output);
      u8g2.sendBuffer();
    }

    estadoActual = SUBMENU_MANUAL;
    break;
  case SUBMENU_CORRIENTE_ACTIVACION:

    break;
  case SUBMENU_TIEMPO_RESPUESTA:

    break;
  case SUBMENU_FLUJO:
    controlBombaDurantePrueba(3, 10, setupvalues);
    Serial.println(resultadosTests["Caudal"].measuredValue);
    
    // digitalWrite(BOMBA_PIN, LOW);
    // inyector.activarInyectorDesdeEncoder();

    // while (!(inyector.isActive && (millis() - inyector.startTime >= inyector.testDurationMs)))
    // {
    //   setupvalues();
    // }

    // digitalWrite(BOMBA_PIN, HIGH);
    estadoActual = SUBMENU_MANUAL;
    break;
  case SUBMENU_TEMPERATURA:
    char outputtt[50];
    runTestTemperatura(selected);
    // sprintf(outputtt, "Temperatura: %.2f %s", result.measuredValue, result.passed ? "OK" : "FALLA");
    u8g2.clearBuffer();
    u8g2.drawStr(0, 12, outputtt);
    u8g2.sendBuffer();
    delay(2000);

    break;
  case SUBMENU_RESULTADOS:

    break;
  case ATRASITO:

    break;

  case AYUDA:
    //  // Prueba 3: 2 Hz, 1000 µs (umbral crítico)

    // delay(10000);
    // delay(10000);
    // Detener manualmente:
    // inyector.stop();
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
      estadoActual = MENU_PRINCIPAL;
    }
    break;
  case SUBMENU_FUNCIONAMIENTO:
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
      estadoActual = ATRASITO;
    }
    break;
  // Agrega más casos según lo requiera tu lógica
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
