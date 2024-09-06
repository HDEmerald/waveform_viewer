// Hunter Drake, Dec. 2023
// Pin A2 is where the analog signal is read from.
// The upper bound of voltage readings is ~3.3V and ~GND is the lower bound.
// Audio library: https://github.com/pschatzmann/arduino-audio-tools

// Audio Includes
#include "Arduino.h"
#include "AudioTools.h"

// Graphics Libraries and such
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

// Audio Defines
#define SAMPLE_RATE 48000
#define NUM_CHANNELS 1
#define BITS_PER_SAMPLE 16 // 12 doesn't work, but serial plotter seems to only see 12 of the 16 bits so it is good

// Defines for Adafruit HUZZAH32 Feather Board/1.44" Display
#define TFT_CS         14
#define TFT_RST        15
#define TFT_DC         32

// For 1.44" and 1.8" TFT with ST7735 use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
#define SCREEN_W 128 // Screen width in pixels
#define SCREEN_H 128 // Screen height in pixels

// Audio Globals
AnalogAudioStream in; 
AudioInfo info(SAMPLE_RATE, NUM_CHANNELS, BITS_PER_SAMPLE);
CsvOutput<int16_t> out(Serial); // ASCII output stream 
StreamCopy copier(out, in); 
int16_t buffer[512];
int sample_size = BITS_PER_SAMPLE / 8;

// Globals for graphics
int sizeBuckets = 4096 / SCREEN_H; // 4096 is the number of possible measured ADC values

// Functions Declaration
void clearTFTDisplay(Adafruit_ST7735 tft);

void setup() {
  // Serial begin
  Serial.begin(115200);
  
  // Audio Setup
  // Default pin to read analog signal from is A2 of the HUZZAH32 board
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);
  auto cfgRx = in.defaultConfig(RX_MODE);
  //cfgRx.start_pin = A1; // optinally define pin
  cfgRx.is_auto_center_read = false; // Configures the ADC such that it doesn't center based on the initial voltage seen on the input
  cfgRx.copyFrom(info);
  in.begin(cfgRx);
  // open output
  out.begin(info);

  // LCD Setup
  // Init the Screen's driver chip
  tft.initR(INITR_144GREENTAB); // Init ST7735R chip, green tab
  // Set the Screen to black and draw the horizontal axis 
  tft.fillScreen(ST77XX_BLACK);
  tft.drawFastHLine(0, (SCREEN_H / 2), SCREEN_W, ST77XX_WHITE);
}

void loop() {
  // Read Audio Data
  size_t eff_samples = in.readBytes((uint8_t*)buffer, 512 * sample_size) / sample_size;

  // Clear pixels
  clearTFTDisplay(tft);

  // Display pixel contents of buffer
  for (int i = 0; i <= 511; i = i + 4) {
    tft.drawPixel(i / 4, buffer[i] / sizeBuckets, ST77XX_WHITE);
  }

  // Delay for each "frame"
  // NOTE: I wouldn't go less than 20ms between frames as there seems to be some weird grey pixel thing that happens.
  // Also jitter is minimized and 20ms delay isn't too terrible
  delay(20);

  // Display audio data on Serial Monitor/Serial Plotter
  //copier.copy();
}

void clearTFTDisplay(Adafruit_ST7735 tft) {
  // Uses this and not tft.fillScreen(ST77XX_BLACK) to save time clearing horizontal axis
  tft.fillRect(0, 0, SCREEN_W, (SCREEN_H / 2), ST77XX_BLACK);
  tft.fillRect(0, (SCREEN_H / 2) + 1, SCREEN_W, (SCREEN_H / 2) - 1, ST77XX_BLACK);
}
