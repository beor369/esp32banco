#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_I2CDevice.h>
#include <ESP32Encoder.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <EEPROM.h>
#include "images/imagenes.h"


// Pines del encoder
#define ENCODER_PIN_A 42
#define ENCODER_PIN_B 41

// Pines de comunicación UART con ESP32-CAM
#define UART_CAM_RX 16
#define UART_CAM_TX 17

// Pin del botón del encoder
const int botonPin = 40;  // SW al pin 1
#define DEBOUNCE_DELAY 10 // Tiempo de debounce en milisegundos
unsigned long lastDebounceTime = 0;
int lastButtonState = HIGH;	 // Estado anterior del botón (asumimos que no está presionado)
int buttonState;			 // Estado actual del botón
const int EEPROM_SIZE = 512; // Tamaño de la EEPROM
// Inicialización de la pantalla con los pines SDA (21) y SCL (20)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/20, /* data=*/21);
// Instancia del encoder
ESP32Encoder encoder;

String texto_seleccionado; // Variable temporal para almacenar lo que se va seleccionando

// Variables del teclado virtual
int filaActual = 0;			   // Índice de la fila actual
int columnaActual = 0;		   // Índice de la columna actual
String textoSeleccionado = ""; // Texto seleccionado por el usuario
bool botonPresionado = false;  // Estado del botón del encoder

// variable clave

String ssid = "";
String clave = "";
// Tamaño de la matriz
const int numFilas = 4;
const int numColumnas[4] = {12, 12, 10, 5}; // Número de columnas en cada fila

// Configuración del encoder
ESP32Encoder encoderTeclado;
int ultimoValorEncoder = 0; // Para el ajuste de sensibilidad

const int BASE_ADDRESS = 0;										  // Dirección base en la EEPROM para almacenar las resistencias de los inyectores base
const int CLICK_FREQUENCY_ADDRESS = BASE_ADDRESS + sizeof(float); // Dirección en la EEPROM para almacenar la frecuencia del clic

volatile uint32_t pulseCount = 0; // Contador de pulsos
float calibrationFactor = 4.5;	  // Ajusta según tu sensor
float flowRate = 0;
float flowMillilitres = 0;
unsigned long oldTime = 0;
// Configuración del Encoder
const unsigned long encoderMoveDelay = 200; // Retardo de 200 ms para el filtro de debouncing
unsigned long lastEncoderMoveTime = 0;		// Tiempo del último movimiento del encoder

// Definición de estados
enum Estado
{
	MENU_SELECCION_INYECTOR,
	MENU_PRINCIPAL,
	MENU_RESISTENCIA,
	MENU_PRUEBA,
	SUBMENU_MANUAL,
	SUBMENU_AUTOMATICO,
	SUBMENU_MID_RES,
	SUBMENU_FLUJO,
	SUBMENU_FUGAS,
	SUBMENU_CLICK,
	SUBMENU_ABANICO,
	SUBMENU_RESULTADOS,
	SUBMENU_FUNCIONAMIENTO,
	AYUDA,
	ATRAS,
	DEVOLVER,
	AGREGAR_INJECTOR,
	MENU_CARACTERISTICAS,
	ATRASITO,
	SELECCIONAR_MOTO,
	SIGUIENTE,
	MENU_CONFIRMA_MOTO,
	AGREGAR_BORRAR_INJ,
	AGREGAR,
	BORRAR,
	TECLADO_VIRTUAL
};
// Variables de estado
Estado estadoActual = MENU_PRINCIPAL;

int indiceMenu = 0;
int indiceInyector = 0;
int indiceSubMenu = 0;
int indiceSubSubMenu = 0;
int indicefuncionamiento = 0;
int indiceseleccionarmoto = 0;
int indiceMotoSeleccionada = 0;
int indiceagregarborrarinjt = 0;

// Espacios para las imágenes



bool resultadoss[5]; // Array que almacena el resultado de las 5 pruebas
const char *nombresPruebas[5] = {"P.MEDIRRES", "P.LLENADO", "P.GOTEO_FUGAS", "P.MEDIR_CLIC", "P.ABANICO"};

// Definir los menús
char nombresMotocicleta[20][50] = {
	"Honda CB 160F", "Yamaha FZ-S FI V3", "Suzuki Gixxer 150",
	"Bajaj Pulsar NS160", "TVS Apache RTR 160 4V", "Honda CB 190R",
	"Yamaha MT-15", "KTM Duke 200", "KTM Duke 250",
	"Benelli TNT 150", "Kawasaki Z125 PRO", "BMW G 310 R"};

const int tamanioTiposInyector = sizeof(nombresMotocicleta) / sizeof(nombresMotocicleta[0]);

const char *selecionarmoto[] = {"siguiente", "caracteristicas", "atras"};
const int tamanioselecionarmoto = sizeof(selecionarmoto) / sizeof(selecionarmoto[0]);
const char *agregarborrarinj[] = {"agregarinj", "borrarinj", "atras"};
const int tamanioagregarborrarinj = sizeof(agregarborrarinj) / sizeof(agregarborrarinj[0]);
const char *menuprincipal[] = {"prueba", "agregar injector", "ayudas"};
const int tamaniomenuprincipal = sizeof(menuprincipal) / sizeof(menuprincipal[0]);

const char *subMenuFuncionamiento[] = {"Manual", "Automatico", "Atras"};
const int tamaniosubMenuFuncionamiento = sizeof(subMenuFuncionamiento) / sizeof(subMenuFuncionamiento[0]);

// const char* subMenuPrueba[] = {"Manual", "Automatico", "Atras"};
// const int tamanioSubMenuPrueba = sizeof(subMenuPrueba) / sizeof(subMenuPrueba[0]);

const char *subMenuManual[] = {"Medir Resistencia", "Flujo", "Fugas", "Click del Inyector", "Prueba de Abanico", "Resultados", "Atras"};
const int tamanioSubMenuManual = sizeof(subMenuManual) / sizeof(subMenuManual[0]);
const int maxItems = 20; // Máximo número de elementos
char numerosParte[20][50] = {
	"FJ7Z-09", "0280156014", "195500-3040",
	"0280158028", "0280156033", "FJ8Z-10",
	"195500-3040", "0280156014", "0280156014",
	"FJ7Z-09", "195500-3040", "0280156014"};

