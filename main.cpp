#include <Adafruit_LiquidCrystal.h>

Adafruit_LiquidCrystal lcd(0);

byte arrowUp[8] = {
0b00100,
0b01110,
0b10101,
0b00100,
0b00100,
0b00100,
0b00100,
0b00000
};

byte arrowDown[8] = {
0b00100,
0b00100,
0b00100,
0b00100,
0b10101,
0b01110,
0b00100,
0b00000
};

// valve control declaration through relay
const int front_inflate = 8;
const int front_deflate = 9;
const int rear_inflate = 10;
const int rear_deflate = 11;
const int compressor = 12;

// switch declaration
const int compressor_switch = 2;
const int front_switch = 3;
const int rear_switch = 4;
const int lift_switch = 5;
const int lower_switch = 6;

// constants
const float VOLTS_PER_BAR = 0.01348;
const int DISPLAY_UPDATE_DELAY = 200;

// State variables
bool compressorState = false;
bool frontState = false;
bool rearState = false;
bool liftState = false;
bool lowerState = false;

// SETUP //
void setup() {
// Initialize relay pins
pinMode(front_inflate, OUTPUT);
pinMode(front_deflate, OUTPUT);
pinMode(rear_inflate, OUTPUT);
pinMode(rear_deflate, OUTPUT);
pinMode(compressor, OUTPUT);

// Initialize switch pins with pull-up resistors
pinMode(compressor_switch, INPUT_PULLUP);
pinMode(front_switch, INPUT_PULLUP);
pinMode(rear_switch, INPUT_PULLUP);
pinMode(lift_switch, INPUT_PULLUP);
pinMode(lower_switch, INPUT_PULLUP);

// Start with all relays off
digitalWrite(front_inflate, LOW);
digitalWrite(front_deflate, LOW);
digitalWrite(rear_inflate, LOW);
digitalWrite(rear_deflate, LOW);
digitalWrite(compressor, LOW);

lcd.begin(16, 2);
lcd.clear();
lcd.setCursor(3, 0);
lcd.print("Duo Ride");
lcd.setCursor(1, 1);
lcd.print("Modol Air Ride");
delay(2000);
lcd.clear();
}

void loop() {
lcd.createChar(0, arrowUp);     // store as index 0
lcd.createChar(1, arrowDown);   // store as index 1
// Read current switch states (LOW = ON, HIGH = OFF due to pull-up)
compressorState = (digitalRead(compressor_switch) == LOW);
frontState = (digitalRead(front_switch) == LOW);
rearState = (digitalRead(rear_switch) == LOW);
liftState = (digitalRead(lift_switch) == LOW);
lowerState = (digitalRead(lower_switch) == LOW);

// Activate only the needed relays based on switch combinations
if (liftState && frontState && rearState) {
  // INFLATE ALL
  digitalWrite(front_inflate, HIGH);
  digitalWrite(rear_inflate, HIGH);
} else if (lowerState && frontState && rearState) {
  // DEFLATE ALL
  digitalWrite(front_deflate, HIGH);
  digitalWrite(rear_deflate, HIGH);
} else if (liftState && frontState) {
  // LIFT FRONT ONLY
  digitalWrite(front_inflate, HIGH);
} else if (liftState && rearState) {
  // LIFT REAR ONLY
  digitalWrite(rear_inflate, HIGH);
} else if (lowerState && frontState) {
  // LOWER FRONT ONLY
  digitalWrite(front_deflate, HIGH);
} else if (lowerState && rearState) {
  // LOWER REAR ONLY
  digitalWrite(rear_deflate, HIGH);
} else {
  // DEFAULT: All relays remain LOW (off)
  // No need to explicitly set them LOW again
}

// Read and display sensors
int front_sensor = analogRead(A0);
int rear_sensor = analogRead(A1);
int storage_sensor = analogRead(A2);

float front_bar = front_sensor * VOLTS_PER_BAR;
float rear_bar = rear_sensor * VOLTS_PER_BAR;
float storage_bar = storage_sensor * VOLTS_PER_BAR;

if (compressor_switch && storage_sensor < 11.0){
  digitalWrite(compressor, HIGH);
}

if (compressor_switch && storage_bar > 11.5){
  digitalWrite(compressor, LOW);
}

// Display pressures
lcd.setCursor(0, 0);
lcd.print("F:");
lcd.print(front_bar, 1);
lcd.print("b ");

lcd.setCursor(8, 0);
lcd.print("S:");
lcd.print(storage_bar, 1);
lcd.print("b");

lcd.setCursor(0, 1);
lcd.print("R:");
lcd.print(rear_bar, 1);
lcd.print("b ");

// Display active modes
lcd.setCursor(8, 1);
if (!liftState && !lowerState && !frontState && !rearState) {
  lcd.print("     ");
} else if (!frontState && !rearState && lowerState && liftState) {
  lcd.print("WARNING!");
} else if (!frontState && rearState && lowerState && liftState) {
  lcd.print("WARNING!");
} else if (frontState && !rearState && lowerState && liftState) {
  lcd.print("WARNING!");
} else if (frontState && rearState && lowerState && liftState) {
  lcd.print("WARNING!");
} else if (liftState && !lowerState && frontState && rearState) {
  lcd.print("P:");
  lcd.write(byte(0));
  lcd.print("T:");
  lcd.write(byte(0));
  lcd.print("  ");
} else if (!liftState && lowerState && frontState && rearState) {
  lcd.print("P:");
  lcd.write(byte(1));
  lcd.print("T:");
  lcd.write(byte(1));
  lcd.print("  ");
} else if (liftState && !lowerState && frontState && !rearState) {
  lcd.print("P:");
  lcd.write(byte(0));
  lcd.print("   ");
  lcd.print("  ");
} else if (liftState && !lowerState && !frontState && rearState) {
  lcd.print("  ");
  lcd.print("T:");
  lcd.write(byte(0));
} else if (!liftState && lowerState && !frontState && rearState) {
  lcd.print("   ");
  lcd.print("T:");
  lcd.write(byte(1));
  lcd.print("  ");
} else if (!liftState && lowerState && frontState && !rearState) {
  lcd.print("P:");
  lcd.write(byte(1));
  lcd.print("   ");
  lcd.print("  ");
} else if (!liftState && lowerState && !frontState && !rearState) {
  lcd.print("Lower    ");
} else if (liftState && !lowerState && !frontState && !rearState) {
  lcd.print("Lift    ");

}
delay(DISPLAY_UPDATE_DELAY);
}