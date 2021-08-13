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

void dumpTwoBytes(int offset) {
  byte val = EEPROM.read(offset);
  Serial.print(val, HEX); Serial.print(" ");
  val = EEPROM.read(offset+1);
  Serial.print(val, HEX); Serial.println();
}

void dumpSlot(int slotNum) {
  Serial.print("Slot "); Serial.print(slotNum); Serial.print(": ");
  int readOffset = PATCH0_OFF + slotNum * PATCH_LEN;
  for (int i = 0 ; i < PATCH_LEN ; i++) {
    byte val = EEPROM.read(readOffset+i);
    Serial.print(val, HEX); Serial.print(" ");
  }
  Serial.println();
}

void dumpEEPROM() {
    Serial.println("----- EEPROM Dump -----");
    Serial.print("MAGIC: ");
    dumpTwoBytes(MAGIC_OFF);
    Serial.print("Taken: ");
    dumpTwoBytes(TAKEN_BITMAP_OFF);
    for (int i = 0 ; i < 16 ; i++) {
      dumpSlot(i);
    }
    for (int i = 0 ; i < 16 ; i++) {
      if (isSlotTaken(i)) {
        Serial.print("X ");
      }
      else {
        Serial.print("O ");
      }
    }
    Serial.println();
    Serial.println("----- EEPROM Dump -----");
}
void PatchStorageSetup() {
  if (!isEEPROMInitialized()) {
    initEEPROM();    
  }  
  dumpEEPROM();
}

bool isSlotTaken(int slotNum) {
  if (slotNum > 15 || slotNum < 0) {
    return false;
  }
  int takenBit;
  byte val;
  if (slotNum < 8) {
    val = EEPROM.read(TAKEN_BITMAP_OFF);
    takenBit = bitRead(val, slotNum);
  }
  else {
    val = EEPROM.read(TAKEN_BITMAP_OFF+1);
    takenBit = bitRead(val, slotNum-8);
  }
  return (takenBit != 0);
}

void printByte(byte val) {
  Serial.print(val,HEX); Serial.println();
}

void setSlotTaken(int slotNum) {
  Serial.print("setSlotTaken for slot: "); Serial.print(slotNum); Serial.println();
  if (slotNum > 15 || slotNum < 0) {
    return false;
  }
  if (slotNum < 8) {
    byte val = EEPROM.read(TAKEN_BITMAP_OFF);
    printByte(val);
    EEPROM.update(TAKEN_BITMAP_OFF, bitSet(val, slotNum));
    val = EEPROM.read(TAKEN_BITMAP_OFF);
    printByte(val);
  }
  else {
    byte val = EEPROM.read(TAKEN_BITMAP_OFF+1);
    printByte(val);
    EEPROM.update(TAKEN_BITMAP_OFF+1, bitSet(val, slotNum-8));
    val = EEPROM.read(TAKEN_BITMAP_OFF+1);
    printByte(val);
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
  Serial.print(val,HEX); Serial.println();
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

byte loadChan(int slotNum, int chan) {
  Serial.print("channel: "); Serial.print(chan);
  int readOffset = PATCH0_OFF + slotNum * PATCH_LEN + chan * CHAN_LEN;
  byte val = EEPROM.read(readOffset++);
  byte v3 = (val & 0xf0) >> 4;
  int len = 1 + v3;
  Serial.print(" Len = "); Serial.print(len);
  pattern_length[chan] = len;
  int nAct = 1 + (val & 0x0f);
  Serial.print(" nAct = "); Serial.print(nAct);
  pattern_nactive[chan] = nAct;
  val = EEPROM.read(readOffset++);
  Serial.print(" Off = "); Serial.print(val & 0x0f,HEX); Serial.println();
  pattern_offset[chan] = (val & 0x0f);
  euclid(pattern_length[chan], pattern_nactive[chan], chan, pattern_offset[chan]);
}

void loadCurrentPatchFromSlot(int slotNum) {
  if (slotNum < 0 || slotNum > 15 || !isSlotTaken(slotNum)) {
    Serial.print("BAD slot: "); Serial.print(slotNum); Serial.println();
    return;
  }
  Serial.print("loadCurrentPatchFromSlot for slot: "); Serial.print(slotNum); Serial.println();
  loadChan(slotNum, 0);   // Channel A
  loadChan(slotNum, 1);   // Channel B
  loadChan(slotNum, 2);   // Channel C
  loadChan(slotNum, 3);   // Channel D
}