char tiposValvula[20][50] = {
	"Aguja", "Disco", "Bola",
	"Aguja", "Disco", "Aguja",
	"Bola", "Disco", "Disco",
	"Aguja", "Bola", "Disco"};

char impedancias[20][50] = {
	"Alta (12 ohmios)", "Alta (14 ohmios)", "Baja (2.5 ohmios)",
	"Alta (12 ohmios)", "Alta (14 ohmios)", "Alta (12 ohmios)",
	"Baja (2.5 ohmios)", "Alta (14 ohmios)", "Alta (14 ohmios)",
	"Alta (12 ohmios)", "Baja (2.5 ohmios)", "Alta (14 ohmios)"};

char angulosAtomizacion[20][50] = {
	"20°-30°", "15°-25°", "20°-30°",
	"15°-25°", "15°-25°", "20°-30°",
	"20°-30°", "15°-25°", "15°-25°",
	"20°-30°", "20°-30°", "15°-25°"};

char caudalCombustible[20][50] = {
	"180 cc/min", "190 cc/min", "160 cc/min",
	"210 cc/min", "200 cc/min", "190 cc/min",
	"160 cc/min", "220 cc/min", "230 cc/min",
	"180 cc/min", "160 cc/min", "240 cc/min"};

char presionOperacion[20][50] = {
	"3 bar", "3 bar", "3 bar",
	"3 bar", "3 bar", "3 bar",
	"3 bar", "3 bar", "3 bar",
	"3 bar", "3 bar", "3 bar"};

char tiposConector[20][50] = {
	"Keihin", "Bosch", "Mikuni",
	"Bosch", "Bosch", "Keihin",
	"Mikuni", "Bosch", "Bosch",
	"Keihin", "Mikuni", "Bosch"};

String inputData = ""; // Almacena temporalmente los datos ingresados
int currentField = 0;  // Controla el campo actual en el que se ingresan datos
int currentIndex = 0;  // Índice para el nuevo dato en el arreglo
int estadoActuall = 0;
// Instancia del MPU6050
Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp;

// Prototipos de funciones
// String leerValorTeclado();
void cargarDatosDeEEPROM();
String leerDeEEPROM(int index);
void checkInjectorFlow();
// void mostrarDatosGuardados();
float calculateFlowRate(uint32_t pulses);
void showResults();
void detectInjectorClickk();
void requestData(const char *prompt);
void checkInjectorResistance();

void compareResistances(float baseResistance, float testResistance);
void mostrarMenu();
void manejarEstado();
void mensajeBienvenida(const char *mensaje);
void IRAM_ATTR pulseCounter(); // Necesario para las ISR en ESP32
void setupoled();
String tecladoVirtual();
void displayFlowRate();
void setupEncoder();
void displayEncoderPosition();
void mostrarResultado();
void mostrarDatosInyector(int indice);
bool botonTecladoPresionado();

// void mostrarTecladoVirtual();
// double manejarTecladoVirtual();
// void actualizarCursorTecladoVirtual();
// void manejarSeleccionTecladoVirtual(char seleccion);
void seleccionarMotocicleta();
void menuSeleccionMotocicleta();
void mostrarCaracteristicasMotocicleta(int indice);
bool encoderButtonPressed();
void mostrarMotocicletaSeleccionada(int indice);
void mostrarSeleccionMotocicleta(int indice);
void mostrarConfirmarMoto();
void displayField();
void guardarDato(String data);
int getFirstAvailableIndex(char arreglo[][50], int maxItems);
void digitardatoswifi();
void guardarEnEEPROM(char arreglo[][50], int index, String data);
void enviarWifiDatos();
void solicitarYGuardarDatos();

void setup()
{
	u8g2.begin();
	Serial.begin(115200);
	Serial2.begin(115200, SERIAL_8N1, UART_CAM_RX, UART_CAM_TX);
	EEPROM.begin(512); // Ajusta según el tamaño de tu EEPROM
					   // cargarDatosDeEEPROM();// Cargar los datos guardados en los arreglos desde la EEPROM
	// texto_seleccionado = "";

	// currentIndex = getFirstAvailableIndex(nombresMotocicleta, maxItems); // Buscar la primera posición disponible
	//  setupoled();
	setupEncoder();

	mostrarMenu();
}

void loop()
{
	displayEncoderPosition();
	// mostrarDatosGuardados();
}

void digitardatoswifi()
{
	// --- Captura del SSID ---
	u8g2.clearBuffer();
	u8g2.setFont(u8g2_font_ncenB08_tr);
	u8g2.drawStr(0, 12, "Ingrese SSID:");
	u8g2.sendBuffer();
	delay(1000); // Pequeño retardo antes de llamar al teclado
	ssid = tecladoVirtual();
	Serial.print("SSID ingresado: ");
	Serial.println(ssid);

	// --- Captura de la Clave ---
	u8g2.clearBuffer();
	u8g2.setFont(u8g2_font_ncenB08_tr);
	u8g2.drawStr(0, 12, "Ingrese Clave:");
	u8g2.sendBuffer();
	delay(1000);
	//clave = tecladoVirtual();
	Serial.print("Clave ingresada: ");
	Serial.println(clave);
	u8g2.clearBuffer();

	enviarWifiDatos();
}


