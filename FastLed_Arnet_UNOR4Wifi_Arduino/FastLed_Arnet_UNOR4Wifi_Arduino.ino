////////////////
//
// Implementación personalizada de ArtNet para Arduino UNO R4 WiFi
// Basado en el protocolo ArtNet pero adaptado específicamente para
// Arduino UNO R4 WiFi con soporte para tiras LED NeoPixel
//
///////////////

#include <WiFiS3.h>              // Biblioteca WiFi para Arduino UNO R4 WiFi
#include <Adafruit_NeoPixel.h>   // Biblioteca para controlar LEDs WS2812
#include <SPI.h>                 // Para comunicación SPI general
#include <WiFiUdp.h>             // Para comunicación UDP (ArtNet usa UDP)

// Wifi settings
#include "arduino_secrets.h"

// Archivo arduino_secrets.h debe contener:
//
// const int networks_num = n;    // Número de redes a las que te quieres conectar
// const char* networks_ssid[] = {
//   "myNetwork_ssid1",
//   "myNetwork_ssid2",
//   ... ,
//   "myNetwork_ssidN"
// };
//
// const char* networks_pass[] = {
//   "myNetwork_pass1",
//   "myNetwork_pass2",
//   ... ,
//   "myNetwork_passN"
// };

// Configuración ArtNet
#define ARTNET_PORT 6454           // Puerto UDP estándar para ArtNet
#define ARTNET_BUFFER_MAX 530      // Tamaño del buffer (suficiente para paquetes ArtNet)
#define ARTNET_HEADER "Art-Net"    // Encabezado ArtNet para validar paquetes
#define OPCODE_ARTDMX 0x5000       // OpCode para paquetes DMX en ArtNet

// LED Strip
const int numLeds = 100;           // Ajusta según tu configuración
const int numberOfChannels = numLeds * 3; // Total de canales DMX (1 led = 3 canales RGB)

// Pines para Arduino UNO R4 WiFi
#define DATA_PIN 5                 // Pin para datos de la tira LED

// Inicializa la tira LED NeoPixel
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numLeds, DATA_PIN, NEO_GRB + NEO_KHZ800);

// Para comunicación UDP
WiFiUDP Udp;
char packetBuffer[ARTNET_BUFFER_MAX]; // Buffer para paquetes entrantes

// Estado de conexión
WiFiServer server(80);
boolean node_connected = false;

// Built-in led (LED_BUILTIN es 2 en UNO R4 WiFi)
const int ledPin = LED_BUILTIN;    
boolean ledState = false;
unsigned long previous_millis = 0;
const long blink_interval = 1000;

// Control de estado
unsigned long lastWifiCheck = 0;
const long wifiCheckInterval = 10000;  // Comprueba la conexión cada 10 segundos

// Variables para universos DMX
const int startUniverse = 0;
int previousDataLength = 0;

// ------------------- Wifi Management ---------------------

int wifi_management() {
  // Escanea redes cercanas
  Serial.println("** Escaneando Redes **");
  int numSsid = WiFi.scanNetworks();

  Serial.print("Redes encontradas: ");
  Serial.println(numSsid);
  
  // Busca entre las redes disponibles
  int network_id = -1;
  for (int i = 0; i < networks_num; i++) {
    for (int thisNet = 0; thisNet < numSsid; thisNet++) {
      // Obtiene el SSID como String
      String currentSSID = WiFi.SSID(thisNet);
      
      // Compara el SSID con nuestra lista
      if (currentSSID == String(networks_ssid[i])) {
        Serial.print("Red encontrada: ");
        Serial.println(currentSSID);
        network_id = i;
        break;
      }
    }
    if (network_id >= 0) break;
  }

  if (network_id >= 0) {
    ConnectWifi(network_id);
  } else {
    Serial.println("No se encontraron redes conocidas");
    // Indica error con LEDs
    showConnection(false);
    digitalWrite(ledPin, HIGH);
    delay(5000);
  }

  server.begin();
  return 0;
}

boolean ConnectWifi(int id) {
  boolean state = true;
  int i = 0;

  // Usa const char* para WiFi.begin
  const char* ssid = networks_ssid[id];
  const char* pass = networks_pass[id];
  
  WiFi.begin(ssid, pass);
  Serial.println("");
  Serial.println("Conectando a WiFi");
  Serial.print("SSID: ");
  Serial.println(ssid);

  // Espera por conexión
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20) {
      state = false;
      break;
    }
    i++;
  }
  
  if (state) {
    digitalWrite(ledPin, HIGH);
    Serial.println("");
    Serial.print("Conectado a ");
    Serial.println(ssid);
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());
    
    // Inicia el servidor UDP para ArtNet
    Udp.begin(ARTNET_PORT);
    Serial.print("Escuchando paquetes ArtNet en puerto: ");
    Serial.println(ARTNET_PORT);
    
    showConnection(true);
    initTest();
    node_connected = true;
  } else {
    Serial.println("");
    Serial.println("Conexión fallida.");
    showConnection(false);
    digitalWrite(ledPin, HIGH);
  }

  return state;
}

// ---------------- Parse ArtNet Packet ---------------

