#include <WiFiS3.h>
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

// ====== DATOS QUE CADA EQUIPO DEBE CAMBIAR ======
const char* RED_WIFI   = "NOMBRE_DE_TU_RED";
const char* CLAVE_WIFI = "CLAVE_DE_TU_RED";
const char* HOST_MAKE  = "hook.us2.make.com";  // dominio de tu webhook
const char* RUTA_MAKE  = "/PEGA_AQUI_EL_ID";   // lo que sigue despues del dominio
// ================================================

const int PIN_DEMANDA = A0;
const int PIN_OFERTA  = A1;
const unsigned long ESPERA_ALERTA = 60000;    // maximo 1 alerta por minuto

ArduinoLEDMatrix matriz;
WiFiSSLClient cliente;
bool enEscasez = false;
unsigned long ultimaAlerta = 0;

void setup() {
  Serial.begin(9600);
  matriz.begin();
  mostrar("WiFi");
  while (WiFi.begin(RED_WIFI, CLAVE_WIFI) != WL_CONNECTED) {
    Serial.println("Conectando a WiFi...");
    delay(3000);
  }
  Serial.println("WiFi conectado");
  mostrar("OK");
}

void loop() {
  int demanda = map(analogRead(PIN_DEMANDA), 0, 1023, 10, 90);
  int oferta  = map(analogRead(PIN_OFERTA),  0, 1023, 10, 90);

  // Modelo lineal: demanda P = a - Q ; oferta P = Q - b
  int a = 100 + demanda;
  int b = oferta - 50;
  int precio = ((a - b) / 2) * 4;   // pesos por kilo

  Serial.print("Demanda: ");  Serial.print(demanda);
  Serial.print("  Oferta: "); Serial.print(oferta);
  Serial.print("  Precio: $"); Serial.println(precio);
  mostrar("$" + String(precio));

  bool escasez = (demanda - oferta) >= 25;
  if (escasez && !enEscasez && millis() - ultimaAlerta > ESPERA_ALERTA) {
    enviarAMake(demanda, oferta, precio);
    ultimaAlerta = millis();
  }
  enEscasez = escasez;
  delay(500);
}

void mostrar(String texto) {
  matriz.beginDraw();
  matriz.stroke(0xFFFFFFFF);
  matriz.textScrollSpeed(60);
  matriz.textFont(Font_4x6);
  matriz.beginText(0, 1, 0xFFFFFF);
  matriz.println(texto);
  matriz.endText(SCROLL_LEFT);
  matriz.endDraw();
}

void enviarAMake(int demanda, int oferta, int precio) {
  if (!cliente.connect(HOST_MAKE, 443)) {
    Serial.println("No se pudo conectar con Make");
    return;
  }
  String ruta = String(RUTA_MAKE) + "?demanda=" + demanda + "&oferta=" + oferta
              + "&precio=" + precio + "&estado=escasez";
  cliente.println("GET " + ruta + " HTTP/1.1");
  cliente.println("Host: " + String(HOST_MAKE));
  cliente.println("Connection: close");
  cliente.println();
  delay(500);
  cliente.stop();
  Serial.println("Alerta enviada a Make");
}