void enviarWifiDatos()
{
	u8g2.clearBuffer();
	u8g2.setFont(u8g2_font_ncenB08_tr);
	u8g2.drawStr(0, 12, "Enviando datos al ESP32-CAM...");

	Serial.println("Enviando datos al ESP32-CAM...");

	// Enviar SSID y contraseña
	Serial2.print(ssid + "," + clave + "\n");

	// Espera la respuesta del ESP32-CAM
	unsigned long tiempoInicio = millis();
	while (millis() - tiempoInicio < 10000)
	{ // 10 segundos de espera
		if (Serial2.available())
		{
			String respuesta = Serial2.readStringUntil('\n');
			respuesta.trim();

			if (respuesta == "OK")
			{
				u8g2.drawStr(0, 12, "Conexión exitosa a Wi-Fi.");
				Serial.println("Conexión exitosa a Wi-Fi.");
			}
			else
			{
				Serial.println("Error en la conexión Wi-Fi.");
				u8g2.drawStr(0, 12, "Error en la conexión Wi-Fi.");
			}
			return;
		}
	}
	Serial.println("No se recibió respuesta del ESP32-CAM.");
	u8g2.drawStr(0, 12, "No se recibió respuesta del ESP32-CAM.");
	u8g2.sendBuffer();
	delay(1000);
}
// Esta función devuelve true si el botón del encoder fue presionado
bool encoderButtonPressed()
{
	static unsigned long lastPressTime = 0;	  // Para el control de debounce
	const unsigned long debounceDelay = 30;	  // Tiempo de debounce de 50 ms
	bool buttonState = digitalRead(botonPin); // Leer el estado del botón

	if (buttonState == LOW)
	{ // El botón del encoder está presionado (asumiendo que LOW es presionado)
		unsigned long currentTime = millis();
		if (currentTime - lastPressTime > debounceDelay)
		{ // Verificar que ha pasado suficiente tiempo
			lastPressTime = currentTime;
			return true; // Botón presionado y debounce pasado
		}
	}

	return false; // No hay acción del botón
}
void displayEncoderPosition()
{
	int valorEncoder = encoder.getCount();

	if (millis() - lastEncoderMoveTime > encoderMoveDelay)
	{
		if (valorEncoder > 0)
		{
			if (estadoActual == MENU_PRINCIPAL)
			{
				indiceMenu = (indiceMenu + 1) % tamaniomenuprincipal;
			}
			else if (estadoActual == SELECCIONAR_MOTO)
			{
				indiceseleccionarmoto = (indiceseleccionarmoto + 1) % tamanioselecionarmoto;
			}
			else if (estadoActual == AGREGAR_BORRAR_INJ)
			{
				indiceagregarborrarinjt = (indiceagregarborrarinjt + 1) % tamanioagregarborrarinj;
			}
			else if (estadoActual == MENU_SELECCION_INYECTOR)
			{
				indiceInyector = (indiceInyector + 1) % tamanioTiposInyector;
			}
			else if (estadoActual == SUBMENU_FUNCIONAMIENTO)
			{
				indicefuncionamiento = (indicefuncionamiento + 1) % tamaniosubMenuFuncionamiento;
			}
			else if (estadoActual == SUBMENU_MANUAL)
			{
				indiceSubSubMenu = (indiceSubSubMenu + 1) % tamanioSubMenuManual;
			}
			else if (estadoActual == TECLADO_VIRTUAL)
			{
				tecladoVirtual();
			}
			encoder.clearCount();
			mostrarMenu();
			lastEncoderMoveTime = millis();
		}
		else if (valorEncoder < 0)
		{
			if (estadoActual == MENU_PRINCIPAL)
			{
				indiceMenu = (indiceMenu - 1 + tamaniomenuprincipal) % tamaniomenuprincipal;
			}
			else if (estadoActual == SELECCIONAR_MOTO)
			{
				indiceseleccionarmoto = (indiceseleccionarmoto - 1 + tamanioselecionarmoto) % tamanioselecionarmoto;
			}
			else if (estadoActual == MENU_SELECCION_INYECTOR)
			{
				indiceInyector = (indiceInyector - 1 + tamanioTiposInyector) % tamanioTiposInyector;
			}
			else if (estadoActual == AGREGAR_BORRAR_INJ)
			{
				indiceagregarborrarinjt = (indiceagregarborrarinjt - 1 + tamanioagregarborrarinj) % tamanioagregarborrarinj;
			}
			else if (estadoActual == SUBMENU_FUNCIONAMIENTO)
			{
				indicefuncionamiento = (indicefuncionamiento - 1 + tamaniosubMenuFuncionamiento) % tamaniosubMenuFuncionamiento;
			}
			else if (estadoActual == SUBMENU_MANUAL)
			{
				indiceSubSubMenu = (indiceSubSubMenu - 1 + tamanioSubMenuManual) % tamanioSubMenuManual;
			}
			else if (estadoActual == TECLADO_VIRTUAL)
			{
				tecladoVirtual();
			}
			encoder.clearCount();
			mostrarMenu();
			lastEncoderMoveTime = millis();
		}

		bool estadoBoton = digitalRead(botonPin);

		if (estadoBoton == LOW)
		{
			delay(50); // Debounce
			if (estadoBoton == LOW)
			{
				manejarEstado();
				mostrarMenu();
				delay(300); // Debounce
			}
		}
	}
}

void IRAM_ATTR pulseCounter()
{
	pulseCount++;
}

void setupEncoder()
{
	pinMode(ENCODER_PIN_A, INPUT_PULLUP);
	pinMode(ENCODER_PIN_B, INPUT_PULLUP);
	encoder.attachHalfQuad(ENCODER_PIN_A, ENCODER_PIN_B);
	encoderTeclado.attachHalfQuad(ENCODER_PIN_A, ENCODER_PIN_B);
	encoder.clearCount();
	pinMode(botonPin, INPUT_PULLUP);
	// Inicializar encoder
}

void setupoled()
{
	// u8g2.begin();	
}

void dibujarImagen(const char* nombreImagen) {
    u8g2.clearBuffer();
    u8g2.drawXBMP(0, 0, 128, 64, Imagenes::obtenerImagen(nombreImagen));
    u8g2.sendBuffer();
}

