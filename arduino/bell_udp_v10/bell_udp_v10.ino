/*---------------------------------------------------------------------------------------------
  Ring The cows project
  Firmware
  Alain Bellet 2016 / 2025

  V9
  --------------------------------------------------------------------------------------------- */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>  // for saving to EEPROM
#include <Ticker.h>  // for interupts ISR

char ssid[] = "cowbell";       // your network SSID (name)
char pass[] = "##bellbell##";  // your network password

bool DEBUG_MODE = 1;
const int FIRMWARE = 10;
const int RED_LED_PIN = 0;
const int BLUE_LED_PIN = 2;
const int SOLENOID_PIN = 15;
const int TILT_SWITCH_PIN = 12;
const int BUTTON_PIN = 13;
const int BATTERY_PIN = A0;
unsigned long milliseconds;
unsigned long loopcount = 0;
unsigned long seconds;

// A UDP instance to let us send and receive packets over UDP
WiFiClient client;
WiFiUDP Udp;
const IPAddress outIp(192, 168, 1, 11);  // remote IP (not needed for receive)
const unsigned int outPort = 8888;       // remote port (not needed for receive)
const unsigned int localPort = 9999;     // local port to listen for UDP packets (here's where we send the packets)

char packetBuffer[255];
char sendpacketBuffer[255];

int counter = 0;
char str[10];
unsigned long saved_millis;
int lastdrop = 0;
int32_t rssi;
int bellid;
char message[50];
// for tiltswitch reading
int reading;      // the current reading from the input pin
int lastReading;  // the last reading from the input pin
int batterylevel;
int movementTrigger = 0;
int bellmute = 0;
boolean useWifi = true;  // turn off for other test to avoid connection trial
uint8_t *bssid;
char bssid_last[10];
Ticker tick_1sec;
Ticker tick_500msec;
unsigned long longPressTimer;
boolean button_trigerred;
byte wifiLedStatus;
unsigned long lastWifiConnectTime = 0;    // Time when WiFi was last connected
const unsigned long LED_TIMEOUT = 30000;  // Turn off LED after 30 seconds (30000ms)
boolean ledActive = false;                // Track if the LED is currently on

// Button state variables
int buttonState = 0;                         // Current state of the button
int lastButtonState = 0;                     // Previous state of the button
unsigned long buttonPressTime = 0;           // Time when button was pressed
boolean buttonLongPressed = false;           // Flag for long press detection
const unsigned long LONG_PRESS_TIME = 2500;  // 2.5 seconds for long press

// function prototypes to avoid compile errors
void readEEPROM();
void writeEEPROM(int);
void clearEEPROM();
void WIFIconnect();
void UDPconnect();
void sendMessage(const char *data, const char *type);
void handleMessage(const char *data);
void triggerBell();
void tiltSwich();
void battery_level();
void heartbeat();
void isr_every_seconds();
void isr_every_500milliseconds();

// ## SETUP
void setup() {

  tick_1sec.attach_ms(1000, isr_every_seconds);
  tick_500msec.attach_ms(500, isr_every_500milliseconds);

  if (DEBUG_MODE) {
    Serial.begin(115200);
    String s = WiFi.macAddress();
    Serial.println("MAC ADDRESS: " + s);
    delay(50);
  }

  delay(100);

  // Manage LEDS and GPIO
  // Red LED
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, HIGH);
  // Blue LED
  pinMode(BLUE_LED_PIN, OUTPUT);
  digitalWrite(BLUE_LED_PIN, HIGH);
  // solenoid PIN
  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(SOLENOID_PIN, LOW);
  // tilt switch pin
  pinMode(TILT_SWITCH_PIN, INPUT);
  // digitalWrite(12, HIGH); // set pull-up resistor

  // Init other variables
  counter = 0;

  //*******************************************
  EEPROM.begin(512);
  writeEEPROM(1);  // write EEPROM for flashing change for every board, comment when done
  readEEPROM();
  EEPROM.end();
  //*******************************************

  Serial.print("BELL ID: ");
  Serial.println(bellid);
  Serial.print("FIMRWARE VERSION.: ");
  Serial.println(FIRMWARE);

  // Then connect to a WiFi network
  if (useWifi == true) {
    // setup
    WiFi.mode(WIFI_STA);           // station mode
    WiFi.setAutoReconnect(false);  // Enable auto reconnection
    WiFi.persistent(false);        // Save settings to flash
    WIFIconnect();
    // connect UDP
    UDPconnect();
  }
}

