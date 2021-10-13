import ch.bildspur.artnet.*;

ArtNetClient artnet;
byte[] dmxData01 = new byte[512];
byte[] dmxData02 = new byte[512];



void clearLeds(int numLeds, String arduinoIP) {

  // fill dmx array
  byte[] dmxData = new byte[512];
  for (int i = 0; i < 512; i++) {
    dmxData[i] = (byte) 0;
  }
  // send dmx to localhost
  artnet.unicastDmx(arduinoIP, 0, 0, dmxData);
  artnet.unicastDmx(arduinoIP, 0, 1, dmxData);
}


void setupDMX(int numLeds, String arduinoIP)
{

  artnet = new ArtNetClient(null);
  artnet.start();
  clearLeds(numLeds, arduinoIP);
  for (int i = 0; i < 512; i++) {
    dmxData01[i] = (byte) 0;
    dmxData02[i] = (byte) 0;
  }
}

void sendDMX(int numLeds, String arduinoIP)
{
  loadPixels();
  int positionY = height/2;
  int pixelSeparation = width / numLeds;

  for (int i = 0; i < numLeds; i++) {

    int pixelPosition = width*height/2 + i*pixelSeparation;

    if (i < 170) {
      dmxData01[i*3 + 0] = (byte) red(pixels[pixelPosition]);
      dmxData01[i*3 + 1] = (byte) green(pixels[pixelPosition]);
      dmxData01[i*3 + 2] = (byte) blue(pixels[pixelPosition]);
    } else {
      int j = i - 170;
      dmxData02[j*3 + 0] = (byte) red(pixels[pixelPosition]);
      dmxData02[j*3 + 1] = (byte) green(pixels[pixelPosition]);
      dmxData02[j*3 + 2] = (byte) blue(pixels[pixelPosition]);
    }
  }

  artnet.unicastDmx(arduinoIP, 0, 0, dmxData01);
  artnet.unicastDmx(arduinoIP, 0, 1, dmxData02);
}

void exit() {

  clearLeds(numLeds, arduinoIP);
  super.exit();
}
