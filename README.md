# Neno Euclid
## _Arduino-based Euclid Rhythm Sequencer_

### Features
- 4 Channels with 4 gate outputs
- Each channel has 16 steps pattern
- NeoPixels ring is used as main UI elelemt
- Rotary encoder to control various parameters of the pattern
    - Pattern length 1-16 notes
    - Number of "ON" slots in the pattern
    - Pattern start offset
- Each channel can be independently edited, and the 4 channels combination is a "patch"
- You can save and later load up to 16 patches in EEPROM (kept even with power off)
- The unit has internal clock with controlled rythm, but can also be switched to use external eurorack clock signal

### To build the software
The following library is needed:
Adafruit_NeoPixel for the NeoPixels ring display [Available on their Github page](https://github.com/adafruit/Adafruit_NeoPixel)

### Hardware
Check the Docs sub-folder for both the user manual as well some information on hardware and the circuit used
