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
// File: ClockToggle.ino
// Functions to handle Clock Source Toggle (Int/Ext)
//
// Author: Israel Roth
// Date: May 28, 2021
//
//---------------------------------------------------------

#define CLK_SRC_PIN      12

int last_toggle_val = LOW;

void ClockToggleSetup() {
  pinMode(CLK_SRC_PIN, INPUT_PULLUP);
  int last_toggle_val = digitalRead(CLK_SRC_PIN);
  clock_source = last_toggle_val == LOW ? CLK_SRC_INT : CLK_SRC_INT;
}

void handleClockToggle() {
  int sensorValue = digitalRead(CLK_SRC_PIN);
  if (sensorValue == last_toggle_val) {
    return;
  }
  last_toggle_val = sensorValue;
  if (sensorValue == LOW) {
    clock_source = CLK_SRC_INT;
    clockToInternal();
  }
  else {
    clock_source = CLK_SRC_EXT;
    clockToExternal();
  }
}