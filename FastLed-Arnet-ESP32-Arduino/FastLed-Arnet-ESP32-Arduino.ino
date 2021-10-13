////////////////
//
// Basado en:
// https://learn.sparkfun.com/tutorials/using-artnet-dmx-and-the-esp32-to-drive-pixels/creating-an-artnet-node-on-the-esp32
//
///////////////
//
// TODO:
//     - The led strip or the builtin led show when the conection was lost
//     - Reboot the nodemcu when the conection was lost
//     - Improve the init testing, try to show all the leds working dynamically
//     - Add web server with a info page
//
//
/////////

#include <ArtnetWifi.h>
#include <FastLED.h>


//Wifi settings
#include "arduino_secrets.h"

// You need to create the file arduino_secrets.h with the wifi credentials
//
//const int networks_num = n;    //number of total networks that you want to connect the nodemcu
//const String networks_ssid[] = {
//  "myNetwork_ssid1",
//  "myNetwork_ssid2",
//  ... ,
//  "myNetwork_ssidN"
//};

//const String networks_pass[] = {
//  "myNetwork_pass1",
//  "myNetwork_pass2",
//  ... ,
//  "myNetwork_passN"
//};

WiFiServer server(80);

boolean node_connected = false;

// LED Strip
// const int numLeds = 300; // Change if your setup has more or less LED's
const int numLeds = 140;
const int numberOfChannels = numLeds * 3; // Total number of DMX channels you want to receive (1 led = 3 channels)

#define DATA_PIN 6  //The data pin that the APA_102 strips are connected to.
#define CLOCK_PIN 7 //The clock pin that the APA_102 strips are connected to.

CRGB leds[numLeds];

// Artnet settings
ArtnetWifi artnet;
const int startUniverse = 0;

bool sendFrame = 1;
int previousDataLength = 0;

// Built-in led blink variables
unsigned long previous_millis = 0;
const long blink_interval = 1000;
const int ledPin = 2;
boolean ledState = false;

// ------------------- Wifi Managmanet ---------------------
// wifi scanNetworks, wifi connection

int wifi_managment() {
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  byte numSsid = WiFi.scanNetworks();

  // print the list of networks seen:
  Serial.print("SSID List:");
  Serial.println(numSsid);
  // print the network number and name for each network found:
  //for (int thisNet = 0; thisNet < numSsid; thisNet++) {
  //  Serial.print(thisNet);
  //  Serial.print(") Network: ");
  //  Serial.println(WiFi.SSID(thisNet));
  //}

  int network_id = -1;
  for (int i = 0; i < networks_num; i++) {
    for (int thisNet = 0; thisNet < numSsid; thisNet++) {
      if (networks_ssid[i] == WiFi.SSID(thisNet)) {
        Serial.print("Network found ");
        Serial.println(WiFi.SSID(thisNet));
        network_id = i;
        break;
      } else {
        Serial.println("Network Not found! ");
      }
    }
  }

  if (network_id >= 0) {
    ConnectWifi(network_id);
  } else {
    while (1) {
      showConnection(0);
      analogWrite(ledPin, 800);
      delay(5000);
    }
  }

  server.begin();

}


//------------------------ Connect Wifi --------------
// connect to wifi – returns true if successful or false if not

boolean ConnectWifi(int id)
{
  boolean state = true;
  int i = 0;

  WiFi.begin(networks_ssid[id], networks_pass[id]);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
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
    analogWrite(ledPin, 800);
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(networks_ssid[id]);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    WiFi.hostname("WebPool");
    showConnection(state);
    initTest();
    node_connected = true;
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
    showConnection(state);
    analogWrite(ledPin, 800);
  }

  return state;
}

// ---------------- on DMX Frame ---------------
// send the recibe DMX to the leds

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data){
  
  sendFrame = 1;
  // set brightness of the whole strip Send the Brightness in the first byte from the 15th universe
  if (universe == 15){
    FastLED.setBrightness(data[0]);
  }
  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++){
    int led = i + (universe - startUniverse) * (length / 3);
    if (led < numLeds){
      leds[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    }
  }
  previousDataLength = length;
  FastLED.show();
}

// ---------------- init Test ---------------//
// testing the strip leds

void initTest()
{
  int stopLed = 10;
  for (int i = 0 ; i < stopLed ; i += 5)
    leds[i] = CRGB(0, 0, 10);
  FastLED.show();
  delay(2000);
  for (int i = 0 ; i < stopLed ; i += 5)
    leds[i] = CRGB(0, 10, 0);
  FastLED.show();
  delay(2000);
  for (int i = 0 ; i < stopLed ; i += 5)
    leds[i] = CRGB(10, 0, 0);
  FastLED.show();
  delay(2000);
  for (int i = 0 ; i < stopLed ; i += 5)
    leds[i] = CRGB(0, 0, 0);
  FastLED.show();
}

// -----------  Show Connection ---------
// show that we are connected to the net in the strip led, Blue = conected, Red = not conected

void showConnection(boolean connection) {
  for (int i = 0 ; i < 5 ; i++)
    leds[i] = connection ? CRGB::Blue : CRGB:: Red;
  FastLED.show();

  delay(5000);
  FastLED.clear();
}

// ----------- Builtin Led Blink ------------
// control the built led

void builtin_led_blink () {

  unsigned long current_millis = millis();

  if (current_millis - previous_millis >= blink_interval) {
    previous_millis = current_millis;

    if (ledState == LOW) {
      ledState = HIGH;
      analogWrite(ledPin, 1012);
    } else {
      ledState = LOW;
      analogWrite(ledPin, 1024);
    }

    //digitalWrite(ledPin, ledState);

  }
}



// ----------------   Setup --------------------

void setup()
{
  //pinMode(ledPin, OUTPUT);
  //analogWrite(ledPin, 800); // remember HIGH is off
  Serial.begin(115200);
  
  //Serial.print("LED_BUILTIN = ");
  //Serial.println(LED_BUILTIN, DEC);

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, numLeds); // Strip Led Neopixels (WS2812)
  //FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, numLeds); // Matrix LED APA102

  wifi_managment();

  artnet.begin();
  // onDmxFrame will execute every time a packet is received by the ESP32
  artnet.setArtDmxCallback(onDmxFrame);
}


// ----------------- Loop ----------------

void loop()
{
  artnet.read();
  //builtin_led_blink();
  //webclientloop();
}
