// Numero de leds
int numLeds = 100;
// IP del arduino 
//String arduinoIP = "192.168.8.128";
String arduinoIP = "192.168.150.240";


void setup () {
  size(900, 600);
  setupDMX(numLeds, arduinoIP);
  AudioSetup();
}

void draw () {
  background(0); 
  noStroke();
  fill(0,100,0); 
  
  
  visualizacion(1);
  // 0 = cuadro que se mueve con el mouse
  // 1 = rallas de colores que se mueven automanticamente
  // 2 = cuadro de otro color
  // 3 = todos los leds cambian de color en conjunto
  // 4 = circulo blanco central que se mueve con el sonido en el microfono
  
  //sendDMX(numLeds, arduinoIP);
  
  stroke(255);
  line(0,height/2,width,height/2);
  
}
