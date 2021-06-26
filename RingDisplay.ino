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
const color_t RED =    {180, 0, 0};
const color_t GREEN =  {0, 180, 0};
const color_t BLUE =   {0, 0, 180};

color_t plen[NUM_CHANNELS] = { PINK, YELLOW, ORANGE, RED };
color_t actv[NUM_CHANNELS] = { BLUE, PURPLE, VIOLET, CYAN };
int brightness = 20;

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
  color_t show[4] = {{150, 150,0}, {200,100,0}, {200,0,0}, {150,0,150}};
  for (int i = 0 ; i < 4 ; i++) {
    ringCycle(&show[i]);
  }
}

void clockToInternal() {
  color_t red = {200,0,0};  
  ringCycle(&red);
}

void clockToExternal() {
  color_t green = {0,200,0};  
  ringCycle(&green);
}

void displayLedRingWhilePlaying() {
  pixels.clear(); // Set all pixel colors to 'off'
  brightness = 20;
  if (clock_source == CLK_SRC_EXT){
    brightness = 50;
  }
  pixels.setBrightness(brightness);
  for(int i=0; i<pattern_length[curChannel]; i++) { // For each pixel...
    if (euclid_pattern[curChannel][i]) {
      pixels.setPixelColor(i, pixels.Color(actv[curChannel].r, actv[curChannel].g, actv[curChannel].b));
    }
    else {
      pixels.setPixelColor(i, pixels.Color(plen[curChannel].r, plen[curChannel].g, plen[curChannel].b));
    }
  }
  pixels.setPixelColor(gSeqNoteIndex[curChannel], 50,250,50);  
  pixels.show();   // Send the updated pixel colors to the hardware.
}

void displayLedRingWhileEditing() {
  pixels.clear(); // Set all pixel colors to 'off'
  brightness = 20;
  if (clock_source == CLK_SRC_EXT){
    brightness = 50;
  }
  pixels.setBrightness(brightness);
  switch(edit_mode) {
    case EDIT_MODE_PATTERN_LEN:
      for(int i=0; i<pattern_length[curChannel]; i++) { // For each pixel...
        if (euclid_pattern[curChannel][i]) {
          pixels.setPixelColor(i, pixels.Color(actv[curChannel].r, actv[curChannel].g, actv[curChannel].b));
        }
        else {
          pixels.setPixelColor(i, pixels.Color(plen[curChannel].r, plen[curChannel].g, plen[curChannel].b));
        }
      }
      break;      
    case EDIT_MODE_ACTIVE_BEATS:
      for(int i=0; i<pattern_length[curChannel]; i++) { // For each pixel...
        if (euclid_pattern[curChannel][i]) {
          pixels.setPixelColor(i, pixels.Color(200, 200, 200));
        }
        else {
          pixels.setPixelColor(i, pixels.Color(plen[curChannel].r, plen[curChannel].g, plen[curChannel].b));
        }
      }
      break;      
    case EDIT_MODE_OFFSET:
      for(int i=0; i<pattern_length[curChannel]; i++) { // For each pixel...
        if (euclid_pattern[curChannel][i]) {
          pixels.setPixelColor(i, pixels.Color(40, 180, 0));
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
            pixels.setPixelColor(i, pixels.Color(0, 200, 0));
          }
          else {
            pixels.setPixelColor(i, pixels.Color(200, 0, 0));            
          }
        }
      }      
      break;      
  }
  pixels.show();   // Send the updated pixel colors to the hardware.
}

void displayLedRing() {
  if (isPlaying)   
  {
    displayLedRingWhilePlaying();
  }
  else {    
    displayLedRingWhileEditing();
  }
}