void mostrarMenu() {
    switch (estadoActual) {
        case MENU_PRINCIPAL: {
            const char* imagenes[] = {"iniciar_prueba", "agregarborrarinyector", "ayuda"};
            if (indiceMenu >= 0 && indiceMenu < 3) {
                dibujarImagen(imagenes[indiceMenu]);
            }
            break;
        }
        case MENU_SELECCION_INYECTOR:
            mostrarSeleccionMotocicleta(indiceInyector);
            break;
        case MENU_CARACTERISTICAS:
            mostrarCaracteristicasMotocicleta(indiceMotoSeleccionada);
            break;
        case MENU_CONFIRMA_MOTO:
            mostrarSeleccionMotocicleta(indiceMotoSeleccionada);
            break;
        case SUBMENU_FUNCIONAMIENTO: {
            const char* imagenes[] = {"modo_manual", "modo_automatico", "atras"};
            if (indicefuncionamiento >= 0 && indicefuncionamiento < 3) {
                dibujarImagen(imagenes[indicefuncionamiento]);
            }
            break;
        }
        case SUBMENU_MANUAL: {
            const char* imagenes[] = {
                "prueba_medir_resistencia", "prueba_de_llenado", "prueba_de_goteo", "prueba_medir_clic",
                "prueba_abanico", "resultados", "devolver"
            };
            if (indiceSubSubMenu >= 0 && indiceSubSubMenu < 7) {
                dibujarImagen(imagenes[indiceSubSubMenu]);
            }
            break;
        }
        case AGREGAR_BORRAR_INJ: {
            const char* imagenes[] = {"AGREGAR_INJ", "BORRAR_INJ", "ATRASAGREGARINYECTOR_INJ"};
            if (indiceagregarborrarinjt >= 0 && indiceagregarborrarinjt < 3) {
                dibujarImagen(imagenes[indiceagregarborrarinjt]);
            }
            break;
        }
        case SELECCIONAR_MOTO: {
            const char* imagenes[] = {"SIGUIENTEFUNCIONAMIENTO", "VER_CARACT", "ATRASMENU"};
            if (indiceseleccionarmoto >= 0 && indiceseleccionarmoto < 3) {
                dibujarImagen(imagenes[indiceseleccionarmoto]);
            }
            break;
        }
    }
}

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
			estadoActual = AGREGAR;
			// estadoActual == 0;
		}
		else if (indiceagregarborrarinjt == 1)
		{
			estadoActual = BORRAR;
		}
		else if (indiceagregarborrarinjt == 2)
		{
			estadoActual = MENU_PRINCIPAL;
		}

		break;
	case AGREGAR:
		// displayField();
		tecladoVirtual();
		break;

	case BORRAR:

		break;

	case AYUDA:

		digitardatoswifi();

		break;

	case MENU_SELECCION_INYECTOR:

		// if (encoderButtonPressed()) { // Si el botón del encoder es presionado
		// Aquí se actualiza la selección de la motocicleta
		indiceMotoSeleccionada = indiceInyector;
		estadoActual = SELECCIONAR_MOTO; // Ir a confirmar selección de motocicleta
		// }
		break;

	case SELECCIONAR_MOTO:
		if (indiceseleccionarmoto == 0)
		{
			estadoActual = SUBMENU_FUNCIONAMIENTO;
			// estadoActual == 0;
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

	case MENU_CONFIRMA_MOTO:

		if (encoderButtonPressed())
		{
			estadoActual = SELECCIONAR_MOTO; // Confirmar selección y pasar al siguiente menú
		}
		break;

		// case MENU_PRUEBA:
		//     // Lógica del menú de prueba
		//     if (encoderButtonPressed()) {
		//         estadoActual = MENU_CARACTERISTICAS; // Ir a mostrar las características
		//     }
		//     break;

	case MENU_CARACTERISTICAS:

		if (encoderButtonPressed())
		{
			estadoActual = SELECCIONAR_MOTO; // Volver al menú anterior
		}
		break;

	// case MENU_RESISTENCIA:
	//     checkInjectorResistance();
	//     break;
	// case MENU_PRUEBA:
	//     if (indiceSubMenu == 0) {
	//         estadoActual = SUBMENU_MANUAL;
	//         indiceSubSubMenu = 0;
	//     } else if (indiceSubMenu == 1) {
	//         estadoActual = SUBMENU_AUTOMATICO;
	//     } else if (indiceSubMenu == 2) {
	//         estadoActual = MENU_SELECCION_INYECTOR;
	//         indiceSubMenu = 0;
	//     }
	//     break;
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
			checkInjectorResistance();
		}
		else if (indiceSubSubMenu == 1)
		{
			displayFlowRate();
		}
		else if (indiceSubSubMenu == 2)
		{
			mostrarResultado();
		}
		else if (indiceSubSubMenu == 3)
		{
			detectInjectorClickk();
		}
		else if (indiceSubSubMenu == 4)
		{

		}
		else if (indiceSubSubMenu == 5)
		{
			showResults();
		}
		else if (indiceSubSubMenu == 6)
		{
			estadoActual = SUBMENU_FUNCIONAMIENTO;
		}
		break;
	case SUBMENU_AUTOMATICO:
		// Lógica para realizar las pruebas automáticas
		checkInjectorResistance();
		displayFlowRate();
		mostrarResultado();
		detectInjectorClickk();

		showResults();
		estadoActual = SUBMENU_FUNCIONAMIENTO;
		break;
	case ATRAS:
		estadoActual = SELECCIONAR_MOTO;
		break;

	case DEVOLVER:
		estadoActual = SUBMENU_FUNCIONAMIENTO;
		indicefuncionamiento = 0;
		break;

	default:
		break;
	}
}