// CUSTOM FUNCTIONS
void WIFIconnect() {
  digitalWrite(BLUE_LED_PIN, HIGH);  // blue led off
  // Connect to WiFi network
  if (DEBUG_MODE) {
    Serial.print("Connecting to WIFI - ");
    Serial.println(ssid);
  }
  WiFi.begin(ssid, pass);
  // wait for connection
  byte ledStatus = LOW;
  int while_counter = 0;
  // Try to connect for a limited time
  while (while_counter < 20) {
    delay(20);
    if (DEBUG_MODE) {
      Serial.print("wifi try ");
      Serial.println(while_counter);
    }
    delay(20);
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    digitalWrite(BLUE_LED_PIN, ledStatus);  // blue led blink
    while_counter++;
    if (WiFi.status() == WL_CONNECTED) {
      digitalWrite(BLUE_LED_PIN, LOW);  // ON
      lastWifiConnectTime = millis();
      ledActive = true;
      if (DEBUG_MODE) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("RSSI: ");
        Serial.println(WiFi.RSSI());
      }
      break;
    }
    delay(500);
  }
}

void WIFIdisconnect() {
  digitalWrite(BLUE_LED_PIN, HIGH);  // blue led off
  // Disconnect to WiFi network
  if (DEBUG_MODE) {
    Serial.print("Disconnect  WIFI - ");
  }
  // WiFi.mode(WIFI_STA); // station mode
  WiFi.disconnect();
}

void UDPconnect() {
  if (DEBUG_MODE) {
    Serial.println("Starting UDP");
  }
  Udp.begin(localPort);
  delay(50);
  if (DEBUG_MODE) {
    Serial.print("Local port: ");
    Serial.println(Udp.localPort());
  }
}

void sendMessage(const char *data, const char *type) {
  char buffer[255];  // Buffer to hold the complete message
  int pos = 0;       // Position in the buffer

  if (strcmp(type, "P") == 0) {  //  ping back message
    pos = snprintf(buffer, sizeof(buffer), "P,%d,-", bellid);
  } else if (strcmp(type, "T") == 0) {  // movementTrigger state (tilt switch)
    pos = snprintf(buffer, sizeof(buffer), "T,%d,%s,-", bellid, data);
  } else if (strcmp(type, "S") == 0) {  // Status message
    rssi = WiFi.RSSI();                 // signal strength
    IPAddress ip = WiFi.localIP();
    pos = snprintf(buffer, sizeof(buffer), "S,%d.%d.%d.%d,%d,%d,%d,%s,%d,-",
                   ip[0], ip[1], ip[2], ip[3],
                   rssi,
                   bellid,
                   batterylevel,
                   bssid_last,
                   FIRMWARE);
  } else if (strcmp(type, "B") == 0) {  // reply to bang received
    pos = snprintf(buffer, sizeof(buffer), "B");
  } else if (strcmp(type, "H") == 0) {  // heartbeat
    pos = snprintf(buffer, sizeof(buffer), "H");
  }

  // Ensure the buffer is properly terminated
  if (pos >= sizeof(buffer)) {
    pos = sizeof(buffer) - 1;
  }
  buffer[pos] = '\0';

  // Send the complete message in one go
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(buffer, pos);
  Udp.endPacket();

  // if RSSI too low force disconnect (experimental)
  //  if (rssi < -80) {
  //    //Serial.println(rssi);
  //    WIFIdisconnect();
  //  }
}

