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
// File: RotaryEncoder.ino
// Functions to handle the rotary encoder + push button
//
// Author: Israel Roth
// Date: May 28, 2021
//
//---------------------------------------------------------

//--------------- Rotary Encoder Stuff --------------------
#define PIN_ENCODER_A      7
#define PIN_ENCODER_B      8
#define PIN_ENCODER_SWITCH 9
#define LONG_PRESS_TIME    800
static uint8_t enc_prev_pos   = 0;
static uint8_t enc_flags      = 0;
int lastState = LOW;
int currentState;
unsigned long pressedTime  = 0;
bool isPressing = false;
bool isLongDetected = false;

void RotaryEncoderSetup() {
  pinMode(PIN_ENCODER_A, INPUT_PULLUP);
  pinMode(PIN_ENCODER_B, INPUT_PULLUP);
  pinMode(PIN_ENCODER_SWITCH, INPUT_PULLUP);
  // get an initial reading on the encoder pins
  if (digitalRead(PIN_ENCODER_A) == LOW) {
    enc_prev_pos |= (1 << 0);
  }
  if (digitalRead(PIN_ENCODER_B) == LOW) {
    enc_prev_pos |= (1 << 1);
  }
}

void handleShortButtonPress() {
  Serial.println("Short Button Pressed");
  edit_mode++;
  if (edit_mode > EDIT_MODE_OFFSET) {
    edit_mode = EDIT_MODE_PATTERN_LEN;
  }
}

void handleLongButtonPress() {
  Serial.println("Long Button Pressed");
  edit_mode = EDIT_MODE_MEMORY;
  dumpEEPROM();
}

void handleRotaryInc() {
  Serial.println("Rotary Inc");
  bool anyChange = false;
  switch(edit_mode) {
    case EDIT_MODE_PATTERN_LEN:
      if (pattern_length[curChannel] > 1) {
        pattern_length[curChannel]--;
        if (pattern_nactive[curChannel] > pattern_length[curChannel]) {
          pattern_nactive[curChannel] = pattern_length[curChannel];
        }
        if (pattern_offset[curChannel] > pattern_length[curChannel]-1) {
          pattern_nactive[curChannel] = 0;
        }
        anyChange = true;        
      }
      break;      
    case EDIT_MODE_ACTIVE_BEATS:
      if(pattern_nactive[curChannel] < pattern_length[curChannel]) {
        pattern_nactive[curChannel]++;
        anyChange = true;
      }
      break;      
    case EDIT_MODE_OFFSET:
      pattern_offset[curChannel]--;
      if (pattern_offset[curChannel] < 0) {
        pattern_offset[curChannel] = pattern_length[curChannel] - 1;
      }
      anyChange = true;       
      break;
    case EDIT_MODE_MEMORY:
      if (cur_memory_pos > 0) {
        cur_memory_pos--;
      }
      else {
        cur_memory_pos = 15;
      }
      break;      
  }
  if (anyChange) {
    euclid(pattern_length[curChannel], pattern_nactive[curChannel], curChannel, pattern_offset[curChannel]);
  }
}

void handleRotaryDec() {
  Serial.println("Rotary Dec");
  bool anyChange = false;
  switch(edit_mode) {
    case EDIT_MODE_PATTERN_LEN:
      if (pattern_length[curChannel] < 16) {
        pattern_length[curChannel]++; 
        anyChange = true;       
      }
      break;      
    case EDIT_MODE_ACTIVE_BEATS:
      if(pattern_nactive[curChannel] > 0) {
        pattern_nactive[curChannel]--;
        anyChange = true;
      }
      break;      
    case EDIT_MODE_OFFSET:
      pattern_offset[curChannel]++; 
      if (pattern_offset[curChannel] >= (pattern_length[curChannel])) {
        pattern_offset[curChannel] = 0;        
      }
      anyChange = true;        
      break;      
    case EDIT_MODE_MEMORY:
      if (cur_memory_pos < 15) {
        cur_memory_pos++;
      }
      else {
        cur_memory_pos = 0;
      }
      break;      
  }
  if (anyChange) {
    euclid(pattern_length[curChannel], pattern_nactive[curChannel], curChannel, pattern_offset[curChannel]);
  }
}

void handleRotaryEncoder() {
  int8_t enc_action = 0; // 1 or -1 if moved, sign is direction
  uint8_t enc_cur_pos = 0;
  // read in the encoder state first
  if (digitalRead(PIN_ENCODER_A) == LOW) {
    enc_cur_pos |= (1 << 0);
  }
  if (digitalRead(PIN_ENCODER_B) == LOW) {
    enc_cur_pos |= (1 << 1);
  }

  // if any rotation at all
  if (enc_cur_pos != enc_prev_pos)
  {
    if (enc_prev_pos == 0x00)
    {
      // this is the first edge
      if (enc_cur_pos == 0x01) {
        enc_flags |= (1 << 0);
      }
      else if (enc_cur_pos == 0x02) {
        enc_flags |= (1 << 1);
      }
    }

    if (enc_cur_pos == 0x03)
    {
      // this is when the encoder is in the middle of a "step"
      enc_flags |= (1 << 4);
    }
    else if (enc_cur_pos == 0x00)
    {
      // this is the final edge
      if (enc_prev_pos == 0x02) {
        enc_flags |= (1 << 2);
      }
      else if (enc_prev_pos == 0x01) {
        enc_flags |= (1 << 3);
      }

      // check the first and last edge
      // or maybe one edge is missing, if missing then require the middle state
      // this will reject bounces and false movements
      if (bit_is_set(enc_flags, 0) && (bit_is_set(enc_flags, 2) || bit_is_set(enc_flags, 4))) {
        enc_action = 1;
      }
      else if (bit_is_set(enc_flags, 2) && (bit_is_set(enc_flags, 0) || bit_is_set(enc_flags, 4))) {
        enc_action = 1;
      }
      else if (bit_is_set(enc_flags, 1) && (bit_is_set(enc_flags, 3) || bit_is_set(enc_flags, 4))) {
        enc_action = -1;
      }
      else if (bit_is_set(enc_flags, 3) && (bit_is_set(enc_flags, 1) || bit_is_set(enc_flags, 4))) {
        enc_action = -1;
      }

      enc_flags = 0; // reset for next time
    }
  }

  enc_prev_pos = enc_cur_pos;

  if (enc_action > 0) {
    handleRotaryDec();
  }
  else if (enc_action < 0) {
    handleRotaryInc();
  }

  // remember that the switch is active low 
  currentState = digitalRead(PIN_ENCODER_SWITCH);
  if(lastState == HIGH && currentState == LOW) {        // button is pressed
    pressedTime = millis();
    isPressing = true;
    isLongDetected = false;
  } else if(lastState == LOW && currentState == HIGH) { // button is released
    isPressing = false;
    if (!isLongDetected) {
      handleShortButtonPress();      
    }
  }

  if(isPressing == true && isLongDetected == false) {
    long pressDuration = millis() - pressedTime;
    if( pressDuration > LONG_PRESS_TIME ) {
      isLongDetected = true;
      handleLongButtonPress();
    }
  }

  // save the the last state
  lastState = currentState;

}