// Función del teclado virtual que devuelve el texto seleccionado
String tecladoVirtual()
{
	const char teclado[3][4][11] = {
		{ // Modo 0: Mayúsculas
			{'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K'},
			{'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V'},
			{'W', 'X', 'Y', 'Z', '1', '2', '3', '4', '5', '6', '7'},
			{'8', '9', '0', '.', '-', '<', '>', '*', ' ', ' ', ' '}},
		{ // Modo 1: Minúsculas
			{'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k'},
			{'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v'},
			{'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7'},
			{'8', '9', '0', '.', '-', '<', '>', '*', ' ', ' ', ' '}},
		{ // Modo 2: Caracteres especiales
			{'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_'},
			{'+', '=', '<', '>', '?', '/', '[', ']', '{', '}', '|'},
			{'~', '`', ':', ';', '"', '\'', '\\', ' ', ' ', ' ', ' '},
			{' ', ' ', ' ', ' ', ' ', '<', '>', '*', ' ', ' ', ' '}}
	};

	String textoSeleccionado = "";
	int fila = 0, columna = 0;
	int lastEncoderPosition = encoder.getCount();
	bool seleccionando = true;
	int modo = 0; // 0 = Mayúsculas, 1 = Minúsculas, 2 = Caracteres especiales

	while (seleccionando)
	{
		int encoderPosition = encoder.getCount();
		u8g2.clearBuffer();
		u8g2.setFont(u8g2_font_ncenB08_tr);

		// Controlar el movimiento del cursor con el encoder
		if (encoderPosition != lastEncoderPosition)
		{
			if (encoderPosition > lastEncoderPosition)
			{
				columna++;
				if (columna > 10)
				{
					columna = 0;
					fila++;
					if (fila > 3)
					{
						fila = 0;
					}
				}
			}
			else
			{
				columna--;
				if (columna < 0)
				{
					columna = 10;
					fila--;
					if (fila < 0)
					{
						fila = 3;
					}
				}
			}
			lastEncoderPosition = encoderPosition;
		}

		// Detectar la presión del botón para seleccionar una letra/número o símbolo
		if (digitalRead(botonPin) == LOW)
		{
			char seleccion = teclado[modo][fila][columna];
			if (seleccion == '-')
			{
				// Borrar la última letra
				if (textoSeleccionado.length() > 0)
				{
					textoSeleccionado.remove(textoSeleccionado.length() - 1);
				}
			}
			else if (seleccion == '>')
			{
				// Si selecciona ">", terminar la selección y devolver el texto
				seleccionando = false;
			}
			else if (seleccion == '<')
			{
				return ""; // Devolver un valor vacío o lo que prefieras.
			}
			else if (seleccion == '*')
			{
				// Cambiar de modo (mayúsculas, minúsculas, especiales)
				modo = (modo + 1) % 3;
			}
			else
			{
				// Agregar la letra o número seleccionado
				textoSeleccionado += seleccion;
			}
			delay(200); // Debounce para evitar múltiples selecciones
		}

		// Mostrar el teclado en pantalla
		u8g2.clearBuffer();
		u8g2.setFont(u8g2_font_ncenB08_tr);

		// Mostrar las letras/números del teclado con el cursor encima del número seleccionado
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 11; j++)
			{
				int x = j * 11;		  // Espaciado horizontal
				int y = (i + 1) * 12; // Espaciado vertical

				// Dibujar el cursor
				if (i == fila && j == columna)
				{
					u8g2.drawStr(x, y, "[");
					u8g2.drawStr(x + 9, y, "]");
					u8g2.drawStr(x + 3, y, String(teclado[modo][i][j]).c_str());
				}
				else
				{
					u8g2.drawStr(x, y, String(teclado[modo][i][j]).c_str());
				}
			}
		}

		// Mostrar el texto seleccionado hasta el momento
		u8g2.drawStr(0, 60, textoSeleccionado.c_str());

		// Enviar los datos a la pantalla OLED
		u8g2.sendBuffer();
	}
	return textoSeleccionado; // Devolver el texto cuando selecciona ">"
}



void showResults()
{
	bool exit = false;
	while (!exit)
	{
		u8g2.clearBuffer();
		u8g2.setFont(u8g2_font_5x8_tr); // Fuente compacta para más información

		// Encabezados
		u8g2.drawStr(0, 8, "Resultados finales:");
		u8g2.drawStr(0, 16, "Prueba   Estado");

		// Resultados de cada prueba
		for (int i = 0; i < 5; i++)
		{
			char linea[20];
			// Verifica si el resultado fue "Bien" (true) o "Mal" (false)
			sprintf(linea, "%s   %s", nombresPruebas[i], (Imagenes::obtenerImagen("resultados")[i]) ? "Bien" : "Mal");
			u8g2.drawStr(0, 24 + (i * 8), linea);
		}

		// Calcular puntaje total
		int puntajeTotal = 0;
		for (int i = 0; i < 5; i++)
		{
			puntajeTotal += Imagenes::obtenerImagen("resultados")[i] ? 1 : 0;
		}

		// Mostrar estado general del inyector
		const char *estadoInyector;
		if (puntajeTotal == 5)
		{
			estadoInyector = "Perfecto";
		}
		else if (puntajeTotal >= 3)
		{
			estadoInyector = "Regular";
		}
		else
		{
			estadoInyector = "Reparar";
		}

		char lineaFinal[20];
		sprintf(lineaFinal, "Estado: %s", estadoInyector);
		u8g2.drawStr(0, 64 - 8, lineaFinal);

		// Mostrar en la pantalla OLED
		u8g2.sendBuffer();
		// Enviar el contenido al display
		u8g2.sendBuffer();

		delay(1000); // Pequeño retardo para evitar rebotes y actualizar con frecuencia

		// Manejar el botón para salir
		if (digitalRead(botonPin) == LOW)
		{
			delay(50); // Debounce
			if (digitalRead(botonPin) == LOW)
			{
				while (digitalRead(botonPin) == LOW)
				{
					delay(10); // Espera a que se suelte el botón
				}
				exit = true;
				estadoActual = SUBMENU_MANUAL;
			}
		}
	}
}

