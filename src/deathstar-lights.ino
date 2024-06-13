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

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
uint8_t cooling=75;

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
uint8_t sparking = 35;

CRGBPalette16 gPal;

void setup() {
  strip_long.begin();
  strip_short.begin();
  strip_long.setBrightness(100);
  strip_short.setBrightness(100);
  strip_short.fill(strip_short.Color(0,255,0));
  strip_short.show();
  strip_long.clear();
  strip_long.fill(strip_short.Color(0,255,0),0,POS_FIRE_LEDS-1);
  strip_long.show();

  // This first palette is the basic 'black body radiation' colors,
  // which run from black to red to bright yellow to white.
  // gPal = HeatColors_p;
  
  gPal = CRGBPalette16( CRGB::Red,CRGB::DarkOrange, CRGB::Yellow, CRGB::LightYellow);
  
  // like the heat colors, but blue/aqua instead of red/yellow
  // gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);
  // gPal = ForestColors_p;
  // gPal = PartyColors_p;
  // gPal = CloudColors_p;
  
  // Third, here's a simpler, three-step gradient, from black to red to white
  // gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::White);  

  delay(1000);
  Serial.printf("Lights for the deathstar-chair project\n");
  Serial.printf("%s commit #%s\n",GIT_URL, GIT_REV );

}

void loop() {
  static uint32_t lastprint;
  animation_idle(&strip_short,&strip_long,0.05,7);
  Fire2012(&strip_long);
  strip_short.show();
  strip_long.show();
  FastLED.delay(1000 / 20);
//  cooling=map(analogRead(PIN_A5),0,1024,0,255);
//  sparking=map(analogRead(PIN_A6),0,1024,0,255);
  if(Serial.available()) handleSerial();
  if(millis()-lastprint > 100) {
    lastprint = millis();
    Serial.printf("cool:%3d spark:%3d\r",cooling,sparking);
  }
}


void animation_idle(Adafruit_NeoPixel *strip_short, Adafruit_NeoPixel *strip_long,float speed,float wavelength) {
  uint16_t i, j;
    
    static uint32_t darkgreen = strip_short->Color(0,80,0);
    static uint32_t green = strip_short->Color(0,255,0);


    for(i=0; i< strip_short->numPixels(); i++) {
      int red = 0;
      int green = max(0,min(255, strip_short->sine8(i * wavelength + millis() * speed )-80));
      int blue = 0;
      uint32_t c = strip_short->Color(red, green, blue);
      if(i==0) c = strip_short->Color(0, 1, 0);
      strip_short->setPixelColor(strip_short->numPixels()-i, c);
      strip_long->setPixelColor(strip_short->numPixels()-i, c);
    }
}


void Fire2012(Adafruit_NeoPixel *strip)
{
  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_FIRE_LEDS; i++) {
      heat[i] = qsub8(heat[i], random(0, cooling ));
    }
  
// Step 2. Heat from each cell diffuses a little to the sides
  int left, right;
  for (int k = NUM_FIRE_LEDS - 1; k >= 1; k--) {
    left=k-1;
    right=k+1;
    if(left>=0)        heat[left] += qsub8(heat[k]/2,cooling);
    if(right<NUM_FIRE_LEDS) heat[right] += qsub8(heat[k]/2,cooling);
  }

  // Step 3. Randomly ignite new 'sparks' anywhere
  for (int k = NUM_FIRE_LEDS - 1; k >= 1; k--) {
    if (random(255) < sparking) {
      heat[k] = qadd8(heat[k], random(80, 180));
    }
  }

  // Step 4. Convert heat to LED colors
  for (int j = 0; j < NUM_FIRE_LEDS; j++) {
    setPixelHeatColor(strip, j+POS_FIRE_LEDS, heat[j]);
  }
}

void setPixelHeatColor(Adafruit_NeoPixel *strip, int pixel, byte temperature) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    uint8_t colorindex = scale8( temperature, 240);
    CRGB color = ColorFromPalette( gPal, colorindex);
    int pixelnumber;
    strip->setPixelColor(pixel, color.red, color.green,color.blue);
}

void handleSerial() {
  char buffer[3];
  int i=0;
  // read 3 bytes
  while(Serial.available() && i<3) {
    buffer[i++]=Serial.read();
  }
  if(i==3 && buffer[0]==27 && buffer[1]==91) {
    switch(buffer[2]) {
      case 65: // up
        sparking++;break;
      case 66: // down
        sparking--;break;
      case 68: // left
        cooling--;break;
      case 67: // right
        cooling++;break;
    }
  }

}