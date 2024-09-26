#include <OctoWS2811.h>

// Easy to adjust parameters
#define COLS_LEDs 50  // Number of LEDs in each strip
#define ROWS_LEDs 8   // Number of LED strips

#define ANIMATION_SPEED 30     // Delay between frames in milliseconds (higher = slower)
#define MAX_BRIGHTNESS 192     // Maximum regular brightness (0-255)
#define POP_FREQUENCY 2        // Frequency of bright "pops" (higher = more frequent, max 1000)
#define POP_BRIGHTNESS 255     // Brightness of "pops" (0-255)
#define COLOR_FADE_RATE 20     // Rate of color fading (higher = faster, max 100)
#define PLASMA_SPEED 2         // Speed of plasma movement (higher = faster)

// Wave effect parameters
#define WAVE_INTERVAL 1000     // Average frames between wave effects (higher = less frequent)
#define WAVE_DURATION 150      // Duration of wave effect in frames
#define WAVE_SPEED 2           // Speed of wave movement (higher = faster)
#define WAVE_WIDTH 20          // Width of the wave in pixels
#define WAVE_BRIGHTNESS 255    // Brightness of the main wave (0-255)
#define NUM_ECHOES 3           // Number of echo waves
#define ECHO_SPACING 15        // Spacing between echo waves
#define ECHO_DECAY 64          // How much each echo diminishes (higher = faster decay)
#define SHUDDER_STRENGTH 3     // Strength of the shudder effect (higher = stronger)
#define SHUDDER_FREQUENCY 5    // Frequency of shudders (higher = more frequent)

// OctoWS2811 configuration
#define LEDS_PER_STRIP (COLS_LEDs * (ROWS_LEDs / 8))
DMAMEM int displayMemory[LEDS_PER_STRIP*6];
int drawingMemory[LEDS_PER_STRIP*6];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(LEDS_PER_STRIP, displayMemory, drawingMemory, config);

uint8_t const cos_wave[256] PROGMEM =  
{0,0,0,0,1,1,1,2,2,3,4,5,6,6,8,9,10,11,12,14,15,17,18,20,22,23,25,27,29,31,33,35,38,40,42,
45,47,49,52,54,57,60,62,65,68,71,73,76,79,82,85,88,91,94,97,100,103,106,109,113,116,119,
122,125,128,131,135,138,141,144,147,150,153,156,159,162,165,168,171,174,177,180,183,186,
189,191,194,197,199,202,204,207,209,212,214,216,218,221,223,225,227,229,231,232,234,236,
238,239,241,242,243,245,246,247,248,249,250,251,252,252,253,253,254,254,255,255,255,255,
255,255,255,255,254,254,253,253,252,252,251,250,249,248,247,246,245,243,242,241,239,238,
236,234,232,231,229,227,225,223,221,218,216,214,212,209,207,204,202,199,197,194,191,189,
186,183,180,177,174,171,168,165,162,159,156,153,150,147,144,141,138,135,131,128,125,122,
119,116,113,109,106,103,100,97,94,91,88,85,82,79,76,73,71,68,65,62,60,57,54,52,49,47,45,
42,40,38,35,33,31,29,27,25,23,22,20,18,17,15,14,12,11,10,9,8,6,6,5,4,3,2,2,1,1,1,0,0,0,0
};

uint8_t const exp_gamma[256] PROGMEM =
{0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,3,3,3,3,3,
4,4,4,4,4,5,5,5,5,5,6,6,6,7,7,7,7,8,8,8,9,9,9,10,10,10,11,11,12,12,12,13,13,14,14,14,15,15,
16,16,17,17,18,18,19,19,20,20,21,21,22,23,23,24,24,25,26,26,27,28,28,29,30,30,31,32,32,33,
34,35,35,36,37,38,39,39,40,41,42,43,44,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,
61,62,63,64,65,66,67,68,70,71,72,73,74,75,77,78,79,80,82,83,84,85,87,89,91,92,93,95,96,98,
99,100,101,102,105,106,108,109,111,112,114,115,117,118,120,121,123,125,126,128,130,131,133,
135,136,138,140,142,143,145,147,149,151,152,154,156,158,160,162,164,165,167,169,171,173,175,
177,179,181,183,185,187,190,192,194,196,198,200,202,204,207,209,211,213,216,218,220,222,225,
227,229,232,234,236,239,241,244,246,249,251,253,254,255
};


// Array to store current LED states
uint32_t ledStates[COLS_LEDs * ROWS_LEDs];

// Wave effect variables
int wavePosition = -WAVE_WIDTH;
int waveFramesLeft = 0;
unsigned long lastWaveStart = 0;

void setup() {
  pinMode(13, OUTPUT);
  leds.begin();
  leds.show();
  
  // Initialize LED states
  for (int i = 0; i < COLS_LEDs * ROWS_LEDs; i++) {
    ledStates[i] = 0;
  }
}

