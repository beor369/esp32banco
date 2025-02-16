#include <Preferences.h>

#define MAX_INJECTORES         10    // Número máximo de inyectores personalizados
#define MAX_MODEL_LEN          30    // Longitud máxima del nombre/modelo
#define NUM_DEFAULT_INJECTORES 3     // Número de inyectores de fábrica (por defecto)

// Estructura que contiene los parámetros ideales de un inyector
struct InjectorData {
  char modelo[MAX_MODEL_LEN];   // Nombre o modelo
  float resistencia;
  float caudal;
  float fugas;
  float tiempoRespuesta;
  float corrienteActivacion;
  float temperaturaOperativa;
  float sonidoActivacion;
};

// Arreglo global para los inyectores personalizados y contador
InjectorData inyectores[MAX_INJECTORES];
uint8_t numInyectores = 0;

// Arreglo constante con inyectores de fábrica (por defecto)
const InjectorData defaultInyectores[NUM_DEFAULT_INJECTORES] = {
  {"ModeloA", 10.0, 5.0, 0.5, 0.8, 3.2, 80.0, 2.5},
  {"ModeloB", 12.0, 6.0, 0.4, 1.0, 3.0, 85.0, 2.0},
  {"ModeloC",  9.5, 5.5, 0.6, 0.7, 3.5, 78.0, 2.7}
};

Preferences preferences;  // Para manejar la NVS

// -------------------- Funciones de almacenamiento en NVS --------------------

// Carga los datos personalizados almacenados en la NVS
void cargarDatos() {
  preferences.begin("inyectorData", true);  // Modo lectura
  numInyectores = (uint8_t) preferences.getUInt("num", 0);
  if (numInyectores > 0) {
    preferences.getBytes("data", &inyectores, sizeof(inyectores));
  }
  preferences.end();
}

// Guarda los datos personalizados en la NVS
void guardarDatos() {
  preferences.begin("inyectorData", false);  // Modo lectura/escritura
  preferences.putUInt("num", numInyectores);
  preferences.putBytes("data", &inyectores, sizeof(inyectores));
  preferences.end();
}

// -------------------- Funciones de entrada por Serial --------------------

// Función para leer un valor float SIN opción default
float leerFloat(String prompt) {
  Serial.print(prompt);
  while (true) {
    if (Serial.available()) {
      String entrada = Serial.readStringUntil('\n');
      entrada.trim();
      float valor = entrada.toFloat();
      // Se acepta "0" solo si se ingresa "0" o "0.0"
      if (valor == 0 && entrada != "0" && entrada != "0.0") {
        Serial.println("Valor no válido. Intente nuevamente.");
        Serial.print(prompt);
      } else {
        return valor;
      }
    }
  }
}

// Función para leer un valor float con opción de usar el valor por defecto
float leerFloatConDefault(String prompt, float valorDefault) {
  Serial.print(prompt);
  while (true) {
    if (Serial.available()) {
      String entrada = Serial.readStringUntil('\n');
      entrada.trim();
      // Si el usuario ingresa "D" o "d", se usa el valor por defecto
      if (entrada.equalsIgnoreCase("d")) {
        Serial.print("Usando valor por defecto: ");
        Serial.println(valorDefault);
        return valorDefault;
      }
      float valor = entrada.toFloat();
      if (valor == 0 && entrada != "0" && entrada != "0.0") {
        Serial.println("Valor no válido. Intente nuevamente.");
        Serial.print(prompt);
      } else {
        return valor;
      }
    }
  }
}

// Lee una cadena (String) desde el monitor serial
String leerString(String prompt) {
  Serial.print(prompt);
  while (true) {
    if (Serial.available()) {
      String entrada = Serial.readStringUntil('\n');
      entrada.trim();
      if (entrada.length() == 0) {
        Serial.println("Entrada vacía. Intente nuevamente.");
        Serial.print(prompt);
      } else {
        return entrada;
      }
    }
  }
}

// Lee un número entero desde el monitor serial
int leerEntero(String prompt) {
  Serial.print(prompt);
  while (true) {
    if (Serial.available()) {
      String entrada = Serial.readStringUntil('\n');
      entrada.trim();
      int valor = entrada.toInt();
      if (valor == 0 && entrada != "0") {
        Serial.println("Valor no válido. Intente nuevamente.");
        Serial.print(prompt);
      } else {
        return valor;
      }
    }
  }
}

