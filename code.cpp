#include <Wire.h> 
#include <RCSwitch.h> 
#include <Adafruit_GFX.h>  
#include <Adafruit_SSD1306.h> 

#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 32  
#define OLED_ADDR 0x3C  

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);  

RCSwitch mySwitch = RCSwitch(); 
   
// Define your remote codes here 
const long RF_COMPRESSOR_TOGGLE = 14637857; // ← NOW USED AS EMERGENCY PAUSE / RESUME
const long RF_PRESET1 = 11789684; 
const long RF_PRESET2 = 2673720; 
const long RF_PRESET3 = 2673714; 

// PININ
const int sensor_fornt = A0; // front axle
const int sensor_rear = A1; // rear axle
const int sensor_storage = A2; // storage tank

// PINOUT 
const int compressor = 8;
const int Front_up = 9; // inflate front (solenoid A)
const int Rear_up = 10; // inflate rear
const int Front_down = 11; // deflate front (solenoid B)
const int Rear_down = 12; // deflate rear

// constants
const float VOLTS_PER_BAR = 0.01348; // your sensor constant (pressure in BAR)

// ================================================
// NON-BLOCKING PRESET STATE (for emergency pause/resume)
// ================================================
bool presetInProgress = false; // is a preset currently active?
bool presetPaused = false; // emergency pause flag
float presetTargetFront = 0.0;
float presetTargetRear = 0.0;
bool frontDone = false;
bool rearDone = false;
unsigned long presetStartTime = 0;
unsigned long lastPresetStepTime = 0;

const float TOLERANCE = 0.5; // ±0.5 bar tolerance
const float TANK_MARGIN = 2.0; // tank must be this much higher than target
const unsigned long PRESET_TIMEOUT = 60000UL; // 60 s safety timeout

// ================================================
// PRESSURE READING (BAR)
// ================================================
float getFrontPressure() {
  int raw = analogRead(sensor_fornt);
  float volts = raw * (5.0 / 1023.0);
  return volts / VOLTS_PER_BAR;
}

float getRearPressure() {
  int raw = analogRead(sensor_rear);
  float volts = raw * (5.0 / 1023.0);
  return volts / VOLTS_PER_BAR;
}

float getTankPressure() {
  int raw = analogRead(sensor_storage);
  float volts = raw * (5.0 / 1023.0);
  return volts / VOLTS_PER_BAR;
}

// ================================================
// VALVE & COMPRESSOR HELPERS
// ================================================
void setFrontValve(int state) { // 1 = inflate, -1 = deflate, 0 = hold
  digitalWrite(Front_up, (state == 1) ? HIGH : LOW);
  digitalWrite(Front_down, (state == -1) ? HIGH : LOW);
}

void setRearValve(int state) {
  digitalWrite(Rear_up, (state == 1) ? HIGH : LOW);
  digitalWrite(Rear_down, (state == -1) ? HIGH : LOW);
}

void setCompressor(bool on) {
  digitalWrite(compressor, on ? HIGH : LOW);
}

// ================================================
// START A PRESET (non-blocking)
// ================================================
void startPreset(float targetF, float targetR) {
  presetTargetFront = targetF;
  presetTargetRear = targetR;
  presetInProgress = true;
  presetPaused = false;
  frontDone = false;
  rearDone = false;
  presetStartTime = millis();
  lastPresetStepTime = millis();

  Serial.println("=== PRESET START ===");
  Serial.print("Target Front: "); Serial.print(targetF);
  Serial.print(" bar | Target Rear: "); Serial.print(targetR);
  Serial.println(" bar");
}

// ================================================
// STOP PRESET (cleanup)
// ================================================
void stopPreset(const char* status) {
  setFrontValve(0);
  setRearValve(0);
  setCompressor(false);
  presetInProgress = false;
  presetPaused = false;

  Serial.print("PRESET "); Serial.println(status);

  // Update OLED
  display.clearDisplay();  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,10);
  display.print("PRESET ");
  display.println(status);
  display.display();
}