void handleMessage(const char *data) {
  // Get the message type (first character)
  char type = data[0];

  // Skip the first character to get the rest of the data
  const char *restOfData = data + 1;

  // Serial.print("r: ");
  // Serial.println(restOfData);

  if (type == 'P') {       // Ping
    sendMessage("", "P");  // send Ping back
  }
  if (type == 'B') {  // Bang
    // turn movementTrigger off (must be better integrated)
    // movementTrigger = 0;
    triggerBell();
    sendMessage("", "B");
  }
  if (type == 'T') {  // Tilt switch toggle
    toggleMovementTrigger();
  }
  if (type == 'R') {  // disconnect the wifi to force reconnect
    WIFIdisconnect();
  }
  if (type == 'S') {  // Status request
    sendMessage("", "S");
  }

  counter++;
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
  //}
  if (DEBUG_MODE) {
    Serial.println("DING");
  }
}

void triggerMultipleBell(int nbr) {
  for (int i = 0; i <= nbr; i++) {
    // red LED
    digitalWrite(RED_LED_PIN, LOW);
    // if (bellmute == 0) {
    //  Solenoid PIN
    digitalWrite(SOLENOID_PIN, HIGH);
    delay(30);
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(SOLENOID_PIN, LOW);
    delay(10);
    //}
    if (DEBUG_MODE) {
      Serial.println("DING");
    }
  }
}

void heartbeat() {
  sendMessage("", "H");
}

void tiltSwich() {
  reading = digitalRead(TILT_SWITCH_PIN);
  if (DEBUG_MODE) {
    // Serial.println(reading);
  }

  if (lastReading != reading) {
    lastReading = reading;
    triggerBell();
    if (DEBUG_MODE) {
      Serial.println("tilt activated");
    }
    delay(100);
  }
}

void toggleMovementTrigger() {
  if (movementTrigger == 1) {
    movementTrigger = 0;
    sendMessage("0", "T");
    // digitalWrite(0, HIGH);
  } else {
    movementTrigger = 1;
    sendMessage("1", "T");
    // digitalWrite(0, LOW);
  }
}

void battery_level() {

  // read the battery level from the ESP8266 analog in pin.
  // analog read level is 10 bit 0-1023 (0V-1V).
  // our 1M & 220K voltage divider takes the max
  // lipo value of 4.2V and drops it to 0.758V max.
  // this means our min analog read value should be 580 (3.14V)
  // and the max analog read value should be 774 (4.2V).
  int analog_level = analogRead(BATTERY_PIN);
  static int level;
  // convert battery level to percent
  // Constrain the level to be within the expected range
  level = constrain(analog_level, 580, 773);
  batterylevel = map(level, 580, 773, 0, 100);

  // Ensure battery level is never negative
  if (batterylevel < 0)
    batterylevel = 0;
  if (batterylevel > 100)
    batterylevel = 100;

  if (DEBUG_MODE) {
    Serial.print("Battery Analog read: ");
    Serial.print(analog_level);
    Serial.print("Battery level: ");
    Serial.print(batterylevel);
    Serial.println("%");
  }
}

// ## EEPROM
void readEEPROM() {
  int addr = 0;
  byte value;
  // read a byte from the current address of the EEPROM
  value = EEPROM.read(addr);
  bellid = (int)value;
  delay(50);
}

void writeEEPROM(int val) {
  // first clear to avoid problems
  clearEEPROM();
  // the current address in the EEPROM (i.e. which byte
  // we're going to write to next)
  int addr = 0;
  EEPROM.begin(512);
  // each byte of the EEPROM can only hold a
  // value from 0 to 255.
  // write the value to the appropriate byte of the EEPROM.
  // these values will remain there when the board is
  // turned off.
  EEPROM.write(addr, val);
  EEPROM.commit();
  delay(500);
}

void clearEEPROM() {
  // write a 0 to all 512 bytes of the EEPROM
  for (int i = 0; i < 512; i++)
    EEPROM.write(i, 0);
  EEPROM.commit();
  delay(50);
}