// -------------------- Funciones del Banco de Inyectores --------------------

// Agrega un nuevo inyector personalizado; para cada parámetro, el usuario puede ingresar un valor o teclear "D" para usar el valor por defecto.
void agregarInyector() {
  if (numInyectores >= MAX_INJECTORES) {
    Serial.println("Se alcanzó el número máximo de inyectores personalizados.");
    return;
  }
  
  InjectorData nuevoInyector;
  String modelo = leerString("Ingrese el modelo o nombre del inyector: ");
  modelo.toCharArray(nuevoInyector.modelo, MAX_MODEL_LEN);
  
  Serial.println("Ingrese los valores ideales (parámetros de referencia).");
  Serial.println("Para usar el valor por defecto, ingrese 'D'.");
  
  // Se utilizan como referencia los valores del inyector de fábrica "ModeloA"
  nuevoInyector.resistencia         = leerFloatConDefault("  Resistencia eléctrica: ", 10.0);
  nuevoInyector.caudal              = leerFloatConDefault("  Caudal de combustible: ", 5.0);
  nuevoInyector.fugas               = leerFloatConDefault("  Fugas en reposo: ", 0.5);
  nuevoInyector.tiempoRespuesta     = leerFloatConDefault("  Tiempo de respuesta: ", 0.8);
  nuevoInyector.corrienteActivacion = leerFloatConDefault("  Corriente de activación: ", 3.2);
  nuevoInyector.temperaturaOperativa= leerFloatConDefault("  Temperatura operativa: ", 80.0);
  nuevoInyector.sonidoActivacion    = leerFloatConDefault("  Sonido de activación: ", 2.5);
  
  inyectores[numInyectores] = nuevoInyector;
  numInyectores++;
  guardarDatos();
  
  Serial.println("Datos del inyector personalizado agregados exitosamente.");
}

// Muestra los datos de un inyector personalizado (buscado por modelo)
void mostrarInyector() {
  if (numInyectores == 0) {
    Serial.println("No hay inyectores personalizados almacenados.");
    return;
  }
  
  String modeloBusqueda = leerString("Ingrese el modelo o nombre del inyector a mostrar: ");
  bool encontrado = false;
  
  for (int i = 0; i < numInyectores; i++) {
    if (modeloBusqueda.equalsIgnoreCase(inyectores[i].modelo)) {
      Serial.print("\n--- Datos del inyector personalizado: ");
      Serial.print(inyectores[i].modelo);
      Serial.println(" ---");
      Serial.print("  Resistencia: ");           Serial.println(inyectores[i].resistencia);
      Serial.print("  Caudal: ");                Serial.println(inyectores[i].caudal);
      Serial.print("  Fugas: ");                 Serial.println(inyectores[i].fugas);
      Serial.print("  Tiempo de respuesta: ");   Serial.println(inyectores[i].tiempoRespuesta);
      Serial.print("  Corriente de activación: "); Serial.println(inyectores[i].corrienteActivacion);
      Serial.print("  Temperatura operativa: ");   Serial.println(inyectores[i].temperaturaOperativa);
      Serial.print("  Sonido de activación: ");    Serial.println(inyectores[i].sonidoActivacion);
      Serial.println("-------------------------------------------");
      encontrado = true;
      break;
    }
  }
  
  if (!encontrado) {
    Serial.println("Modelo o nombre no encontrado en inyectores personalizados.");
  }
}

// Muestra en una lista todos los inyectores disponibles (de fábrica y personalizados)
void mostrarTodosInyectores() {
  Serial.println("\n--- Lista de inyectores disponibles ---");
  // Inyectores de fábrica
  for (int i = 0; i < NUM_DEFAULT_INJECTORES; i++) {
    Serial.print(i + 1);
    Serial.print(". [Fábrica] ");
    Serial.println(defaultInyectores[i].modelo);
  }
  // Inyectores personalizados
  for (int j = 0; j < numInyectores; j++) {
    Serial.print(NUM_DEFAULT_INJECTORES + j + 1);
    Serial.print(". [Personalizado] ");
    Serial.println(inyectores[j].modelo);
  }
  Serial.println("----------------------------------------");
}