void solicitarYGuardarDatos()
{
	// Obtener el siguiente índice disponible en el arreglo
	currentIndex = getFirstAvailableIndex(nombresMotocicleta, 20); // Ajustar según el tamaño máximo
	if (currentIndex == -1)
	{
		Serial.println("No hay espacio disponible para agregar más datos.");
		return;
	}
	String datos[7]; // Arreglo temporal para almacenar los 7 datos ingresados por el usuario

	// Mensajes para cada dato que se solicitará
	const char *mensajes[] = {
		"Ingresa el nombre de la moto:",
		"Ingresa la impedancia:",
		"Ingresa el tipo de valvula:",
		"Ingresa el caudal de combustible:",
		"Ingresa la presion de operacion:",
		"Ingresa el tipo de conector:",
		"Ingresa el angulo de atomizacion:"};

	// Recorrer y solicitar los 7 datos
	for (int i = 0; i < 7; i++)
	{
		bool datoValido = false;
		while (!datoValido)
		{
			u8g2.clearBuffer();
			u8g2.setFont(u8g2_font_ncenB08_tr);
			u8g2.drawStr(0, 12, mensajes[i]); // Mostrar el mensaje en la pantalla
			u8g2.sendBuffer();
			delay(5000); // Puedes ajustar el tiempo de espera

			String input = tecladoVirtual(); // Usar el teclado virtual para obtener el dato

			if (input != "")
			{
				datos[i] = input; // Almacenar el dato ingresado en el arreglo
				datoValido = true;
			}
			else
			{
				u8g2.clearBuffer();
				u8g2.setFont(u8g2_font_ncenB08_tr);
				u8g2.drawStr(0, 12, "Dato invalido, intenta nuevamente.");
				u8g2.sendBuffer();
				delay(1000);
			}
		}
	}

	// Ahora guardar los datos en los arreglos y EEPROM
	for (int j = 0; j < 7; j++)
	{
		currentField = j;	   // Actualizar el campo actual para cada dato
		guardarDato(datos[j]); // Llamar a la función guardarDato que ya tienes
	}

	// Confirmación de guardado en la pantalla
	u8g2.clearBuffer();
	u8g2.setFont(u8g2_font_ncenB08_tr);
	u8g2.drawStr(0, 12, "Datos guardados exitosamente!");
	u8g2.sendBuffer();
	delay(2000); // Pausar para que el usuario vea la confirmación
}

void guardarDato(String data)
{
	switch (currentField)
	{
	case 0:
		// Asegúrate de que nombresMotocicleta sea un arreglo de tipo char[20][50] (por ejemplo)
		strncpy(nombresMotocicleta[currentIndex], data.c_str(), sizeof(nombresMotocicleta[currentIndex]) - 1);
		nombresMotocicleta[currentIndex][sizeof(nombresMotocicleta[currentIndex]) - 1] = '\0'; // Asegura el terminador nulo
		guardarEnEEPROM(nombresMotocicleta, currentIndex, data);
		break;
	case 1:
		strncpy(impedancias[currentIndex], data.c_str(), sizeof(impedancias[currentIndex]) - 1);
		impedancias[currentIndex][sizeof(impedancias[currentIndex]) - 1] = '\0';
		guardarEnEEPROM(impedancias, currentIndex, data);
		break;
	case 2:
		strncpy(tiposValvula[currentIndex], data.c_str(), sizeof(tiposValvula[currentIndex]) - 1);
		tiposValvula[currentIndex][sizeof(tiposValvula[currentIndex]) - 1] = '\0';
		guardarEnEEPROM(tiposValvula, currentIndex, data);
		break;
	case 3:
		strncpy(caudalCombustible[currentIndex], data.c_str(), sizeof(caudalCombustible[currentIndex]) - 1);
		caudalCombustible[currentIndex][sizeof(caudalCombustible[currentIndex]) - 1] = '\0';
		guardarEnEEPROM(caudalCombustible, currentIndex, data);
		break;
	case 4:
		strncpy(presionOperacion[currentIndex], data.c_str(), sizeof(presionOperacion[currentIndex]) - 1);
		presionOperacion[currentIndex][sizeof(presionOperacion[currentIndex]) - 1] = '\0';
		guardarEnEEPROM(presionOperacion, currentIndex, data);
		break;
	case 5:
		strncpy(tiposConector[currentIndex], data.c_str(), sizeof(tiposConector[currentIndex]) - 1);
		tiposConector[currentIndex][sizeof(tiposConector[currentIndex]) - 1] = '\0';
		guardarEnEEPROM(tiposConector, currentIndex, data);
		break;
	case 6:
		strncpy(angulosAtomizacion[currentIndex], data.c_str(), sizeof(angulosAtomizacion[currentIndex]) - 1);
		angulosAtomizacion[currentIndex][sizeof(angulosAtomizacion[currentIndex]) - 1] = '\0';
		guardarEnEEPROM(angulosAtomizacion, currentIndex, data);
		break;
		// Agregar más casos si es necesario.
	}
}

int getFirstAvailableIndex(char arreglo[][50], int maxItems)
{
	for (int i = 0; i < maxItems; i++)
	{
		if (arreglo[i][0] == '\0')
		{
			return i;
		}
	}
	return -1;
}

void guardarEnEEPROM(char arreglo[][50], int index, String data)
{
	int address = index * 51;			  // Espacio adecuado para cada entrada
	EEPROM.write(address, data.length()); // Guarda la longitud
	for (int i = 0; i < data.length(); i++)
	{
		EEPROM.write(address + 1 + i, data[i]); // Guarda el dato
	}
	EEPROM.write(address + 1 + data.length(), '\0'); // Terminador nulo
}

String leerDeEEPROM(int index)
{
	int address = index * 50;
	int length = EEPROM.read(address); // Leer la longitud del dato

	char buffer[51]; // 50 caracteres + 1 para el terminador nulo
	for (int i = 0; i < length; i++)
	{
		buffer[i] = EEPROM.read(address + 1 + i);
	}
	buffer[length] = '\0'; // Añadir terminador nulo al final

	return String(buffer); // Devolver la cadena leída
}

void cargarDatosDeEEPROM()
{
	for (int i = 0; i < 20; i++)
	{										  // Ajustar según el tamaño máximo
		String nombre = "Prueba" + String(i); // Simulación de diferentes valores

		Serial.print("Nombre leído: ");
		Serial.println(nombre);

		if (nombre.length() > 0 && nombre.length() < sizeof(nombresMotocicleta[i]))
		{
			// Copiar el nombre al arreglo en la posición correspondiente
			strncpy(nombresMotocicleta[i], nombre.c_str(), sizeof(nombresMotocicleta[i]) - 1);
			nombresMotocicleta[i][sizeof(nombresMotocicleta[i]) - 1] = '\0';

			Serial.print("Nombre guardado en la posición ");
			Serial.print(i);
			Serial.print(": ");
			Serial.println(nombresMotocicleta[i]);
		}
	}
}