void isr_every_seconds() {
  // every 10 seconds
  sendMessage("", "S");
  // every 5 seconds
  if (seconds % 5) {
    battery_level();
    // Get BSSID (Mac from router)
    bssid = WiFi.BSSID();
    // Serial.print("bssid ");
    sprintf(bssid_last, "%02x", bssid[5]);
  }
  seconds = seconds + 1;
  if (ledActive && (millis() - lastWifiConnectTime > LED_TIMEOUT)) {
    digitalWrite(BLUE_LED_PIN, HIGH);  // Blue LED off
    ledActive = false;
  }
}

void isr_every_500milliseconds() {
  heartbeat();
}

// #LOOP
void loop() {
  milliseconds = millis();

  if (DEBUG_MODE) {
    if (milliseconds - saved_millis > 1) {
      Serial.println(milliseconds - saved_millis);
    }
  }
  if (movementTrigger == 1) {
    if ((milliseconds % 100) == 0) {
      // check for tilt switch
      tiltSwich();
    }
  }

  // Non-blocking button handling
  int currentButtonState = digitalRead(BUTTON_PIN);

  // Button state changed from not pressed to pressed
  if (currentButtonState == HIGH && lastButtonState == LOW) {
    buttonPressTime = millis();  // Record press time
    buttonLongPressed = false;   // Reset long press flag
    if (DEBUG_MODE) {
      Serial.println("Button pressed");
    }
  }
  // Button is being held down
  else if (currentButtonState == HIGH && lastButtonState == HIGH) {
    // Check for long press
    if (!buttonLongPressed && (millis() - buttonPressTime >= LONG_PRESS_TIME)) {
      buttonLongPressed = true;
      if (DEBUG_MODE) {
        Serial.println("Long press detected - entering deep sleep");
      }
      // Enter deep sleep mode
      ESP.deepSleep(0);  // must use the hardware reset button to wake-up
    }
  }
  // Button released
  else if (currentButtonState == LOW && lastButtonState == HIGH) {
    // If it wasn't a long press and button was held for at least 100ms (debounce)
    if (!buttonLongPressed && (millis() - buttonPressTime >= 100)) {
      // Normal press action - trigger bell
      triggerBell();
      // turn on blue LED again to see wifi status
      if (WiFi.status() == WL_CONNECTED) {
        lastWifiConnectTime = millis();
      }
      if (DEBUG_MODE) {
        Serial.println("Normal press - bell triggered");
      }
    }
  }

  // Save current button state for next loop
  lastButtonState = currentButtonState;

  // check if WIFI lost

  if (WiFi.status() == WL_CONNECTED) {
    // Only turn on LED if it's not active and we're within the timeout period
    if (!ledActive && (millis() - lastWifiConnectTime <= LED_TIMEOUT)) {
      digitalWrite(BLUE_LED_PIN, LOW);  // Blue LED solid ON when connected
      ledActive = true;
      if (DEBUG_MODE) {
        Serial.println("LED activated");
      }
    }

    // Check if we need to turn off the LED to save battery
    if (ledActive && (millis() - lastWifiConnectTime > LED_TIMEOUT)) {
      digitalWrite(BLUE_LED_PIN, HIGH);  // Turn LED off after timeout
      ledActive = false;
      if (DEBUG_MODE) {
        Serial.println("LED timeout - turning off to save battery");
      }
    }
  } else {
    // Disconnected or reconnecting
    digitalWrite(BLUE_LED_PIN, HIGH);  // Blue LED off when disconnected
    ledActive = false;                 // Mark LED as inactive
    WIFIconnect();
  }

  // UDP / OSC
  int packetSize = Udp.parsePacket();
  // Serial.println(packetSize);
  if (packetSize) {
    int len = Udp.read(packetBuffer, 255);
    if (len > 0)
      packetBuffer[len] = 0;
    // Serial.println(packetBuffer);
    handleMessage(packetBuffer);
  }
  saved_millis = milliseconds;
}
