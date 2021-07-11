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
// File: TempoPot.ino
// Functions to handle Tempo potentiometer
//
// Author: Israel Roth
// Date: May 28, 2021
//
//---------------------------------------------------------

#define TEMPO_POT_PIN      A1
int last_tempo_pot_value = 0;

void setTempoFromPot(int val) {
  curBPM = 20 + (1023-val) / 3;
//  Serial.println("Set Tempo: "+ curBPM);
  setTempo();
}

void TempoPotSetup() {
  last_tempo_pot_value = stableAnalogRead(TEMPO_POT_PIN);
  setTempoFromPot(last_tempo_pot_value);
}

void handleTempoPot() {
  int sensorValue = stableAnalogRead(TEMPO_POT_PIN);
  if (abs(sensorValue-last_tempo_pot_value) < 5) {
    return;
  }
  last_tempo_pot_value = sensorValue;
  setTempoFromPot(last_tempo_pot_value);
}
