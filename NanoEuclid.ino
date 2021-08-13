//---------------------------------------------------------
//--------------- NanoEuclid ------------------------------
//---------------------------------------------------------
//
// Euclidean Rythm Sequencer using Arduino Nano Every as
// the processor, a NeoPixels led ring as the main UI
// Rotary Encoder + push button, two buttons a toggle and
// a potentiometer as input devices
//
// The sequencer has 4 channels each with up to 16 steps
// 4 Eurorack Gate outputs are provided
// The sequencer has internal clock, but can be switched to
// use an external clock from Eurorack
//
//---------------------------------------------------------

//---------------------------------------------------------
//
// File: NanoEuclid.ino
// This is the main file of the sequencer
//
// Author: Israel Roth
// Date: May 28, 2021
//
//---------------------------------------------------------

//--------------- Actual Seq Params --------------------
int pattern_len = 16;
int active_beats = 1;
int cur_memory_pos = 0;


//--------------- Sequencer Stuff --------------------

bool isPlaying = false;

#define NUM_CHANNELS 4
int curChannel = 0;
byte euclid_pattern[NUM_CHANNELS][16];
int pattern_length[NUM_CHANNELS] = {16, 16, 16, 16};
int pattern_nactive[NUM_CHANNELS] = {1, 1, 1, 1};
int pattern_offset[NUM_CHANNELS] = {0, 0, 0, 0};
byte gSeqNoteIndex[NUM_CHANNELS] = {0, 0, 0, 0};
int channelGatePin[NUM_CHANNELS] = {2, 3, 4, 5};

int clock_source;
#define CLK_SRC_INT      0
#define CLK_SRC_EXT      1

int curBPM = 120;
unsigned int gSeqT16 = 125;
unsigned long gSeqTimeCurrent;
unsigned long gSeqTimeLast;
unsigned long gSeqTimeGate;

#define PIN_SYNC_IN 10  // clock in pin

bool gSyncOn; // we are in note on phase
bool gSyncOff; // we are in note off phase (after gate)
bool gSyncNoteOn; // trigger note on handling
bool gSyncNoteOff; // trigger note off handling

int edit_mode = 0;
#define EDIT_MODE_PATTERN_LEN      0
#define EDIT_MODE_ACTIVE_BEATS     1
#define EDIT_MODE_OFFSET           2
#define EDIT_MODE_MEMORY           3

void setTempo(void) {
  gSeqT16 = 1000 / ((curBPM * 4) / ((float)60));
  gSeqTimeGate = gSeqT16 / 2;
}

//--------------- The Euclid Thingy --------------------
void offset_pattern(int chan, int offset) {
  if (offset == 0) {
    return;
  }
  int abs_off = abs(offset);
  int pLen = pattern_length[chan];

  for (int i = 0 ; i < abs_off ; i++) {
    if (offset < 0) {
      int keep = euclid_pattern[chan][0];
      for (int j = 0 ; j < pLen-1 ; j++) {
        euclid_pattern[chan][j] = euclid_pattern[chan][j+1] ;
      }
      euclid_pattern[chan][pLen-1] = keep;
    }
    else {
      int keep = euclid_pattern[chan][pLen-1];
      for (int j = pLen-1 ; j > 0 ; j--) {
        euclid_pattern[chan][j] = euclid_pattern[chan][j-1];
      }
      euclid_pattern[chan][0] = keep;
    }
  }
}

