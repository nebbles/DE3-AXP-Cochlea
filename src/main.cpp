#include <Arduino.h>
#include <FastLED.h>  // Library for ws2801x led strips from http://FastLED.io
#define NUM_LEDS 240  // sets number of LEDs on strip
#define DATA_PIN 6    // digital out pin on the Arduino
#define totalBands 25 // sets the number of bands in message
CRGB leds[NUM_LEDS];  // create CRGB object of led strip

int time_delay = 500; // msec
float brightness = 0.5;
float vid_scale = brightness * 256.0;

int serialValue;
int bandNumber = 0;
int mode = 101;
int value;
int bandIntensities[totalBands] = {0};
int bandHues[totalBands] = {0};
int bandSaturations[totalBands] = {0};

int bandSize = 4; // first and last band need one added to it
int bandNum = 0;
int startStrip = 2; // index of first LED
int offset = 4;     // number of LEDs turned off between spirals

void updateBands() {
  // For each band, update the LEDs assigned to it with the latest Hue,
  // Saturation, Value

  int bandNum = 0;
  int LEDcount = 0;

  for (int LEDi = startStrip; LEDi < startStrip + 100; LEDi++) {

    // Serial.println(LEDi);
    // Serial.println(bandNum); // set band stuff
    leds[LEDi] = CHSV(bandHues[bandNum], bandSaturations[bandNum],
                      bandIntensities[bandNum]);

    LEDcount++;
    if (LEDcount == 4) {
      LEDcount = 0;
      bandNum++;
    }
  }

  bandNum = 24;
  LEDcount = 0;
  int reverseStrip = startStrip + 100 + offset;

  bandNum = 0;
  LEDcount = 0;
  for (int LEDi = reverseStrip + 100 - 1; LEDi > reverseStrip - 1; LEDi--) {

    // Serial.println(LEDi);
    // Serial.println(bandNum); // set band stuff
    leds[LEDi] = CHSV(bandHues[bandNum], bandSaturations[bandNum],
                      bandIntensities[bandNum]);

    LEDcount++;
    if (LEDcount == 4) {
      LEDcount = 0;
      bandNum++;
    }
  }
}

void setIntensity(int &band, int &intensity) {
  // Saves intensity as 0->100 converted 0->255 value in bandHue array.
  // Intensity must be an int percentage.
  bandIntensities[band] = map(intensity, 0, 100, 0, 255);
}

void setHue(int &band, int &hue) {
  // From hue % value, saves hue (0->255) in bandHues array.
  // (Note: hue value must be received as int percentage).
  bandHues[band] = map(hue, 0, 100, 0, 255);
}

void setSaturation(int &band, int &saturation) {
  // From hue % value, saves hue (0->255) in bandHues array.
  // (Note: hue value must be received as int percentage).
  bandSaturations[band] = map(saturation, 0, 100, 0, 255);
}

void setBandsBlack() {
  for (int i = 0; i < totalBands; i++) {
    bandIntensities[i] = 0; // set all intensity values to 0, black
  }
}
void setBandsWhite() {
  for (int i = 0; i < totalBands; i++) {
    bandSaturations[i] = 0;   // set all saturations to 0, white
    bandIntensities[i] = 255; // set all intensity values to 255, bright
  }
}

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  for (int i = 0; i < totalBands; i++) {
    bandHues[i] = 0;          // set all hues to 0, red by default
    bandSaturations[i] = 255; // set all saturations to 255, colour by default
    bandIntensities[i] = 0;   // set all intensities to 0, off by default
  }
  FastLED.show();
  Serial.begin(9600); // highest baud rate: 115200, standard: 9600
  while (!Serial) {
    ; // wait for Serial to establish
  }
  Serial.println("\nCochlea Model LED Control");
  Serial.println("v0.1");
  Serial.println("");
  Serial.println("March 2018");
  Serial.println("Ben Greenberg");
  Serial.println("github.com/nebbles");
  Serial.println("");
  Serial.println("---------------------");
  Serial.println("LED Strip: WS2812B");
  Serial.print("DATA_PIN: ");
  Serial.println(DATA_PIN);
  Serial.print("NUM of LEDs: ");
  Serial.println(NUM_LEDS);
  Serial.print("Total bands: ");
  Serial.println(totalBands);
  Serial.println("---------------------");
  Serial.println("101: All bands on/off");
  Serial.println("102: Set intensities");
  Serial.println("103: Set hues");
  Serial.println("104: Set saturations");
  Serial.println("---------------------");
}

void loop() {
  if (Serial.available() > 0) {
    serialValue = Serial.read(); // reads 1 value in msg at a time
    // Serial.print("Serial read: ");
    // Serial.println(serialValue); // debug the value

    // IF NUMBER RECEIVED > 100 THEN MODE RECEIVED AT START OF MESSAGE
    if (serialValue > 100) {
      switch (serialValue) {
      case 101:
        bandNumber = 0; // reset band number for upcoming values
        mode = 101;     // set mode
        break;
      case 102:
        bandNumber = 0;
        mode = 102;
        break;
      case 103:
        bandNumber = 0;
        mode = 103;
        break;
      case 104:
        bandNumber = 0;
        mode = 104;
        break;
      }
    } else {
      switch (mode) {
      case 101: // action for the special mode (controlling all bands at once)
        if (serialValue == 0) {
          setBandsBlack();
          updateBands();
          FastLED.show();
        } else if (serialValue == 1) {
          setBandsWhite();
          updateBands();
          FastLED.show();
        }
        break;
      case 102:
        setIntensity(bandNumber, serialValue); // set intensity of band X
        bandNumber++;                          // increment bandNumber
        break;

      case 103:
        setHue(bandNumber, serialValue); // set colour Hue of band X
        bandNumber++;                    // increment bandNumber
        break;

      case 104:
        setSaturation(bandNumber, serialValue); // set colour Hue of band X
        bandNumber++;                           // increment bandNumber
        break;
      }

      if (bandNumber == totalBands) {
        updateBands();  // update LEDs based on band values
        bandNumber = 0; // reset band number
        FastLED.show(); // refresh state of LED strip
      }
    }
  }
}
