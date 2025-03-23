#include <Preferences.h>
#include "saveInjector.h"
#include "Config.h"
#include "encoderhadler/EncoderHandler.h"
#include "tecladovirtual/TecladoVirtual.h"
#include "GlobalVarials/GlobalVarials.h"

// Preferences para NVS (almacenamiento de datos personalizados)
Preferences preferences;
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern ESP32Encoder encoder;
// ====== ESTRUCTURA Y DATOS DE INYECTORES ======

// struct InjectorData {
//    char modelo[MAX_MODEL_LEN];   // Nombre o modelo
//    float resistencia;
//    float caudal;
//   float fugas;
//    float tiempoRespuesta;
//    float corrienteActivacion;
//    float temperaturaOperativa;
//    float sonidoActivacion;
// };

InjectorData inyectores[MAX_INJECTORES]; // Personalizados (almacenados en NVS)
uint8_t numInyectores = 0;

// Datos de fábrica (por defecto)
const InjectorData defaultInyectores[NUM_DEFAULT_INJECTORES] = {
    {"ns 200", 13.0, 5.0, 0.5, 0.8, 3.2, 80.0, 40.0},
    {"fz 2.0", 13.0, 6.0, 0.4, 1.0, 3.0, 85.0, 40.0},
    {"nkd", 13.0, 5.5, 0.6, 0.7, 3.5, 78.0, 40.0}};

// ====== FUNCIONES DE NVS ======
void cargarDatos()
{
  preferences.begin("inyectorData", true);
  numInyectores = (uint8_t)preferences.getUInt("num", 0);
  if (numInyectores > 0)
  {
    preferences.getBytes("data", &inyectores, sizeof(inyectores));
  }
  preferences.end();
}

void guardarDatos()
{
  preferences.begin("inyectorData", false);
  preferences.putUInt("num", numInyectores);
  preferences.putBytes("data", &inyectores, sizeof(inyectores));
  preferences.end();
}

// ====== FUNCIONES DE INTERFAZ CON ENCODER Y OLED ======

// Espera a que se presione el botón del encoder (con simple debouncing)
void waitForButtonPress()
{
  encoder.clearCount();
  while (digitalRead(botonPin) == HIGH)
  {
    delay(10);
  }
  // Espera a soltar el botón
  while (digitalRead(botonPin) == LOW)
  {
    delay(10);
  }
  delay(200); // delay para evitar rebotes
}

// Muestra un menú con opciones y permite seleccionar mediante encoder
// int lastEncoderPosition = encoder.getCount();
// bool seleccionando = true;
int selectMenuOption(const char *title, const char *options[], int numOptions)
{
  long encoderPos = 0;
  int selected = 0;
  int offset = 0;        // Índice del primer elemento visible
  const int maxRows = 4; // Número máximo de filas a mostrar en pantalla
  encoder.clearCount();  // Reinicia la cuenta del encoder

  while (true)
  {
    // Leer posición del encoder
    long newPos = encoder.getCount();
    if (newPos != encoderPos)
    {
      encoderPos = newPos;
      selected = (int)(encoderPos % numOptions);
      if (selected < 0)
        selected += numOptions;

      // Ajustar el offset para que el elemento seleccionado siempre sea visible
      if (selected < offset)
      {
        offset = selected;
      }
      if (selected >= offset + maxRows)
      {
        offset = selected - maxRows + 1;
      }
    }

    // Dibujar el menú en la pantalla OLED
    u8g2.firstPage();
    do
    {
      u8g2.setFont(u8g2_font_6x12_tf);
      u8g2.drawStr(0, 10, title);

      // Mostrar sólo "maxRows" elementos a partir del índice "offset"
      for (int i = 0; i < maxRows; i++)
      {
        int idx = offset + i;
        if (idx >= numOptions)
          break;
        int y = 20 + i * 12;
        if (idx == selected)
        {
          u8g2.drawStr(0, y, ">");
        }
        u8g2.drawStr(10, y, options[idx]);
      }
    } while (u8g2.nextPage());

    // Si se presiona el botón, retorna la opción seleccionada
    if (digitalRead(botonPin) == LOW)
    {
      waitForButtonPress();
      return selected;
    }
    delay(50);
  }
}

