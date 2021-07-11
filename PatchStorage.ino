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
// File: PatchStorage.ino
// Functions to handle storing and reading patches
// from EEPROM
//
// Author: Israel Roth
// Date: May 29, 2021
//
//---------------------------------------------------------

#include <EEPROM.h>
#define MAGIC_OFF            0   // offset to magic byte to see if we already programmed the EEPROM
#define TAKEN_BITMAP_OFF     2   // offset to bitmap marking which patches were stored to
#define PATCH0_OFF           4   // start of patch storage area
#define PATCH_LEN            8   // length in bytes of each patch
#define CHAN_LEN             2   // length in bytes of each channel in a patch

bool isEEPROMInitialized() {
  bool isInitialized = (EEPROM.read(MAGIC_OFF) == 0xAA && EEPROM.read(MAGIC_OFF+1) == 0xAA);
  if (isInitialized) {
    Serial.println("EEPROM Initialized");
  }
  return isInitialized;
}

void initEEPROM() {
    Serial.println("EEPROM Being Initialized");
    for (int i = 0 ; i < EEPROM.length() ; i++) { // clear EEPROM
      EEPROM.write(i, 0);
    }
    EEPROM.write(MAGIC_OFF, 0xAA);
    EEPROM.write(MAGIC_OFF+1, 0xAA);
}

void PatchStorageSetup() {
  if (!isEEPROMInitialized()) {
    initEEPROM();    
  }
}

bool isSlotTaken(int slotNum) {
  if (slotNum > 15 || slotNum < 0) {
    return false;
  }
  int takenBit;
  if (slotNum < 8) {
    takenBit = bitRead(EEPROM.read(TAKEN_BITMAP_OFF), slotNum);
  }
  takenBit = bitRead(EEPROM.read(TAKEN_BITMAP_OFF+1), slotNum-8);
  return (takenBit != 0);
}

void printByte(byte val) {
  Serial.print(val,BIN); Serial.println();
}

void setSlotTaken(int slotNum) {
  Serial.print("setSlotTaken for slot: "); Serial.print(slotNum); Serial.println();
  if (slotNum > 15 || slotNum < 0) {
    return false;
  }
  if (slotNum < 8) {
    byte val = EEPROM.read(TAKEN_BITMAP_OFF);
    EEPROM.update(TAKEN_BITMAP_OFF, bitSet(val, slotNum));
  }
  else {
    byte val = EEPROM.read(TAKEN_BITMAP_OFF+1);
    EEPROM.update(TAKEN_BITMAP_OFF+1, bitSet(val, slotNum-8));
  }
}

byte storeChan(int slotNum, int chan) {
  int writeOffset = PATCH0_OFF + slotNum * PATCH_LEN + chan * CHAN_LEN;
  Serial.print("storeChan at offset: "); Serial.print(writeOffset); Serial.println();
  byte val = 0x00;
  val = ((pattern_length[chan]-1) << 4) + (pattern_nactive[chan] - 1);    // CHANNEL A
  printByte(val);
  EEPROM.update(writeOffset++, val);
  val = pattern_offset[chan];
  Serial.print(val,BIN); Serial.println();
  EEPROM.update(writeOffset++, val);
}

void storeCurrentPatchToSlot(int slotNum) {
  Serial.print("storeCurrentPatchToSlot for slot: "); Serial.print(slotNum); Serial.println();
  if (slotNum < 0 || slotNum > 15) {
    return;
  }
  storeChan(slotNum, 0);   // Channel A
  storeChan(slotNum, 1);   // Channel B
  storeChan(slotNum, 2);   // Channel C
  storeChan(slotNum, 3);   // Channel D
  setSlotTaken(slotNum);
}

void loadCurrentPatchFromSlot(int slotNum) {
  Serial.print("loadCurrentPatchFromSlot for slot: "); Serial.print(slotNum); Serial.println();
  if (slotNum < 0 || slotNum > 15 || !isSlotTaken(slotNum)) {
    return;
  }
  int readOffset = PATCH0_OFF + slotNum * PATCH_LEN;
  byte val = EEPROM.read(readOffset++);      // CHANNEL A
  Serial.print(val,BIN); Serial.println();
  pattern_length[0] = val & 0xf0 >> 4;
  pattern_nactive[0] = val & 0x0f;
  val = EEPROM.read(readOffset++);
  Serial.print(val,BIN); Serial.println();
  pattern_offset[0] = val & 0x0f;
  val = EEPROM.read(readOffset++);      // CHANNEL B
  Serial.print(val,BIN); Serial.println();
  pattern_length[1] = val & 0xf0 >> 4;
  pattern_nactive[1] = val & 0x0f;
  val = EEPROM.read(readOffset++);
  Serial.print(val,BIN); Serial.println();
  pattern_offset[1] = val & 0x0f;
  val = EEPROM.read(readOffset++);      // CHANNEL C
  Serial.print(val,BIN); Serial.println();
  pattern_length[2] = val & 0xf0 >> 4;
  pattern_nactive[2] = val & 0x0f;
  val = EEPROM.read(readOffset++);
  Serial.print(val,BIN); Serial.println();
  pattern_offset[2] = val & 0x0f;
  val = EEPROM.read(readOffset++);      // CHANNEL D
  Serial.print(val,BIN); Serial.println();
  pattern_length[3] = val & 0xf0 >> 4;
  pattern_nactive[3] = val & 0x0f;
  val = EEPROM.read(readOffset++);
  Serial.print(val,BIN); Serial.println();
  pattern_offset[3] = val & 0x0f; 
}
