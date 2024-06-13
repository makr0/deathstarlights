#include <Adafruit_NeoPixel.h>
#include <FastLED.h>

#define PIN_LONG PIN_A0
#define PIN_SHORT PIN_A1
#define LED_NUM_SHORT 9
#define LED_NUM_LONG 32-LED_NUM_SHORT
#define NUM_FIRE_LEDS LED_NUM_LONG - LED_NUM_SHORT
#define POS_FIRE_LEDS 9
#define REVERSE_FIRE

Adafruit_NeoPixel strip_long = Adafruit_NeoPixel(LED_NUM_LONG, PIN_LONG, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip_short = Adafruit_NeoPixel(LED_NUM_SHORT, PIN_SHORT, NEO_GRB + NEO_KHZ800);
// Array of temperature readings at each simulation cell
static uint8_t heat[NUM_FIRE_LEDS];

void setup() {
  strip_long.begin();
  strip_long.setBrightness(100);
  strip_long.show(); 
  strip_short.begin();
  strip_short.setBrightness(100);
  strip_short.show();
}

void loop() {
  animation_idle(&strip_short,&strip_long,0.03,10);
  Fire2012();
  strip_short.show();
  strip_long.show();
  FastLED.delay(1000 / 20);
}


void animation_idle(Adafruit_NeoPixel *strip_short, Adafruit_NeoPixel *strip_long,float speed,float wavelength) {
  uint16_t i, j;
    
    uint32_t darkgreen = strip_short->Color(0,80,0);
    uint32_t green = strip_short->Color(0,255,0);


    for(i=0; i< strip_short->numPixels(); i++) {
      int red = 0;
      int green = max(0,min(255, strip_short->sine8(i * wavelength + millis() * speed )-80));
      int blue = 0;
      uint32_t c = strip_short->Color(red, green, blue);
      if(i==0) c /=3;
      strip_short->setPixelColor(strip_short->numPixels()-i, c);
      strip_long->setPixelColor(strip_short->numPixels()-i, c);
    }
}


// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
#define COOLING  100

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 50


void Fire2012()
{
  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_FIRE_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_FIRE_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_FIRE_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_FIRE_LEDS; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      #ifdef REVERSE_FIRE
        pixelnumber = (NUM_FIRE_LEDS-1) - j;
      #else
        pixelnumber = j;
      #endif
      strip_long.setPixelColor(pixelnumber + POS_FIRE_LEDS,color.red,color.green,color.blue);
    }
}