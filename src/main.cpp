#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_I2CDevice.h>
#include <ESP32Encoder.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <EEPROM.h>
#include "images/imagenes.h"
#include <nvs_flash.h>
#include <nvs.h>

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

const char *subMenuManual[] = {"Medir Resistencia", "Flujo", "Fugas", "Click del Inyector", "Prueba de Abanico", "Resultados", "Atras"};
const int tamanioSubMenuManual = sizeof(subMenuManual) / sizeof(subMenuManual[0]);
void mostrarMenu();
void manejarEstado();
void setupEncoder();
// void checkInjectorResistance();
//void displayFlowRate();
void detectInjectorClickk();

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

#define NAMESPACE "injectors"


// ------------------------------
//     Uso de la Clase
// ------------------------------



// void mostrarTecladoVirtual();
// double manejarTecladoVirtual();
// void actualizarCursorTecladoVirtual();
// void manejarSeleccionTecladoVirtual(char seleccion);

String tecladoVirtual()
{
	const char teclado[3][4][11] = {
		{// Modo 0: Mayúsculas
		 {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K'},
		 {'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V'},
		 {'W', 'X', 'Y', 'Z', '1', '2', '3', '4', '5', '6', '7'},
		 {'8', '9', '0', '.', '-', '<', '>', '*', ' ', ' ', ' '}},
		{// Modo 1: Minúsculas
		 {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k'},
		 {'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v'},
		 {'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7'},
		 {'8', '9', '0', '.', '-', '<', '>', '*', ' ', ' ', ' '}},
		{// Modo 2: Caracteres especiales
		 {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_'},
		 {'+', '=', '<', '>', '?', '/', '[', ']', '{', '}', '|'},
		 {'~', '`', ':', ';', '"', '\'', '\\', ' ', ' ', ' ', ' '},
		 {' ', ' ', ' ', ' ', ' ', '<', '>', '*', ' ', ' ', ' '}}};

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

void loop()
{
	displayEncoderPosition();
	// mostrarDatosGuardados();
}

void dibujarImagen(const char *nombreImagen)
{
	u8g2.clearBuffer();
	u8g2.drawXBMP(0, 0, 128, 64, Imagenes::obtenerImagen(nombreImagen));
	u8g2.sendBuffer();
}

void mostrarMenu()
{
	switch (estadoActual)
	{
	case MENU_PRINCIPAL:
	{
		const char *imagenes[] = {"iniciar_prueba", "agregarborrarinyector", "ayuda"};
		if (indiceMenu >= 0 && indiceMenu < 3)
		{
			dibujarImagen(imagenes[indiceMenu]);
		}
		break;
	}
	case MENU_SELECCION_INYECTOR:
		break;
	case MENU_CARACTERISTICAS:
		break;
	case MENU_CONFIRMA_MOTO:

		break;
	case SUBMENU_FUNCIONAMIENTO:
	{
		const char *imagenes[] = {"modo_manual", "modo_automatico", "atras"};
		if (indicefuncionamiento >= 0 && indicefuncionamiento < 3)
		{
			dibujarImagen(imagenes[indicefuncionamiento]);
		}
		break;
	}
	case SUBMENU_MANUAL:
	{
		const char *imagenes[] = {
			"prueba_medir_resistencia", "prueba_de_llenado", "prueba_de_goteo", "prueba_medir_clic",
			"prueba_abanico", "resultados", "devolver"};
		if (indiceSubSubMenu >= 0 && indiceSubSubMenu < 7)
		{
			dibujarImagen(imagenes[indiceSubSubMenu]);
		}
		break;
	}
	case AGREGAR_BORRAR_INJ:
	{
		const char *imagenes[] = {"AGREGAR_INJ", "BORRAR_INJ", "ATRASAGREGARINYECTOR_INJ"};
		if (indiceagregarborrarinjt >= 0 && indiceagregarborrarinjt < 3)
		{
			dibujarImagen(imagenes[indiceagregarborrarinjt]);
		}
		break;
	}
	case SELECCIONAR_MOTO:
	{
		const char *imagenes[] = {"SIGUIENTEFUNCIONAMIENTO", "VER_CARACT", "ATRASMENU"};
		if (indiceseleccionarmoto >= 0 && indiceseleccionarmoto < 3)
		{
			dibujarImagen(imagenes[indiceseleccionarmoto]);
		}
		break;
	}
	}
}

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
	
		break;

	case BORRAR:

		break;

	case AYUDA:

		

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
			//checkInjectorResistance();
		}
		else if (indiceSubSubMenu == 1)
		{
			//displayFlowRate();
		}
		else if (indiceSubSubMenu == 2)
		{
		}
		else if (indiceSubSubMenu == 3)
		{
			//detectInjectorClickk();
		}
		else if (indiceSubSubMenu == 4)
		{
		}
		else if (indiceSubSubMenu == 5)
		{

		}
		else if (indiceSubSubMenu == 6)
		{
			estadoActual = SUBMENU_FUNCIONAMIENTO;
		}
		break;
	case SUBMENU_AUTOMATICO:
		// Lógica para realizar las pruebas automáticas
		//checkInjectorResistance();
		//displayFlowRate();
		//detectInjectorClickk();

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


// void checkInjectorResistance()
// {

// 	int resistencia = analogRead(34); // Ejemplo con el pin A2
// 	if (resistencia > 200 && resistencia < 500)
// 	{
// 		resultadoss[1] = true; // Prueba bien
// 	}
// 	else
// 	{
// 		resultadoss[1] = false; // Prueba mal
// 	}
// 	u8g2.sendBuffer();
// }

// void detectInjectorClickk()
// {

// 	int CLIC = analogRead(34); // Ejemplo con el pin A2
// 	if (CLIC > 200 && CLIC < 500)
// 	{
// 		resultadoss[4] = true; // Prueba bien
// 	}
// 	else
// 	{
// 		resultadoss[4] = false; // Prueba mal
// 	}
// }


// void displayFlowRate()
// {
// 	int CAUDAL = analogRead(34); // Ejemplo con el pin A2
// 	if (CAUDAL > 200 && CAUDAL < 500)
// 	{
// 		resultadoss[2] = true; // Prueba bien
// 	}
// 	else
// 	{
// 		resultadoss[2] = false; // Prueba mal
// 	}
// }
// 	// bool exit = false;
// 	// while (!exit) {
// 	//     if ((millis() - oldTime) > 1000) {
// 	//         detachInterrupt(digitalPinToInterrupt(4));
// 	//         flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
// 	//         oldTime = millis();
// 	//         flowMillilitres = (flowRate / 60) * 1000;
// 	//         pulseCount = 0;
// 	//         attachInterrupt(digitalPinToInterrupt(4), pulseCounter, FALLING);
// 	//     }
// 	//     lcd.clear();
// 	//     lcd.setCursor(0, 0);
// 	//     lcd.print("Flujo: ");
// 	//     lcd.setCursor(0, 1);
// 	//     lcd.print(flowRate);
// 	//     lcd.print(" L/min");
// 	//     delay(1000); // Pequeño retardo para evitar rebotes y actualizar con frecuencia

// 	//     if (digitalRead(botonPin) == LOW) {
// 	//         delay(50); // Debounce
// 	//         if (digitalRead(botonPin) == LOW) {
// 	//             while (digitalRead(botonPin) == LOW) {
// 	//                 delay(10); // Espera a que se suelte el botón
// 	//             }
// 	//             exit = true;
// 	//             estadoActual = SUBMENU_MANUAL;
// 	//         }
// 	//     }
// 	// }