// Función auxiliar: determina si un valor medido está dentro del ±10% del valor ideal
bool enRango(float sensor, float ideal) {
  if (ideal == 0) return (sensor == 0);
  float tolerancia = ideal * 0.10;
  return (sensor >= (ideal - tolerancia)) && (sensor <= (ideal + tolerancia));
}

// Realiza la prueba de un inyector, mostrando primero la lista combinada y luego un submenú para ver características, continuar o volver.
void testInyector() {
  mostrarTodosInyectores();
  
  int totalInyectores = NUM_DEFAULT_INJECTORES + numInyectores;
  int seleccion = leerEntero("Seleccione el número del inyector a probar: ");
  
  if (seleccion < 1 || seleccion > totalInyectores) {
    Serial.println("Número de inyector inválido.");
    return;
  }
  
  InjectorData selectedInyector;
  if (seleccion <= NUM_DEFAULT_INJECTORES) {
    selectedInyector = defaultInyectores[seleccion - 1];
  } else {
    selectedInyector = inyectores[seleccion - NUM_DEFAULT_INJECTORES - 1];
  }
  
  bool salirSubmenu = false;
  while (!salirSubmenu) {
    Serial.println("\nInyector seleccionado: " + String(selectedInyector.modelo));
    Serial.println("Seleccione una opción:");
    Serial.println("1. Ver características del inyector");
    Serial.println("2. Continuar con la prueba");
    Serial.println("3. Volver al menú principal");
    int opcionSubmenu = leerEntero("Ingrese su opción: ");
    
    if (opcionSubmenu == 1) {
      Serial.println("\n--- Características del inyector ---");
      Serial.print("Modelo: ");                   Serial.println(selectedInyector.modelo);
      Serial.print("Resistencia eléctrica: ");     Serial.println(selectedInyector.resistencia);
      Serial.print("Caudal de combustible: ");      Serial.println(selectedInyector.caudal);
      Serial.print("Fugas en reposo: ");           Serial.println(selectedInyector.fugas);
      Serial.print("Tiempo de respuesta: ");       Serial.println(selectedInyector.tiempoRespuesta);
      Serial.print("Corriente de activación: ");   Serial.println(selectedInyector.corrienteActivacion);
      Serial.print("Temperatura operativa: ");       Serial.println(selectedInyector.temperaturaOperativa);
      Serial.print("Sonido de activación: ");        Serial.println(selectedInyector.sonidoActivacion);
      Serial.println("------------------------------------");
      
      Serial.println("\n¿Desea continuar con la prueba?");
      Serial.println("1. Sí, continuar");
      Serial.println("2. Volver al menú principal");
      int opcionContinuar = leerEntero("Ingrese su opción: ");
      if (opcionContinuar == 1) {
        salirSubmenu = true;
      } else if (opcionContinuar == 2) {
        return;
      } else {
        Serial.println("Opción no válida, regresando al menú de selección.");
      }
    } 
    else if (opcionSubmenu == 2) {
      salirSubmenu = true;
    } 
    else if (opcionSubmenu == 3) {
      return;
    } 
    else {
      Serial.println("Opción no válida, intente nuevamente.");
    }
  }
  
  // Se continúa con la prueba
  Serial.print("\nRealizando prueba para el inyector: ");
  Serial.println(selectedInyector.modelo);
  
  float sensorResistencia         = leerFloat("Ingrese el valor medido de Resistencia eléctrica: ");
  float sensorCaudal              = leerFloat("Ingrese el valor medido de Caudal de combustible: ");
  float sensorFugas               = leerFloat("Ingrese el valor medido de Fugas en reposo: ");
  float sensorTiempoRespuesta     = leerFloat("Ingrese el valor medido de Tiempo de respuesta: ");
  float sensorCorrienteActivacion = leerFloat("Ingrese el valor medido de Corriente de activación: ");
  float sensorTemperaturaOperativa= leerFloat("Ingrese el valor medido de Temperatura operativa: ");
  float sensorSonidoActivacion    = leerFloat("Ingrese el valor medido de Sonido de activación: ");
  
  bool okResistencia         = enRango(sensorResistencia, selectedInyector.resistencia);
  bool okCaudal              = enRango(sensorCaudal, selectedInyector.caudal);
  bool okFugas               = enRango(sensorFugas, selectedInyector.fugas);
  bool okTiempoRespuesta     = enRango(sensorTiempoRespuesta, selectedInyector.tiempoRespuesta);
  bool okCorrienteActivacion = enRango(sensorCorrienteActivacion, selectedInyector.corrienteActivacion);
  bool okTemperaturaOperativa= enRango(sensorTemperaturaOperativa, selectedInyector.temperaturaOperativa);
  bool okSonidoActivacion    = enRango(sensorSonidoActivacion, selectedInyector.sonidoActivacion);
  
  Serial.println("\n--- Resultados de la prueba ---");
  
  Serial.print("Resistencia eléctrica: ");
  Serial.print(sensorResistencia);
  Serial.print("  | Ideal: ");
  Serial.print(selectedInyector.resistencia);
  Serial.print("  | ");
  Serial.println(okResistencia ? "OK" : "FUERA DE RANGO");
  
  Serial.print("Caudal de combustible: ");
  Serial.print(sensorCaudal);
  Serial.print("  | Ideal: ");
  Serial.print(selectedInyector.caudal);
  Serial.print("  | ");
  Serial.println(okCaudal ? "OK" : "FUERA DE RANGO");
  
  Serial.print("Fugas en reposo: ");
  Serial.print(sensorFugas);
  Serial.print("  | Ideal: ");
  Serial.print(selectedInyector.fugas);
  Serial.print("  | ");
  Serial.println(okFugas ? "OK" : "FUERA DE RANGO");
  
  Serial.print("Tiempo de respuesta: ");
  Serial.print(sensorTiempoRespuesta);
  Serial.print("  | Ideal: ");
  Serial.print(selectedInyector.tiempoRespuesta);
  Serial.print("  | ");
  Serial.println(okTiempoRespuesta ? "OK" : "FUERA DE RANGO");
  
  Serial.print("Corriente de activación: ");
  Serial.print(sensorCorrienteActivacion);
  Serial.print("  | Ideal: ");
  Serial.print(selectedInyector.corrienteActivacion);
  Serial.print("  | ");
  Serial.println(okCorrienteActivacion ? "OK" : "FUERA DE RANGO");
  
  Serial.print("Temperatura operativa: ");
  Serial.print(sensorTemperaturaOperativa);
  Serial.print("  | Ideal: ");
  Serial.print(selectedInyector.temperaturaOperativa);
  Serial.print("  | ");
  Serial.println(okTemperaturaOperativa ? "OK" : "FUERA DE RANGO");
  
  Serial.print("Sonido de activación: ");
  Serial.print(sensorSonidoActivacion);
  Serial.print("  | Ideal: ");
  Serial.print(selectedInyector.sonidoActivacion);
  Serial.print("  | ");
  Serial.println(okSonidoActivacion ? "OK" : "FUERA DE RANGO");
  
  if (okResistencia && okCaudal && okFugas && okTiempoRespuesta &&
      okCorrienteActivacion && okTemperaturaOperativa && okSonidoActivacion) {
    Serial.println("\nEl inyector PASA la prueba.");
  } else {
    Serial.println("\nEl inyector FALLA la prueba.");
  }
}

