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

bool isEEPROMInitialized() {
  return (EEPROM.read(MAGIC_OFF) == 0xAA && EEPROM.read(MAGIC_OFF+1) == 0xAA);
}

void initEEPROM() {
    for (int i = 0 ; i < EEPROM.length() ; i++) { // clear EEPROM
      EEPROM.write(i, 0);
    }
    EEPROM.write(MAGIC_OFF, 0xAA);
    EEPROM.write(MAGIC_OFF+1, 0xAA);
}

void PatchStorageSetup() {
  if (isEEPROMInitialized()) { // if was initialized, load patch 0
    loadCurrentPatchFromSlot(0); // if initialized, load patch 0 on startup
  }
  else {
    initEEPROM();    
  }
}

bool isSlotTaken(int slotNum) {
  if (slotNum > 15 || slotNum < 0) {
    return false;
  }
  if (slotNum < 8) {
    return bitRead(EEPROM.read(TAKEN_BITMAP_OFF), slotNum);
  }
  return bitRead(EEPROM.read(TAKEN_BITMAP_OFF+1), slotNum-8);
}

void setSlotTaken(int slotNum) {
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

void storeCurrentPatchToSlot(int slotNum) {
  if (slotNum < 0 || slotNum > 15) {
    return;
  }
  int writeOffset = PATCH0_OFF + slotNum * PATCH_LEN;
  byte val = 0x00;
  val = pattern_length[0] << 4 + pattern_nactive[0];    // CHANNEL A
  EEPROM.update(writeOffset++, val);
  val = pattern_offset[0];
  EEPROM.update(writeOffset++, val);
  val = pattern_length[1] << 4 + pattern_nactive[1];    // CHANNEL B
  EEPROM.update(writeOffset++, val);
  val = pattern_offset[1];
  EEPROM.update(writeOffset++, val);
  val = pattern_length[2] << 4 + pattern_nactive[2];    // CHANNEL C
  EEPROM.update(writeOffset++, val);
  val = pattern_offset[2];
  EEPROM.update(writeOffset++, val);
  val = pattern_length[3] << 4 + pattern_nactive[3];    // CHANNEL D
  EEPROM.update(writeOffset++, val);
  val = pattern_offset[3];
  EEPROM.update(writeOffset++, val);
  setSlotTaken(slotNum);
}

void loadCurrentPatchFromSlot(int slotNum) {
  if (slotNum < 0 || slotNum > 15 || !isSlotTaken(slotNum)) {
    return;
  }
  int readOffset = PATCH0_OFF + slotNum * PATCH_LEN;
  byte val = EEPROM.read(readOffset++);      // CHANNEL A
  pattern_length[0] = val & 0xf0 >> 4;
  pattern_nactive[0] = val & 0x0f;
  val = EEPROM.read(readOffset++);
  pattern_offset[0] = val & 0x0f;
  val = EEPROM.read(readOffset++);      // CHANNEL B
  pattern_length[1] = val & 0xf0 >> 4;
  pattern_nactive[1] = val & 0x0f;
  val = EEPROM.read(readOffset++);
  pattern_offset[1] = val & 0x0f;
  val = EEPROM.read(readOffset++);      // CHANNEL C
  pattern_length[2] = val & 0xf0 >> 4;
  pattern_nactive[2] = val & 0x0f;
  val = EEPROM.read(readOffset++);
  pattern_offset[2] = val & 0x0f;
  val = EEPROM.read(readOffset++);      // CHANNEL D
  pattern_length[3] = val & 0xf0 >> 4;
  pattern_nactive[3] = val & 0x0f;
  val = EEPROM.read(readOffset++);
  pattern_offset[3] = val & 0x0f;
  
}
