//---------------------------------------------------------
//--------------- EuclidNamo ------------------------------
//---------------------------------------------------------
//
// Euclidean Rythm Sequencer using Arduino Nano Every as
// the processor, a NeoPixels led ring as the main UI
// Rotary Encoder + push button, two buttons a toggle and
// a potentiometer as input devices
//
// The sequencer has 4 channels each with up to 16 steps
// 4 Eurorack Gate outputs are provided
// The sequencer has internal clock, but can be switch to
// use an external clock from Eurorack
//
//---------------------------------------------------------

//---------------------------------------------------------
//
// File: RingDisplay.ino
// Functions to handle the display on the NeoPixels Ring
// From Adafruit
//
// Author: Israel Roth
// Date: May 28, 2021
//
//---------------------------------------------------------

#include <Adafruit_NeoPixel.h>

//--------------- NeoPixels Stuff --------------------
#define NEOPIXELS_PIN       6
#define NUMPIXELS 16 // Popular NeoPixel ring size
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXELS_PIN, NEO_GRB + NEO_KHZ800);

struct color_t
  {
      byte r;
      byte g;
      byte b;
  };
void ringCycle(color_t *c);

const color_t CYAN =   {0, 120, 120};
const color_t YELLOW = {120, 120, 0};
const color_t ORANGE = {180, 40, 0};
const color_t PINK =   {180, 20, 40};
const color_t PURPLE = {120, 0, 120};
const color_t VIOLET = {40, 0, 180};
const color_t RED =    {150, 0, 0};
const color_t GREEN =  {0, 80, 0};
const color_t BLUE =   {0, 0, 80};

const color_t LIGHT_YELLOW =   {60, 40, 10};
const color_t LIGHT_ORANGE =   {90, 40, 10};
const color_t LIGHT_RED =   {110, 30, 0};

color_t plen[NUM_CHANNELS] = { LIGHT_YELLOW, LIGHT_ORANGE, LIGHT_RED, RED };
color_t actv[NUM_CHANNELS] = { BLUE, BLUE, BLUE, BLUE };
int brightness = 15;

void RingDisplaySetup() {
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(brightness);
  openingShow();
}

void ringCycle(color_t *c) {
    pixels.clear(); // Set all pixel colors to 'off'
    for (int j = 0; j < 16 ; j++) {
      pixels.setPixelColor(j, pixels.Color(c->r, c->g, c->b));
      pixels.show();   // Send the updated pixel colors to the hardware.
      delay(20);
    }
}

void openingShow() {
  color_t show[4] = {LIGHT_YELLOW, LIGHT_ORANGE, LIGHT_RED, RED};
  for (int i = 0 ; i < 4 ; i++) {
    ringCycle(&show[i]);
  }
}

void clockToInternal() {
  color_t red = {80,0,0};  
  ringCycle(&red);
}

void clockToExternal() {
  color_t green = {0,80,0};  
  ringCycle(&green);
}

void displayLedRing() {
  pixels.clear(); // Set all pixel colors to 'off'
  switch(edit_mode) {
    case EDIT_MODE_PATTERN_LEN:
      for(int i=0; i<pattern_length[curChannel]; i++) { // For each pixel...
        if (euclid_pattern[curChannel][i]) {
          pixels.setPixelColor(i, pixels.Color(0, 80, 20));
        }
        else {
          pixels.setPixelColor(i, pixels.Color(plen[curChannel].r, plen[curChannel].g, plen[curChannel].b));
        }
      }
      break;      
    case EDIT_MODE_ACTIVE_BEATS:
      for(int i=0; i<pattern_length[curChannel]; i++) { // For each pixel...
        if (euclid_pattern[curChannel][i]) {
          pixels.setPixelColor(i, pixels.Color(40, 0, 80));
        }
        else {
          pixels.setPixelColor(i, pixels.Color(plen[curChannel].r, plen[curChannel].g, plen[curChannel].b));
        }
      }
      break;      
    case EDIT_MODE_OFFSET:
      for(int i=0; i<pattern_length[curChannel]; i++) { // For each pixel...
        if (euclid_pattern[curChannel][i]) {
          pixels.setPixelColor(i, pixels.Color(0, 20, 80));
        }
        else {
          pixels.setPixelColor(i, pixels.Color(plen[curChannel].r, plen[curChannel].g, plen[curChannel].b));
        }
      }
      break;      
    case EDIT_MODE_MEMORY:
      for(int i=0; i<16; i++) { // For each pixel...
        if (i == cur_memory_pos) {
          pixels.setPixelColor(i, pixels.Color(120, 100, 120));
        }
        else {
          if (isSlotTaken(i)) {
            pixels.setPixelColor(i, pixels.Color(0, 100, 0));
          }
          else {
            pixels.setPixelColor(i, pixels.Color(100, 0, 0));            
          }
        }
      }      
      break;      
  }
  if (isPlaying) {
      pixels.setPixelColor(gSeqNoteIndex[curChannel], 10,80,10);  
  }
  pixels.show();   // Send the updated pixel colors to the hardware.
}
