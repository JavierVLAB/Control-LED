import oscP5.*;
import netP5.*;

OscMessage msg;

OscP5 oscP5;

int puertaEscucha = 7777;

boolean show = false;

void oscP5setup() {

  oscP5 = new OscP5(this, puertaEscucha);
}

void oscEvent(OscMessage theOscMessage) {

  if (theOscMessage.checkAddrPattern("/mfcc_in")==true) {
    for (int i = 0; i < theOscMessage.arguments().length; i++) {
      vector[i] = (float) theOscMessage.arguments()[i];
      println(theOscMessage.arguments()[i]);
    }
  }
}
