
int numLeds = 170;
//String arduinoIP = "192.168.1.114";
//String arduinoIP = "192.168.1.22";
String arduinoIP = "192.168.150.198";

float[] vector = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void setup () {
  size(900, 600);
  setupDMX(numLeds, arduinoIP);
  oscP5setup();
  AudioSetup();
}

void draw () {
  background(0); 
  
  //sendDMX(numLeds, arduinoIP);
  noStroke();
  fill(0,100,0); 
  
  for (int i = 0; i < vector.length; i++) {
    //println(vector[i]);
    float f = (float) vector[i];
    rect(i*20+20, height/2 + 50, 20, -f*100);
  }
  
  visualizacion(4);
  
  
  sendDMX(numLeds, arduinoIP);
  
  stroke(255);
  line(0,height/2,width,height/2);
  
}