int euclid(int n, int k, int chan, int offset) {
    int spaces = n - k;
    float per_pulse = (float) n / (float) k;
    float per_space = (float) n / (float) spaces;
    int i;
    float pos = 0.0f;

    Serial.print("Euclid for: "); Serial.print(n); Serial.print(" / "); Serial.print(k); Serial.println();
    Serial.print("Per Pulse: "); Serial.print(per_pulse); Serial.println();
    Serial.print("Per Space: "); Serial.print(per_space); Serial.println();
 
    if (per_space > per_pulse) {
        for (i = 0 ; i < n ; i++) {     // empty array to 1
            euclid_pattern[chan][i] = 1;
        }
        for (i = 0 ; i < spaces ; i++) {
            int p = (int) pos;
            euclid_pattern[chan][p] = 0;
            pos = pos + per_space;
        }
    }
    else {
        for (i = 0 ; i < n ; i++) {     // empty array
            euclid_pattern[chan][i] = 0;
        }
        for (i = 0 ; i < k ; i++) {
          int p = (int) pos;
          euclid_pattern[chan][p] = 1;
          pos = pos + per_pulse;
        }
    }
    offset_pattern(chan, offset);
}

// the setup function runs once when you press reset or power the board
void setup() {

  Serial.begin(115200);

  euclid(16,1,0,0); // init euclid patterns for all channels
  euclid(16,2,1,0);
  euclid(16,4,2,0);
  euclid(16,8,3,0);

  gSeqTimeCurrent = millis();
  gSyncOn = false;
  gSyncOff = true;
  gSyncNoteOn = false;
  gSyncNoteOff = false;
  gSeqTimeLast = gSeqTimeCurrent - gSeqT16;
  for (int i = 0 ; i < NUM_CHANNELS; i++) {   // Gate output pins
    pinMode(channelGatePin[i], OUTPUT);
  }
  pinMode(PIN_SYNC_IN, INPUT_PULLUP);         // ext. clock pin

  RotaryEncoderSetup();
  RingDisplaySetup();
  PushButtonsSetup();
  TempoPotSetup();
  ClockToggleSetup();
  PatchStorageSetup();

}

// the loop function runs over and over again forever
void loop() {
  handleRotaryEncoder();
  handlePushButtons();
  handleTempoPot();
  handleClockToggle();
  displayLedRing();

  gSeqTimeCurrent = millis();

    if (clock_source == CLK_SRC_EXT) {

      // external clock

      // sync received for note on?
      if (digitalRead(PIN_SYNC_IN) == LOW) {
        if (!gSyncOn) {
          gSyncOn = true; // first part of sync pulse detected
          gSyncOff = false;
          gSyncNoteOn = true;
        }
      } else {
        if (!gSyncOff) {
          gSyncOn = false; // pulse ended
          gSyncOff = true;
          gSyncNoteOff = true;
        }
      }

    } else {

      // internal clock

      // time for note on?
      if (gSeqTimeCurrent - gSeqTimeLast >= gSeqT16)
      {
        if (!gSyncOn) {
          gSyncOn = true;
          gSyncOff = false;
          gSyncNoteOn = true;
        }
        gSeqTimeLast = gSeqTimeLast + gSeqT16; // update time for next note
      }

      // time for note off?
      if (gSeqTimeCurrent - gSeqTimeLast >= gSeqTimeGate)
      {
        if (!gSyncOff) {
          gSyncOn = false;
          gSyncOff = true;
          gSyncNoteOff = true;
        }
      }

    } // if (clock_source == SYNC_EXT)
  

  if (gSyncNoteOn) {
    gSyncNoteOn = false;
    for (int i = 0 ; i < NUM_CHANNELS; i++) {
      gSeqNoteIndex[i]++;
      if (gSeqNoteIndex[i] >= pattern_length[i]) {
        gSeqNoteIndex[i] = 0;
      }
      if (euclid_pattern[i][gSeqNoteIndex[i]]) {
        if (isPlaying) {
          digitalWrite(channelGatePin[i], HIGH);
        }
      }
      displayLedRing();
    }
  }

  // time for note off?
  if (gSyncNoteOff)
  {
     gSyncNoteOff = false;
     for (int i = 0 ; i < NUM_CHANNELS; i++) {
      if (isPlaying) {
        digitalWrite(channelGatePin[i], LOW);
      }        
     }
 }

}