// ================================================
// ONE STEP OF THE PRESET (called from loop)
// ================================================
void doPresetStep() {
  if (!presetInProgress || presetPaused) return;

  unsigned long now = millis();
  if (now - presetStartTime >= PRESET_TIMEOUT) {
    stopPreset("TIMEOUT!");
    return;
  }

  float pTank = getTankPressure();
  float pFront = getFrontPressure();
  float pRear = getRearPressure();

  bool compressorNeeded = false;

  // ----- FRONT -----
  if (!frontDone) {
    if (pFront < presetTargetFront - TOLERANCE) {
      if (pTank < presetTargetFront + TANK_MARGIN) compressorNeeded = true;
      setFrontValve(1);
    }
    else if (pFront > presetTargetFront + TOLERANCE) {
      setFrontValve(-1);
    }
    else {
      setFrontValve(0);
      frontDone = true;
    }
  } else {
    setFrontValve(0);
  }

  // ----- REAR -----
  if (!rearDone) {
    if (pRear < presetTargetRear - TOLERANCE) {
      if (pTank < presetTargetRear + TANK_MARGIN) compressorNeeded = true;
      setRearValve(1);
    }
    else if (pRear > presetTargetRear + TOLERANCE) {
      setRearValve(-1);
    }
    else {
      setRearValve(0);
      rearDone = true;
    }
  } else {
    setRearValve(0);
  }

  setCompressor(compressorNeeded);

  // Check if both axles are done
  if (frontDone && rearDone) {
    stopPreset("COMPLETE!");
  }
}

void setup() { 
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {  
    for(;;); // stop if display not found  
  } 

  Serial.begin(9600); // for debugging

  // Receiver on pin 2
  mySwitch.enableReceive(digitalPinToInterrupt(2));

  // Set all outputs
  pinMode(compressor, OUTPUT);
  pinMode(Front_up, OUTPUT);
  pinMode(Rear_up, OUTPUT);
  pinMode(Front_down, OUTPUT);
  pinMode(Rear_down, OUTPUT);

  // Everything OFF / HOLD at startup
  digitalWrite(compressor, LOW);
  digitalWrite(Front_up, LOW);
  digitalWrite(Rear_up, LOW);
  digitalWrite(Front_down, LOW);
  digitalWrite(Rear_down, LOW);

  Serial.println("Air Ride ready - Emergency button ready!");
}

void loop() {
  if (mySwitch.available()) {
    long receivedValue = mySwitch.getReceivedValue();

    display.clearDisplay();  
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,10);

    switch(receivedValue) { 

      // ========================
      // EMERGENCY STOP / RESUME
      // ========================
      case RF_COMPRESSOR_TOGGLE: 
        if (presetInProgress) {
          presetPaused = !presetPaused;

          if (presetPaused) {
            // EMERGENCY STOP
            setFrontValve(0);
            setRearValve(0);
            setCompressor(false);
            display.println("EMERGENCY PAUSED");
            Serial.println("PRESET PAUSED (emergency)");
          } else {
            // RESUME
            display.println("PRESET RESUMED");
            Serial.println("PRESET RESUMED");
          }
          display.display();
        } else {
          // no preset running
          display.println("No preset running");
          display.display();
        }
        break;

      // ========================
      // PRESETS (start non-blocking)
      // ========================
      case RF_PRESET1:
        display.println("Starting PRESET1 (0/0 bar)");
        display.display();
        startPreset(0.0, 0.0);
        break;

      case RF_PRESET2:
        display.println("Starting PRESET2 (6/6 bar)");
        display.display();
        startPreset(6.0, 6.0);
        break;

      case RF_PRESET3: 
        display.println("Starting PRESET3 (11/11 bar)");
        display.display();
        startPreset(11.0, 11.0);
        break;
    } 

    mySwitch.resetAvailable(); 
  }

  // ================================================
  // Run preset step every \~200 ms (non-blocking)
  // ================================================
  if (presetInProgress && !presetPaused && (millis() - lastPresetStepTime >= 200)) {
    doPresetStep();
    lastPresetStepTime = millis();
  }
}