void mostrarDatosGuardados()
{
	Serial.println("Datos guardados en EEPROM:");

	for (int i = 0; i < 20; i++)
	{ // Ajustar según el tamaño máximo
		if (nombresMotocicleta[i][0] != '\0')
		{ // Comprobar si no está vacío
			Serial.print("Moto: ");
			Serial.println(nombresMotocicleta[i]);
		}
		if (impedancias[i][0] != '\0')
		{
			Serial.print("Impedancia: ");
			Serial.println(impedancias[i]);
		}
		if (tiposValvula[i][0] != '\0')
		{
			Serial.print("Tipo de Válvula: ");
			Serial.println(tiposValvula[i]);
		}
		if (caudalCombustible[i][0] != '\0')
		{
			Serial.print("Caudal de Combustible: ");
			Serial.println(caudalCombustible[i]);
		}
		if (presionOperacion[i][0] != '\0')
		{
			Serial.print("Presión de Operación: ");
			Serial.println(presionOperacion[i]);
		}
		if (tiposConector[i][0] != '\0')
		{
			Serial.print("Tipo de Conector: ");
			Serial.println(tiposConector[i]);
		}
		if (angulosAtomizacion[i][0] != '\0')
		{
			Serial.print("Ángulo de Atomización: ");
			Serial.println(angulosAtomizacion[i]);
		}

		Serial.println("-------------");
	}
}

void displayField()
{
	// // 		Serial.println("dispalyField: " + String(currentField));
	// // u8g2.clearBuffer();

	// switch (currentField) {
	// 	// Serial.println("dispalyField: " + String(currentField));
	//     case 0:
	//         u8g2.setCursor(0, 10);
	//         u8g2.print("Ingrese Nombre Motocicleta:");
	//         break;
	//     case 1:
	//         u8g2.setCursor(0, 10);
	//         u8g2.print("Ingrese la impedancia:");
	//         break;
	//     case 2:
	//         u8g2.setCursor(0, 10);
	//         u8g2.print("Ingrese Tipo de Valvula:");
	//         break;

	//     case 3:
	//         u8g2.setCursor(0, 10);
	//         u8g2.print("Ingrese caudal de combustible:");
	//         break;
	// 	case 4:
	//         u8g2.setCursor(0, 10);
	//         u8g2.print("Ingrese presion de operacion:");
	//         break;
	// 	case 5:
	//         u8g2.setCursor(0, 10);
	//         u8g2.print("Ingrese tipo de conector:");
	//         break;
	// 	case 6:
	//         u8g2.setCursor(0, 10);
	//         u8g2.print("Ingrese angulo de atomizacion:");
	//         break;
	//  default:
	//         u8g2.setCursor(0, 10);
	//         u8g2.print("Campo no definido.");
	//         break;

	// }

	// // Manejo del teclado virtual
	// char keyboard[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_. ";
	// static int selectedIndex = 0;

	// int position = encoder.getCount();
	// if (position != selectedIndex) {
	//     selectedIndex = position;
	//     encoder.clearCount();
	// }

	// u8g2.setCursor(0, 30);
	// u8g2.print("Seleccionado: ");
	// u8g2.print(keyboard[selectedIndex]);

	// // Agregar carácter seleccionado al inputData
	// if (digitalRead(botonPin) == LOW) { // Presionar encoder
	//     inputData += keyboard[selectedIndex];
	// }

	// // Botón de borrar
	// if (digitalRead(botonPin) == LOW && inputData.length() > 0) {
	//     inputData.remove(inputData.length() - 1); // Borra el último carácter
	// }

	// // Muestra la cadena temporal ingresada
	// u8g2.setCursor(0, 50);
	// u8g2.print("Dato: ");
	// u8g2.print(inputData);

	// // Botón de aceptar
	// if (digitalRead(botonPin) == LOW) {
	//     guardarDato(inputData);
	//     inputData = "";  // Reinicia el input para el siguiente campo
	//     currentField++;  // Pasa al siguiente campo
	// }

	// u8g2.sendBuffer(); // Muestra todo en pantalla
}
// Función para mostrar las motocicletas mientras se navega con el encoder
void mostrarSeleccionMotocicleta(int indice)
{
	u8g2.clearBuffer();
	u8g2.setFont(u8g2_font_ncenB08_tr); // Ajusta la fuente para la pantalla
	u8g2.drawStr(0, 20, "Selecciona Moto:");
	u8g2.drawStr(0, 40, nombresMotocicleta[indice]); // Mostrar el nombre de la motocicleta seleccionada por el encoder
	u8g2.sendBuffer();
}

// Función para mostrar la motocicleta seleccionada
void mostrarMotocicletaSeleccionada(int indice)
{
	u8g2.clearBuffer();
	u8g2.setFont(u8g2_font_ncenB08_tr); // Ajusta la fuente para la pantalla
	u8g2.drawStr(0, 20, "Moto seleccionada:");
	u8g2.drawStr(0, 40, nombresMotocicleta[indice]); // Mostrar el nombre de la motocicleta seleccionada
	u8g2.sendBuffer();
}

// Función para mostrar las características de la motocicleta seleccionada
void mostrarCaracteristicasMotocicleta(int indice)
{
	u8g2.clearBuffer();
	u8g2.setFont(u8g2_font_ncenB08_tr);				 // Ajusta la fuente para la pantalla
	u8g2.drawStr(0, 10, nombresMotocicleta[indice]); // Mostrar el nombre
	u8g2.drawStr(0, 20, numerosParte[indice]);		 // Mostrar el número de parte
	u8g2.drawStr(0, 30, tiposValvula[indice]);		 // Mostrar el tipo de válvula
	u8g2.drawStr(0, 40, angulosAtomizacion[indice]); // Mostrar el ángulo de atomización
	u8g2.drawStr(0, 50, caudalCombustible[indice]);	 // Mostrar el caudal de combustible
	u8g2.drawStr(0, 60, presionOperacion[indice]);	 // Mostrar la presión de operación
	u8g2.sendBuffer();
}

void checkInjectorResistance()
{

	int resistencia = analogRead(34); // Ejemplo con el pin A2
	if (resistencia > 200 && resistencia < 500)
	{
		resultadoss[1] = true; // Prueba bien
	}
	else
	{
		resultadoss[1] = false; // Prueba mal
	}
	u8g2.sendBuffer();
}


