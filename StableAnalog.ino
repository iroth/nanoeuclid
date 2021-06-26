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
// File: StableAnalog.ino
// Functions to handle Analog input more reliably
//
// Author: Israel Roth
// Date: May 28, 2021
//
//---------------------------------------------------------

#define NUM_READS     10

int stableAnalogRead(int pin) {
  int sum = 0;
  for (int i = 0 ; i < NUM_READS ; i++) {
    sum += analogRead(pin);
  }
  sum = sum / NUM_READS;
  return sum;
}