boolean parseArtNetPacket() {
  // Comprueba si hay datos UDP disponibles
  int packetSize = Udp.parsePacket();
  
  if (packetSize <= 0) {
    return false; // No hay paquetes
  }
  
  // Recibe el paquete
  Udp.read(packetBuffer, ARTNET_BUFFER_MAX);
  
  // Comprueba si es un paquete ArtNet (comienza con "Art-Net")
  if (memcmp(packetBuffer, ARTNET_HEADER, 8) != 0) {
    return false; // No es un paquete ArtNet
  }
  
  // Obtiene el OpCode (posiciones 8 y 9 - recordar que en little endian)
  int opcode = packetBuffer[9] << 8 | packetBuffer[8];
  
  // Verifica si es un paquete DMX (ArtDmx)
  if (opcode != OPCODE_ARTDMX) {
    return false; // No es un paquete ArtDmx
  }
  
  // Obtén el universo desde el paquete (posiciones 14 y 15)
  int universe = packetBuffer[15] << 8 | packetBuffer[14];
  
  // Obtén la longitud de los datos DMX (posiciones 16 y 17)
  int dmxDataLength = packetBuffer[16] << 8 | packetBuffer[17];
  
  // Ahora procesamos los datos DMX (comienzan en la posición 18)
  processDmxData(universe, dmxDataLength, &packetBuffer[18]);
  
  return true;
}

// ---------------- Process DMX Data ---------------

void processDmxData(int universe, int length, uint8_t* data) {
  // Si es el universo 15, usa el primer byte para el brillo
  if (universe == 15) {
    strip.setBrightness(data[0]);
    strip.show();
    return;
  }
  
  // Solo procesamos datos que pertenecen al rango de universos que nos interesa
  if (universe < startUniverse) {
    return;
  }
  
  // Calculamos el LED inicial para este universo
  int ledsPerUniverse = 170; // 170 LEDs por universo (510 bytes / 3 bytes por LED)
  int firstLed = (universe - startUniverse) * ledsPerUniverse;
  
  // Procesamos los datos DMX y los convertimos en valores para los LEDs
  int ledsInThisUniverse = length / 3;
  
  for (int i = 0; i < ledsInThisUniverse; i++) {
    int ledIndex = firstLed + i;
    
    if (ledIndex < numLeds) {
      // Establecemos el color RGB para este LED
      strip.setPixelColor(ledIndex, strip.Color(
        data[i * 3],     // Red
        data[i * 3 + 1], // Green
        data[i * 3 + 2]  // Blue
      ));
    }
  }
  
  // Actualiza los LEDs
  strip.show();
  
  // Parpadea el LED incorporado para indicar recepción de datos
  digitalWrite(ledPin, !digitalRead(ledPin));
}

// ---------------- init Test ---------------

void initTest() {
  // Prueba inicial de LEDs - patrón de colores básicos
  int stopLed = numLeds > 10 ? 10 : numLeds;  // Usa máximo 10 LEDs para prueba
  
  // Azul
  for (int i = 0; i < stopLed; i += 5) {
    strip.setPixelColor(i, strip.Color(0, 0, 10));
  }
  strip.show();
  delay(2000);
  
  // Verde
  for (int i = 0; i < stopLed; i += 5) {
    strip.setPixelColor(i, strip.Color(0, 10, 0));
  }
  strip.show();
  delay(2000);
  
  // Rojo
  for (int i = 0; i < stopLed; i += 5) {
    strip.setPixelColor(i, strip.Color(10, 0, 0));
  }
  strip.show();
  delay(2000);
  
  // Apaga todos
  clearLEDs();
}

// Limpia todos los LEDs
void clearLEDs() {
  for (int i = 0; i < numLeds; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
}

// -----------  Show Connection ---------

void showConnection(boolean connection) {
  // Muestra estado de conexión en las primeras 5 LEDs
  int numToShow = numLeds > 5 ? 5 : numLeds;
  
  for (int i = 0; i < numToShow; i++) {
    if (connection) {
      strip.setPixelColor(i, strip.Color(0, 0, 255)); // Azul = conectado
    } else {
      strip.setPixelColor(i, strip.Color(255, 0, 0)); // Rojo = desconectado
    }
  }
  strip.show();

  delay(5000);
  clearLEDs();
}

// ----------- Builtin Led Blink ------------

void builtin_led_blink() {
  unsigned long current_millis = millis();

  if (current_millis - previous_millis >= blink_interval) {
    previous_millis = current_millis;
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }
}

// ---------------- Check WiFi Status ----------------

void checkWifiStatus() {
  unsigned long currentMillis = millis();
  
  // Verifica el estado del WiFi periódicamente
  if (currentMillis - lastWifiCheck >= wifiCheckInterval) {
    lastWifiCheck = currentMillis;
    
    if (WiFi.status() != WL_CONNECTED && node_connected) {
      Serial.println("Conexión WiFi perdida, intentando reconectar...");
      // Muestra desconexión en LEDs
      showConnection(false);
      node_connected = false;
      
      // Intenta reconectar
      wifi_management();
    }
  }
}

// ----------------   Setup --------------------

void setup() {
  // Configura el pin del LED incorporado
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  // Enciende el LED
  
  // Inicia comunicación Serial
  Serial.begin(115200);
  delay(3000); // Espera para abrir el monitor serial
  
  Serial.println("Iniciando Arduino UNO R4 WiFi ArtNet Node");
  
  // Inicializa NeoPixel
  strip.begin();
  strip.setBrightness(50);  // Brillo inicial al 50%
  strip.show(); // Inicializa todos los píxeles en 'apagado'
  
  // Conecta WiFi
  wifi_management();
}

// ----------------- Loop ----------------

void loop() {
  // Busca y procesa paquetes ArtNet
  parseArtNetPacket();
  
  // Parpadea LED incorporado para mostrar que está funcionando
  builtin_led_blink();
  
  // Verifica estado WiFi periódicamente
  checkWifiStatus();
}