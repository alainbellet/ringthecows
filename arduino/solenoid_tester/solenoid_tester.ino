/*---------------------------------------------------------------------------------------------
  Ring The cows project
  Firmware
  Alain Bellet 2016 / 2025

  V9
  --------------------------------------------------------------------------------------------- */

const int RED_LED_PIN = 0;
const int SOLENOID_PIN = 15;
unsigned long milliseconds;
unsigned long loopcount = 0;
unsigned long seconds;

void triggerBell();

// ## SETUP
void setup() {
  delay(100);

  // Manage LEDS and GPIO
  // Red LED
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, HIGH);
  // solenoid PIN
  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(SOLENOID_PIN, LOW);
 
}



void triggerBell() {
  // red LED
  digitalWrite(RED_LED_PIN, LOW);
  // if (bellmute == 0) {
  //  Solenoid PIN
  digitalWrite(SOLENOID_PIN, HIGH);
  delay(30);
  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(SOLENOID_PIN, LOW);
  delay(10);
}



// #LOOP
void loop() {
  triggerBell();
  delay(300);
}
