#ifndef INYECTORES_H

#include <Arduino.h>
#define INYECTORES_H
#define MAX_INJECTORES 10    // Número máximo de inyectores personalizados
#define MAX_MODEL_LEN 30     // Longitud máxima del nombre/modelo
#define NUM_DEFAULT_INJECTORES 3  // Número de inyectores de fábrica



struct InjectorData {
    char modelo[MAX_MODEL_LEN];   // Nombre o modelo
    double resistencia;
    float caudal;
    float fugas;
    float tiempoRespuesta;
    float corrienteActivacion;
    float temperaturaOperativa;
    float sonidoActivacion;
 };
 
 // Declaraciones de variables globales
 extern InjectorData inyectores[MAX_INJECTORES];  // Personalizados (almacenados en NVS)
 extern uint8_t numInyectores;
// Prototipos de las funciones principales
void setup_save();
void loop_save();
void handleAddInjector();
void handleDeleteData();
void handleShowInjector();
void handleTestInjector();
void cargarDatos();
int selectMenuOption(const char* title, const char* options[], int numOptions) ;
extern InjectorData selected; // Declaración externa
  
#endif // INYECTORES_H