// Permite ingresar un valor flotante mediante encoder.
// 'step' define el incremento, y se muestra el valor actual junto con el prompt.
float getFloatInput(const char *prompt, float initial, float step, float minVal, float maxVal)
{
  long encoderPos = 0;
  float value = initial;
  encoder.clearCount();
  while (true)
  {
    int newPos = encoder.getCount();
    if (newPos != encoderPos)
    {
      encoderPos = newPos;
      value = initial + (encoderPos * step);
      if (value < minVal)
        value = minVal;
      if (value > maxVal)
        value = maxVal;
    }

    // Dibujar pantalla de entrada
    char buf[32];
    sprintf(buf, "%.2f", value);
    u8g2.firstPage();
    do
    {
      u8g2.setFont(u8g2_font_6x12_tf);
      u8g2.drawStr(0, 10, prompt);
      u8g2.drawStr(0, 30, buf);
      u8g2.drawStr(0, 50, "Gire para ajustar");
      u8g2.drawStr(0, 62, "Presione para OK");
    } while (u8g2.nextPage());

    // Si se presiona el botón, confirma y retorna el valor
    if (digitalRead(botonPin) == LOW)
    {
      waitForButtonPress();
      return value;
    }
    delay(50);
  }
}

// Muestra un mensaje en la OLED por unos segundos (usado para notificaciones)
void showMessage(const char *msg, int delayMs = 1500)
{
  u8g2.firstPage();
  do
  {
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 30, msg);
  } while (u8g2.nextPage());
  delay(delayMs);
}

// Permite elegir entre "Usar Default" o "Ingresar valor" para cada parámetro
float inputParameter(const char *paramName, float defaultVal)
{
  const char *options[2] = {"Usar Default", "Ingresar valor"};
  char title[40];
  sprintf(title, "%s?", paramName);
  int sel = selectMenuOption(title, options, 2);
  if (sel == 0)
  {
    // Mostrar mensaje y retornar valor default
    char buf[40];
    sprintf(buf, "Default: %.2f", defaultVal);
    showMessage(buf, 1000);
    return defaultVal;
  }
  else
  {
    // Permite ingresar valor (se parte del default)
    char prompt[40];
    sprintf(prompt, "%s (min=0)", paramName);
    return getFloatInput(prompt, defaultVal, 0.1, 0, 1000);
  }
}

// Permite ingresar un string (para el modelo) usando el encoder es complejo;
// en este ejemplo se asume que para el nombre se usa un valor predefinido o se selecciona de un menú simple.
String inputModelo()
{

  showMessage("Digite el nombre del inyector", 1500);
  String modelo = tecladoVirtual();
  // Si el usuario presiona la tecla que indica "cancelar" (por ejemplo, devolviendo una cadena vacía)
  // puedes asignar un valor por defecto o volver a llamar a la función según lo necesites.
  if (modelo.length() == 0)
  {
    modelo = "ModeloDefault";
  }
  return modelo;
}

// Estado: Agregar un inyector personalizado
void handleAddInjector()
{

  delay(1000);
  Serial.println("agregando inyector");
  InjectorData nuevo;
  // Ingresar (o seleccionar) el modelo
  String mod = inputModelo();
  mod.toCharArray(nuevo.modelo, MAX_MODEL_LEN);

  // Para cada parámetro, se permite usar valor default o ingresar uno
  // Aquí se usan como referencia los valores de "ModeloA"
  nuevo.resistencia = inputParameter("Resistencia", 10.0);
  nuevo.caudal = inputParameter("Caudal", 5.0);
  nuevo.fugas = inputParameter("Fugas", 0.5);
  nuevo.tiempoRespuesta = inputParameter("Tiempo Resp.", 0.8);
  nuevo.corrienteActivacion = inputParameter("Corriente", 3.2);
  nuevo.temperaturaOperativa = inputParameter("Temperatura", 80.0);
  nuevo.sonidoActivacion = inputParameter("Sonido", 2.5);

  if (numInyectores < MAX_INJECTORES)
  {
    inyectores[numInyectores] = nuevo;
    numInyectores++;
    guardarDatos();
    showMessage("Inyector agregado", 1500);
  }
  else
  {
    showMessage("Max. inyectores alcanzados", 1500);
  }
  estadoActual = AGREGAR_BORRAR_INJ;
}

// funcion para mostrar los inyectores y seleccionar
// Función que combina los inyectores y permite seleccionarlos mediante encoder.
// Retorna la estructura InjectorData del inyector seleccionado.

