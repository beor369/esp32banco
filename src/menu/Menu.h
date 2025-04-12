#ifndef MENU_H
#define MENU_H

#include "Config.h"
#include "InjectorController/InjectorController.h"

typedef void (*Callback)();
enum EstadoInyector
{
  INYECTOR_DESACTIVADO,
  ACTIVA_INYECTOR_SIN_REVOLUCION,
  ACTIVA_INYECTOR_CON_REVOLUCION,
  INJECTOR_MIN
};
namespace estate_inyector
{
  constexpr EstadoInyector INYECTOR_DESACTIVADO = EstadoInyector::INYECTOR_DESACTIVADO;
  constexpr EstadoInyector ACTIVA_INYECTOR_SIN_REVOLUCION = EstadoInyector::ACTIVA_INYECTOR_SIN_REVOLUCION;
  constexpr EstadoInyector ACTIVA_INYECTOR_CON_REVOLUCION = EstadoInyector::ACTIVA_INYECTOR_CON_REVOLUCION;
  constexpr EstadoInyector INJECTOR_MIN = EstadoInyector::INJECTOR_MIN;
}
// Funciones para manejar y mostrar el menú
void mostrarMenu();
void manejarEstado();
void actualizarIndice(bool incremento);
void controlBombaDurantePrueba(unsigned long tiempoBombaEncendidaS,unsigned long tiempoBombaApagadaS,long time_injector,Callback actualizaValores,EstadoInyector estadoInyector);
void setupvalues();
#endif
