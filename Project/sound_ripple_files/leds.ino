#include <FastLED.h>  


#define LED_DT 6                           // Data pin to connect to the strip.
#define LED_CK 5                           // Clock pin for WS2801 or APA102.
#define COLOR_ORDER RGB                    // It's GRB for WS2812 and BGR for APA102.
#define LED_TYPE WS2812 //APA102                    // Using APA102, WS2812, WS2801. Don't forget to modify LEDS.addLeds to suit.
#define NUM_LEDS 75                       // Number of LED's.

struct CRGB leds[NUM_LEDS];                                   // Initialize our LED array.

int max_bright = 255;

void setupFastLEDs(){

  LEDS.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, NUM_LEDS);  // Use this for WS2812B
   //LEDS.addLeds<LED_TYPE, LED_DT, LED_CK, COLOR_ORDER>(leds, NUM_LEDS);  // Use this for WS2801 or APA102

  FastLED.setBrightness(max_bright);
  
}
