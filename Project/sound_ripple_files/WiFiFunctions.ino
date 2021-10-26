#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>


int statusCode;
const char* ssid = "my_SSID";
const char* passphrase = "my_Password";
String st;
String content;

//Establishing Local server at port 80 whenever required
ESP8266WebServer server(80);

boolean wifiManagment = true;

int ledModeControl = 1;

int actualLedMode (){
  Serial.println("");
  Serial.println("mode = " + ledModeControl);
  return ledModeControl;
}

void WiFisetup()
{

  WiFi.disconnect();

  //---------------------------------------- Read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");
  String esid = readEEPROM(1);
  Serial.println("SSID = " + esid);

  String epass = readEEPROM(2);
  Serial.println("PASS = " + epass);

  WiFi.begin(esid.c_str(), epass.c_str());

  if (testWifi())
  {
    Serial.println("Succesfully Connected!!!");

    server.begin();
    createWebServer();
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
    //return;
  }
  else
  {
    Serial.println("Turning the HotSpot On");
    setupAP();// Setup HotSpot
  }

  Serial.println();
  Serial.println(WiFi.status());

  while (wifiManagment)
  {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }
}


bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "";//"<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<option>";
    st += WiFi.SSID(i);
    //st += " (";
    //st += WiFi.RSSI(i);

    //st += ")";
    //st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</option>";
  }
  //st += "</ol>";
  delay(100);
  WiFi.softAP("Control-LED", "");
  Serial.println("Initializing_softap_for_wifi credentials_modification");
  Serial.println("");
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  server.begin();
  Serial.println("Server started");

}

void homePage() {
  IPAddress ip = WiFi.softAPIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  content = "<!DOCTYPE HTML>\r\n<html>Welcome to Wifi Credentials Update page";
  content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
  content += ipStr;
  content += "<p>";
  content += st;
  content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
  content += "</html>";
  server.send(200, "text/html", content);
}

void homePageBootstrap() {
  IPAddress ip = WiFi.softAPIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  content = "<!DOCTYPE html>\n<html lang=\"en\">\n<head><meta charset=\"utf-8\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n<title>Network List</title>";
  content += "<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-1BmE4kWBq78iYhFldvKuhfTAU6auU8tT94WrHftjDbrCEXSU1oBoqyl2QvZ6jIW3\" crossorigin=\"anonymous\">";
  content += "<style>\n html, body {height: 100%;}\n body {display: flex;align-items: center;padding-top: 40px;padding-bottom: 40px;background-color: #f5f5f5;}\n .form-signin {width: 100%;max-width: 330px;padding: 15px;margin: auto;}\n .form-signin .form-floating:focus-within {z-index: 2;}";
  content += "\n.inputSelect {border-bottom-right-radius: 0;border-bottom-left-radius: 0;}\n .form-signin input[type=\"password\"] {margin-bottom: 10px; border-top-left-radius: 0;border-top-right-radius: 0;}\n .bd-placeholder-img { font-size: 1.125rem; text-anchor: middle; -webkit-user-select: none; -moz-user-select: none; user-select: none;}\n @media (min-width: 768px) {bd-placeholder-img-lg {font-size: 3.5rem;}}\n</style>\n</head>";
  content += "\n<body class=\"text-center\"><main class=\"form-signin\">\n<form>\n<h1 class=\"h3 mb-3 fw-normal\">List of Networks</h1>\n<div class=\"form-floating\">\n<select type=\"email\" id=\"inputState\" class=\"form-control\">\n<option selected>SSID</option>";
  content += st;
  content += "\n</select>\n</div>\n<div class=\"form-floating\">\n<input type=\"password\" class=\"form-control\" id=\"floatingPassword\" placeholder=\"Password\">\n<label for=\"floatingPassword\">Password</label>\n</div>\n</div>\n<button class=\"w-100 btn btn-lg btn-primary\" type=\"submit\">Enter</button>\n</form>\n</main>\n</body>\n</html>";
  server.send(200, "text/html", content);
}

void sendingWifiCredentialPage() {
  String qsid = server.arg("ssid");
  String qpass = server.arg("pass");

  Serial.println("SSID ..= " + qsid);
  Serial.println("PASS ..= " + qpass);

  if (qsid.length() > 0 && qpass.length() > 0) {
    Serial.println("clearing eeprom");

    Serial.println("writing eeprom ssid:");

    writeEEPROM( 1, qsid.c_str());
    writeEEPROM( 2, qpass);

    Serial.println("SSID xxx= " + readEEPROM(1));
    Serial.println("PASS xxx= " + readEEPROM(2));

    content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
    statusCode = 200;
    ESP.reset();
  } else {
    content = "{\"Error\":\"404 not found\"}";
    statusCode = 404;
    Serial.println("Sending 404");
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(statusCode, "application/json", content);
}

void sendingModeSelectedPage() {
  String modeSelected = server.arg("mode");
  Serial.println("");
  Serial.println(modeSelected);
  ledModeControl = modeSelected.toInt();
  wifiManagment = false;
  content = "{\"Success\":\"new mode selected mode " + modeSelected +"\"}";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", content);
}

void scanPage() {
  //setupAP();
  IPAddress ip = WiFi.softAPIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
  content = "<!DOCTYPE HTML>\r\n<html>go back";
  server.send(200, "text/html", content);
}

void modePage() {
  content = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"><title>Cotrol Mode Select List</title></head><body>";
  content += "<form method='get' action='modeSelect'><h1>Choose the Control Led Mode:</h1>";
  content += "<p><select name=\"mode\" required><option value="">Control LED List</option><optgroup label=\"Reactives\">";
  content += "<option value=\"1\">LED Sound Riples</option></optgroup><optgroup label=\"Wifi\"><option value=\"2\">Artnet Control</option></optgroup>";
  content += "<optgroup label=\"Basic\"><option value=\"3\">Rainbow</option></optgroup></select><input type=\"submit\" value=\"Submit\"></p></form>";
  content += "</body></html>";
  server.send(200, "text/html", content);
}

void createWebServer()
{
  server.on("/", homePage);
  server.on("/scan", scanPage);
  server.on("/setting", sendingWifiCredentialPage);
  server.on("/mode", modePage);
  server.on("/modeSelect", sendingModeSelectedPage);
}
