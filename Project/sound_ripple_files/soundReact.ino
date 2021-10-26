/* This sketch is based on Sound
 * 
 */

#define MIC_PIN   0              // Analog port for microphone

uint8_t squelch = 7;             // Anything below this is background noise, so we'll make it '0'.
int sample;                      // Current sample.
float sampleAvg = 0;             // Smoothed Average.
float micLev = 0;                // Used to convert returned value to have '0' as minimum.
uint8_t maxVol = 11;             // Reasonable value for constant volume for 'peak detector', as it won't always trigger.
bool samplePeak = 0;             // Boolean flag for peak. Responding routine must reset this flag.

// Ripple variables
uint8_t colour;                     // Ripple colour is randomized.
int center = 0;                     // Center of the current ripple.
int step = -1;                      // -1 is the initializing step.
uint8_t myfade = 255;               // Starting brightness.
#define maxsteps 16                 // Case statement wouldn't allow a variable.

void getSample() {
  
  int16_t micIn;                                              // Current sample starts with negative values and large values, which is why it's 16 bit signed.
  static long peakTime;
  
  micIn = analogRead(MIC_PIN);                                // Poor man's analog Read.
  micLev = ((micLev * 31) + micIn) / 32;                      // Smooth it out over the last 32 samples for automatic centering.
  micIn -= micLev;                                            // Let's center it to 0 now.
  micIn = abs(micIn);                                         // And get the absolute value of each sample.
  sample = (micIn <= squelch) ? 0 : (sample + micIn) / 2;     // Using a ternary operator, the resultant sample is either 0 or it's a bit smoothed out with the last sample.
  sampleAvg = ((sampleAvg * 31) + sample) / 32;               // Smooth it out over the last 32 samples.

  if (sample > (sampleAvg+maxVol) && millis() > (peakTime + 50)) {    // Poor man's beat detection by seeing if sample > Average + some value.
    samplePeak = 1;                                                   // Then we got a peak, else we don't. Display routines need to reset the samplepeak value in case they miss the trigger.
    peakTime=millis();                
  }                                                           

}  // getSample()



void ripple() {

  if (samplePeak == 1) {step = -1; samplePeak = 0; }          // If we have a peak, let's reset our ripple.

  fadeToBlackBy(leds, NUM_LEDS, 64);                          // 8 bit, 1 = slow, 255 = fast
  
  switch (step) {

    case -1:                                                  // Initialize ripple variables.
      center = random(NUM_LEDS);
      colour = random8();                                     // More peaks/s = higher the hue colour.
      step = 0;
      break;

    case 0:
      leds[center] = CHSV(colour, 255, 255);                  // Display the first pixel of the ripple.
      step ++;
      break;

    case maxsteps:                                            // At the end of the ripples.
      // step = -1;
      break;

    default:                                                  // Middle of the ripples.
      leds[(center + step + NUM_LEDS) % NUM_LEDS] += CHSV(colour, 255, myfade/step*2);       // Simple wrap.
      leds[(center - step + NUM_LEDS) % NUM_LEDS] += CHSV(colour, 255, myfade/step*2);
      step ++;                                                // Next step.
      break;  
  } // switch step
  
} // ripple()