void loop() {
  static unsigned long frameCount = 25500;
  frameCount++;
  
  uint16_t t = fastCosineCalc((21 * frameCount * PLASMA_SPEED) / 100);
  uint16_t t2 = fastCosineCalc((17 * frameCount * PLASMA_SPEED) / 100);
  uint16_t t3 = fastCosineCalc((19 * frameCount * PLASMA_SPEED) / 100);

  // Check if it's time to start a new wave
  if (waveFramesLeft <= 0 && random(WAVE_INTERVAL) == 0) {
    wavePosition = -WAVE_WIDTH;
    waveFramesLeft = WAVE_DURATION;
    lastWaveStart = frameCount;
  }

  for (uint8_t y = 0; y < ROWS_LEDs; y++) {
    int left2Right, pixelIndex;
    if (((y % (ROWS_LEDs/8)) & 1) == 0) {
      left2Right = 1;
      pixelIndex = y * COLS_LEDs;
    } else {
      left2Right = -1;
      pixelIndex = (y + 1) * COLS_LEDs - 1;
    }
    
    for (uint8_t x = 0; x < COLS_LEDs; x++) {
      // Calculate base plasma values
      uint8_t r = fastCosineCalc(((x << 3) + (t >> 1) + fastCosineCalc((t2 + (y << 3)))));
      uint8_t g = fastCosineCalc(((y << 3) + t + fastCosineCalc(((t3 >> 2) + (x << 3)))));
      uint8_t b = fastCosineCalc(((y << 3) + t2 + fastCosineCalc((t + x + (g >> 2)))));

      // Apply smoother twinkling effect
      uint8_t twinkle = fastCosineCalc((x * 7 + y * 11 + frameCount) * 3) >> 1;
      uint8_t intensity = map(twinkle, 0, 255, 0, MAX_BRIGHTNESS);

      // Occasional "pop" effect
      if (random(1000) < POP_FREQUENCY) {
        intensity = POP_BRIGHTNESS;
      }

      // Apply wave effect if active
      if (waveFramesLeft > 0) {
        // Main wave
        int distFromWave = abs(x - wavePosition);
        if (distFromWave < WAVE_WIDTH) {
          uint8_t waveIntensity = map(distFromWave, 0, WAVE_WIDTH, WAVE_BRIGHTNESS, 0);
          intensity = max(intensity, waveIntensity);
        }
        
        // Echo waves
        for (int i = 1; i <= NUM_ECHOES; i++) {
          int echoPos = wavePosition - i * ECHO_SPACING;
          distFromWave = abs(x - echoPos);
          if (distFromWave < WAVE_WIDTH) {
            uint8_t echoIntensity = map(distFromWave, 0, WAVE_WIDTH, WAVE_BRIGHTNESS >> i, 0);
            echoIntensity = max(0, echoIntensity - (i * ECHO_DECAY));
            intensity = max(intensity, echoIntensity);
          }
        }
        
        // Shudder effect
        if (random(100) < SHUDDER_FREQUENCY) {
          intensity += random(-SHUDDER_STRENGTH, SHUDDER_STRENGTH + 1);
        }
      }

      intensity = constrain(intensity, 0, 255);  // Ensure intensity stays within valid range

      r = (r * intensity) >> 8;
      g = (g * intensity) >> 8;
      b = (b * intensity) >> 8;

      // Apply gamma correction
      r = pgm_read_byte_near(exp_gamma + r);
      g = pgm_read_byte_near(exp_gamma + g);
      b = pgm_read_byte_near(exp_gamma + b);

      // Smooth transition to new color
      uint32_t newColor = (r << 16) | (g << 8) | b;
      uint32_t oldColor = ledStates[pixelIndex];
      uint32_t smoothColor = smoothTransition(oldColor, newColor, COLOR_FADE_RATE);

      ledStates[pixelIndex] = smoothColor;
      leds.setPixel(pixelIndex, smoothColor);
      pixelIndex += left2Right;
    }
  }

  // Update wave position
  if (waveFramesLeft > 0) {
    wavePosition += WAVE_SPEED;
    waveFramesLeft--;
    if (wavePosition > COLS_LEDs + WAVE_WIDTH + (NUM_ECHOES * ECHO_SPACING)) {
      waveFramesLeft = 0;
    }
  }

  digitalWrite(13, HIGH);
  leds.show();
  digitalWrite(13, LOW);
  delay(ANIMATION_SPEED);
}

inline uint8_t fastCosineCalc(uint16_t preWrapVal) {
  uint8_t wrapVal = (preWrapVal % 255);
  if (wrapVal < 0) wrapVal = 255 + wrapVal;
  return (pgm_read_byte_near(cos_wave + wrapVal));
}

// Function to smoothly transition between colors
uint32_t smoothTransition(uint32_t oldColor, uint32_t newColor, uint8_t amount) {
  uint8_t oldR = (oldColor >> 16) & 0xFF;
  uint8_t oldG = (oldColor >> 8) & 0xFF;
  uint8_t oldB = oldColor & 0xFF;
  
  uint8_t newR = (newColor >> 16) & 0xFF;
  uint8_t newG = (newColor >> 8) & 0xFF;
  uint8_t newB = newColor & 0xFF;
  
  uint8_t r = oldR + ((newR - oldR) * amount / 100);
  uint8_t g = oldG + ((newG - oldG) * amount / 100);
  uint8_t b = oldB + ((newB - oldB) * amount / 100);
  
  return (r << 16) | (g << 8) | b;
}
