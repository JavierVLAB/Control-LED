import processing.sound.*;

AudioIn input;
Amplitude loudness;

void AudioSetup() {

  // Create an Audio input and grab the 1st channel
  input = new AudioIn(this, 0);

  // Begin capturing the audio input
  input.start();
  // start() activates audio capture so that you can use it as
  // the input to live sound analysis, but it does NOT cause the
  // captured audio to be played back to you. if you also want the
  // microphone input to be played back to you, call
  //    input.play();
  // instead (be careful with your speaker volume, you might produce
  // painful audio feedback. best to first try it out wearing headphones!)

  // Create a new Amplitude analyzer
  loudness = new Amplitude(this);

  // Patch the input to the volume analyzer
  loudness.input(input);
}


void visualizacion(int n) {
  if (n == 0) { 
    fill(100, 0, 0);
    rect(mouseX, mouseY, 20, 80);
  }
  if (n == 1) {
    int t = frameCount;
    colorMode(HSB);
    for (int i = 0; i < 9; i++) {
      fill((i*10+t)%255, 255, 255);
      rect((i*200 + t)%width, 0, 70, height);
    }
  }
  if (n == 2) {
    rect(0, 150, 100, 100);
  }
  if (n == 3) {
    int t = frameCount;
    colorMode(HSB);
    background((t)%255, 255, 255);
    //for (int i = 0; i < 9; i++) {
    //  fill((i*10+t)%255, 255, 255);
    //  rect((i*200 + t)%width, 0, 70, height);
    //}
  }

  if (n == 4) {

    float inputLevel = map(mouseY, 0, height, 1.0, 0.0);
    input.amp(inputLevel);

    // loudness.analyze() return a value between 0 and 1. To adjust
    // the scaling and mapping of an ellipse we scale from 0 to 0.5
    float volume = loudness.analyze();
    int size = int(map(volume, 0, 0.5, 1, 850));
    int hue = int(map(volume, 0, 1.0, 1, 850));
    noStroke();
    colorMode(HSB);
    int t = frameCount;
    fill(255, 0, 150);
    // We draw a circle whose size is coupled to the audio analysis
    ellipse(width/2, height/2, size, size);
  }
}