// Borra todos los datos personalizados almacenados en la NVS
void borrarDatos() {
  numInyectores = 0;
  guardarDatos();
  Serial.println("Todos los datos personalizados han sido borrados.");
}

// -------------------- Menú y Flujo Principal --------------------

void mostrarMenu2() {
  Serial.println("\n--- Menú Principal ---");
  Serial.println("1. Agregar datos de un inyector personalizado");
  Serial.println("2. Mostrar datos de un inyector personalizado");
  Serial.println("3. Realizar prueba de un inyector");
  Serial.println("4. Borrar todos los datos personalizados");
  Serial.print("Seleccione una opción: ");
}

void setup_save() {
  cargarDatos();
  Serial.println("Banco de prueba de inyectores automáticos para motocicletas");
  mostrarMenu2();
}

void loop_save() {
        if (Serial.available()) {
            String opcion = Serial.readStringUntil('\n');
            opcion.trim();
            
            if (opcion == "1") {
              agregarInyector();
            } 
            else if (opcion == "2") {
              mostrarInyector();
            } 
            else if (opcion == "3") {
              testInyector();
            } 
            else if (opcion == "4") {
              borrarDatos();
            } 
            else {
              Serial.println("Opción inválida. Intente nuevamente.");
            }
            mostrarMenu2();
          }
}