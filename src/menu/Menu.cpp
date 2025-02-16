#include "Menu.h"
#include "displayhandler/DisplayHandler.h"
#include "saveInjector/saveInjector.h"
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
    "prueba_medir_resistencia", "prueba_de_llenado", "prueba_de_goteo", "prueba_medir_clic",
    "prueba_abanico", "resultados", "devolver"};
static const int tamanioSubMenuManual = sizeof(submenuManualImagenes) / sizeof(submenuManualImagenes[0]);

static const char *agregarBorrarImagenes[] = {"AGREGAR_INJ", "BORRAR_INJ", "ATRASAGREGARINYECTOR_INJ"};
static const int tamanioagregarborrarinj = sizeof(agregarBorrarImagenes) / sizeof(agregarBorrarImagenes[0]);

static const char *seleccionarMotoImagenes[] = {"SIGUIENTEFUNCIONAMIENTO", "VER_CARACT", "ATRASMENU"};
static const int tamanioseleccionarmoto = sizeof(seleccionarMotoImagenes) / sizeof(seleccionarMotoImagenes[0]);

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
    // Aquí puedes agregar la lógica para este menú
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
  case AGREGAR:
    Serial.println("hola voy agregar");
    setup_save();
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