void detectInjectorClickk()
{

	int CLIC = analogRead(34); // Ejemplo con el pin A2
	if (CLIC > 200 && CLIC < 500)
	{
		resultadoss[4] = true; // Prueba bien
	}
	else
	{
		resultadoss[4] = false; // Prueba mal
	}

}

void pruebaDeAbanico()
{
	int ABANICO = analogRead(34); // Ejemplo con el pin A2
	if (ABANICO > 200 && ABANICO < 500)
	{
		resultadoss[5] = true; // Prueba bien
	}
	else
	{
		resultadoss[5] = false; // Prueba mal
	}
	// float anguloX, anguloY, anguloZ;
	// bool exit = false;

	// while (!exit) {
	//     leerAngulosMPU(anguloX, anguloY, anguloZ);

	//     lcd.clear();
	//     lcd.setCursor(0, 0);
	//     lcd.print("Angulo X:");
	//     lcd.print(anguloX);
	//     lcd.setCursor(0, 1);
	//     lcd.print("Angulo Y:");
	//     lcd.print(anguloY);

	//     delay(1000);

	//     // Comparar con el rango esperado de ángulo de atomización para la motocicleta seleccionada
	//     float anguloEsperadoInicio = (angulosAtomizacion[indiceMenu][0] - '0') * 10 + (angulosAtomizacion[indiceMenu][1] - '0');
	//     float anguloEsperadoFin = (angulosAtomizacion[indiceMenu][3] - '0') * 10 + (angulosAtomizacion[indiceMenu][4] - '0');

	//     if (anguloX >= anguloEsperadoInicio && anguloX <= anguloEsperadoFin) {
	//         lcd.clear();
	//         lcd.setCursor(0, 0);
	//         lcd.print("Angulo OK");
	//     } else {
	//         lcd.clear();
	//         lcd.setCursor(0, 0);
	//         lcd.print("Angulo fuera de rango");
	//     }

	//     delay(2000);

	//     if (digitalRead(botonPin) == LOW) {
	//         delay(50); // Debounce
	//         if (digitalRead(botonPin) == LOW) {
	//             while (digitalRead(botonPin) == LOW) {
	//                 delay(10); // Espera a que se suelte el botón
	//             }
	//             exit = true;
	//             estadoActual = SUBMENU_MANUAL;
	//         }
	//     }
	// }
}

void displayFlowRate()
{
	int CAUDAL = analogRead(34); // Ejemplo con el pin A2
	if (CAUDAL > 200 && CAUDAL < 500)
	{
		resultadoss[2] = true; // Prueba bien
	}
	else
	{
		resultadoss[2] = false; // Prueba mal
	}
	// bool exit = false;
	// while (!exit) {
	//     if ((millis() - oldTime) > 1000) {
	//         detachInterrupt(digitalPinToInterrupt(4));
	//         flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
	//         oldTime = millis();
	//         flowMillilitres = (flowRate / 60) * 1000;
	//         pulseCount = 0;
	//         attachInterrupt(digitalPinToInterrupt(4), pulseCounter, FALLING);
	//     }
	//     lcd.clear();
	//     lcd.setCursor(0, 0);
	//     lcd.print("Flujo: ");
	//     lcd.setCursor(0, 1);
	//     lcd.print(flowRate);
	//     lcd.print(" L/min");
	//     delay(1000); // Pequeño retardo para evitar rebotes y actualizar con frecuencia

	//     if (digitalRead(botonPin) == LOW) {
	//         delay(50); // Debounce
	//         if (digitalRead(botonPin) == LOW) {
	//             while (digitalRead(botonPin) == LOW) {
	//                 delay(10); // Espera a que se suelte el botón
	//             }
	//             exit = true;
	//             estadoActual = SUBMENU_MANUAL;
	//         }
	//     }
	// }
}

void mostrarResultado()
{
	// bool exit = false;
	// while (!exit) {
	//     lcd.clear();
	//     lcd.setCursor(0, 0);
	//     lcd.print("Flujo (mL/s): ");
	//     lcd.setCursor(0, 1);
	//     lcd.print(flowRate);

	//     if (flowRate > 0) {
	//         lcd.setCursor(0, 2);
	//         lcd.print("Fuga detectada!");
	//     } else {
	//         lcd.setCursor(0, 2);
	//         lcd.print("No hay fuga");
	//     }
	//     delay(1000); // Pequeño retardo para evitar rebotes y actualizar con frecuencia

	//     if (digitalRead(botonPin) == LOW) {
	//         delay(50); // Debounce
	//         if (digitalRead(botonPin) == LOW) {
	//             while (digitalRead(botonPin) == LOW) {
	//                 delay(5); // Espera a que se suelte el botón
	//             }
	//             exit = true;
	//             estadoActual = SUBMENU_MANUAL;
	//         }
	//     }
	// }
}

void mostrarDatosInyector(int indice)
{
	// lcd.clear();
	// lcd.setCursor(0, 0);
	// lcd.print("Datos del Inyector:");

	// lcd.setCursor(0, 1);
	// lcd.print("Num Parte:");
	// lcd.print(numerosParte[indice]);

	// delay(2000);

	// lcd.clear();
	// lcd.setCursor(0, 0);
	// lcd.print("Tipo Valvula:");
	// lcd.print(tiposValvula[indice]);

	// delay(2000);

	// lcd.clear();
	// lcd.setCursor(0, 0);
	// lcd.print("Impedancia:");
	// lcd.print(impedancias[indice]);

	// delay(2000);

	// lcd.clear();
	// lcd.setCursor(0, 0);
	// lcd.print("Angulo:");
	// lcd.print(angulosAtomizacion[indice]);

	// delay(2000);

	// lcd.clear();
	// lcd.setCursor(0, 0);
	// lcd.print("Caudal:");
	// lcd.print(caudalCombustible[indice]);

	// delay(2000);

	// lcd.clear();
	// lcd.setCursor(0, 0);
	// lcd.print("Presion:");
	// lcd.print(presionOperacion[indice]);

	// delay(2000);

	// lcd.clear();
	// lcd.setCursor(0, 0);
	// lcd.print("Conector:");
	// lcd.print(tiposConector[indice]);

	// delay(2000);
}