InjectorData selectInjector()
{
  // Se calcula el total de inyectores disponibles (fábrica + personalizados)
  int total = NUM_DEFAULT_INJECTORES + numInyectores;

  // Si no hay inyectores (caso poco probable), se retorna un inyector vacío.
  if (total == 0)
  {
    InjectorData dummy;
    strcpy(dummy.modelo, "No Inyectores");
    dummy.resistencia = 0;
    dummy.caudal = 0;
    dummy.fugas = 0;
    dummy.tiempoRespuesta = 0;
    dummy.corrienteActivacion = 0;
    dummy.temperaturaOperativa = 0;
    dummy.sonidoActivacion = 0;
    return dummy;
  }

  // Arreglo para formar las opciones a mostrar en el menú
  char opciones[total][MAX_MODEL_LEN + 15]; // +15 para incluir etiqueta y espacios
  // Se agregan primero los inyectores de fábrica
  for (int i = 0; i < NUM_DEFAULT_INJECTORES; i++)
  {
    sprintf(opciones[i], "[F] %s", defaultInyectores[i].modelo);
  }
  // Luego, se agregan los inyectores personalizados
  for (int j = 0; j < numInyectores; j++)
  {
    sprintf(opciones[NUM_DEFAULT_INJECTORES + j], "[P] %s", inyectores[j].modelo);
  }

  // Convertir el arreglo a un arreglo de punteros a char
  const char *opts[total];
  for (int i = 0; i < total; i++)
  {
    opts[i] = opciones[i];
  }

  // Mostrar el menú y obtener la selección mediante encoder
  int sel = selectMenuOption("Seleccione inyector", opts, total);

  // Retornar el inyector correspondiente según la selección
  if (sel < NUM_DEFAULT_INJECTORES)
  {
    return defaultInyectores[sel];
  }
  else
  {
    return inyectores[sel - NUM_DEFAULT_INJECTORES];
  }
}

// Estado: Mostrar datos de un inyector personalizado
void handleShowInjector()
{

  // Crear un arreglo de opciones con los nombres de los inyectores personalizados
  const int total = numInyectores;
  char opciones[total][MAX_MODEL_LEN];
  for (int i = 0; i < total; i++)
  {
    strncpy(opciones[i], inyectores[i].modelo, MAX_MODEL_LEN);
  }
  // Convertir arreglo de char* para el menú
  const char *opts[total];
  for (int i = 0; i < total; i++)
  {
    opts[i] = opciones[i];
  }
  int sel = selectMenuOption("Inyectores:", opts, total);
  InjectorData selected = inyectores[sel];

  // Mostrar detalles en la pantalla (puede mostrar varios renglones)
  u8g2.firstPage();
  do
  {
    u8g2.setFont(u8g2_font_6x12_tf);
    u8g2.drawStr(0, 10, selected.modelo);
    char buf[32];
    sprintf(buf, "Res:%.2fCaudal:%.2f", selected.resistencia, selected.caudal);
    u8g2.drawStr(0, 22, buf);
    sprintf(buf, "Fugas:%.2fTiempo:%.2f", selected.fugas, selected.tiempoRespuesta);
    u8g2.drawStr(0, 34, buf);
    sprintf(buf, "Corr: %.2fTemp:%.2f", selected.corrienteActivacion, selected.temperaturaOperativa);
    u8g2.drawStr(0, 46, buf);
    sprintf(buf, "Sonido:%.2f", selected.sonidoActivacion);
    u8g2.drawStr(0, 58, buf);
  } while (u8g2.nextPage());
  delay(5000);
  estadoActual = SELECCIONAR_MOTO;
}

// Estado: Realizar prueba de un inyector seleccionar inyector
void handleTestInjector()
{
  // Se mostrará una lista combinada de inyectores de fábrica y personalizados
  delay(1000);
  const int totalInyectores = NUM_DEFAULT_INJECTORES + numInyectores;
  char opciones[totalInyectores][MAX_MODEL_LEN + 15]; // para indicar fuente
  for (int i = 0; i < NUM_DEFAULT_INJECTORES; i++)
  {
    sprintf(opciones[i], "[F] %s", defaultInyectores[i].modelo);
  }
  for (int j = 0; j < numInyectores; j++)
  {
    sprintf(opciones[NUM_DEFAULT_INJECTORES + j], "[P] %s", inyectores[j].modelo);
  }
  const char *opts[totalInyectores];
  for (int i = 0; i < totalInyectores; i++)
  {
    opts[i] = opciones[i];
  }
  int sel = selectMenuOption("Seleccione inyector", opts, totalInyectores);
  InjectorData selected;

  if (sel < NUM_DEFAULT_INJECTORES) {
      selected = defaultInyectores[sel];
      globalInjector.setInjector(selected);
  } else {
    selected = inyectores[sel - NUM_DEFAULT_INJECTORES];
    globalInjector.setInjector(selected);
  }

  estadoActual = SELECCIONAR_RPM_TIME;
}
 

// Estado: Borrar datos personalizados
void handleDeleteData()
{
  numInyectores = 0;
  guardarDatos();
  showMessage("Datos borrados", 1500);
  estadoActual = AGREGAR_BORRAR_INJ;
}

// -----------------------------------------------------------------------------
// IMPLEMENTACIÓN DE LAS PRUEBAS (estas funciones pueden basarse en lecturas ADC, etc.)
// Los siguientes ejemplos son esquemáticos y deberán ajustarse según tus sensores y calibración.
// -----------------------------------------------------------------------------

// 1. Prueba de Resistencia Eléctrica