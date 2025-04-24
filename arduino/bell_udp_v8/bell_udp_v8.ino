/*---------------------------------------------------------------------------------------------
  Ring The cows project
  Firmware
  © Alain Bellet 2016

  V4
  --------------------------------------------------------------------------------------------- */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h> // for saving to EEPROM
#include <Ticker.h> // for interupts ISR

char ssid[] = "cowbell";    // your network SSID (name)
char pass[] = "##bellbell##";  // your network password
//char ssid[] = "zorro";    // your network SSID (name)
//char pass[] = "Zampano1";  // your network password
bool DEBUG_MODE = 0;
const int FIRMWARE = 8;
unsigned long milliseconds;
unsigned long loopcount = 0;
unsigned long seconds;

// A UDP instance to let us send and receive packets over UDP
WiFiClient client;
WiFiUDP Udp;
const IPAddress outIp(192, 168, 1, 11);     // remote IP (not needed for receive)
const unsigned int outPort = 8888;          // remote port (not needed for receive)
const unsigned int localPort = 9999;        // local port to listen for UDP packets (here's where we send the packets)

char packetBuffer[255];
char sendpacketBuffer[255];

int counter = 0;
char str[10];
String mystr;
unsigned long saved_millis;
int lastdrop = 0;
int32_t rssi;
int bellid;
String message;
// for tiltswitch reading
int reading;           // the current reading from the input pin
int lastReading;       // the last reading from the input pin
int batterylevel;
int movementTrigger = 0;
int bellmute = 0;
boolean useWifi = true; // turn off for other test to avoid connection trial
int lastButtonState = 0;
int buttonReading = 0;
uint8_t *bssid;
String bssid_last;
Ticker tick_1sec;
Ticker tick_500msec;
unsigned long longPressTimer;
boolean button_trigerred;

// function prototypes to avoid compile errors
void readEEPROM();
void writeEEPROM(int);
void clearEEPROM();
void WIFIconnect();
void UDPconnect();
void sendMessage();
void handleMessage();
void triggerBell();
void tiltSwicth();
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
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
  // Blue LED
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  // solenoid PIN
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  // tilt switch pin
  pinMode(12, INPUT);
  //digitalWrite(12, HIGH); // set pull-up resistor


  // Init other variables
  counter = 0;

  //*******************************************
  writeEEPROM(8); // write EEPROM for flashing change for every board
  //*******************************************
  delay (100);
  readEEPROM();
  delay (100);
  Serial.print("BELL ID: ");
  Serial.println(bellid);
  Serial.print("FIMRWARE VERSION.: ");
  Serial.println(FIRMWARE);

  // Then connect to a WiFi network
  if (useWifi == true) {
    WIFIconnect();
    // connect UDP
    UDPconnect();
  }
}
// CUSTOM FUNCTIONS
void WIFIconnect() {
  digitalWrite(2, HIGH); // blue led off
  // Connect to WiFi network
  if (DEBUG_MODE) {
    Serial.print("Connecting to WIFI - ");
    Serial.println(ssid);
  }
  WiFi.mode(WIFI_STA); // station mode
  //WiFi.setPhyMode(WIFI_PHY_MODE_11B);
  WiFi.begin(ssid, pass);
  // wait for connection
  byte ledStatus = LOW;
  int while_counter = 0;
  while (while_counter < 5) {
    delay(500);
    Serial.print(".");
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;
    digitalWrite(2, ledStatus); // blue led blink
    while_counter++;
    if (WiFi.status() != WL_CONNECTED) {
      digitalWrite(2, LOW); //ON
      if (DEBUG_MODE) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
      }
      break;
    }
  }


}

void WIFIdisconnect() {
  digitalWrite(2, HIGH); // blue led off
  // Disconnect to WiFi network
  if (DEBUG_MODE) {
    Serial.print("Disconnect  WIFI - ");
  }
  //WiFi.mode(WIFI_STA); // station mode
  WiFi.disconnect();

}

void UDPconnect() {
  if (DEBUG_MODE) {
    Serial.println("Starting UDP");
  }
  Udp.begin(localPort);
  delay (50);
  if (DEBUG_MODE) {
    Serial.print("Local port: ");
    Serial.println(Udp.localPort());
  }
}

void sendMessage(char data[], String type) {
  rssi = WiFi.RSSI();//signal strentgh
  //byte bssid[6];


  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());

  //  if (DEBUG_MODE) {
  //    Udp.write("#IP: ");
  //    Udp.print(WiFi.localIP());
  //    Udp.write("#DBm: ");
  //    Udp.print(rssi);
  //    Udp.write(" #drop: ");
  //    Udp.print(lastdrop);
  //    Udp.write(" #count: ");
  //  }
  if (type == "P") { //  ping back message
    Udp.write("P,");
    Udp.print(bellid);
    Udp.write(",");
    Udp.write("-");
  }
  if (type == "T") { // movementTrigger state (tilt switch)
    Udp.write("T,");
    Udp.print(bellid);
    Udp.write(",");
    Udp.print(data);
    Udp.write(",");
    Udp.write("-");

  }
  if (type == "S") { // Status message
    Udp.write("S,");
    Udp.print(WiFi.localIP());
    Udp.write(",");
    Udp.print(rssi);
    Udp.write(",");
    Udp.print(bellid);
    Udp.write(",");
    Udp.print(batterylevel);
    Udp.write(",");
    Udp.print(bssid_last);
    Udp.write(",");
    Udp.print(FIRMWARE);
    Udp.write(",");
    Udp.write("-");

  }
  if (type == "B") { // reply to bang recieved
    Udp.write("B");
  }
  //Udp.println(data);
  Udp.endPacket();

  // if RSSI too low force disconect (experimental)

  //  if (rssi < -80) {
  //    //Serial.println(rssi);
  //    WIFIdisconnect();
  //  }
}




void handleMessage(String data) {
  // split counter and note value
  // p.ex 02637 (the note is the first char)
  String type = data.substring(0, 1);
  data = data.substring(1, data.length());
  //Serial.print("r: ");
  //Serial.println(data);
  if (type == "P") { // Ping
    sendMessage ("", "P"); // send Ping back
  }
  if (type == "B") { // Bang
    // turn movementTrigger off (must be better intagrated)
    //movementTrigger = 0;
    triggerBell();
    sendMessage ("", "B");
  }
  if (type == "T") { // Tilt switch toggle
    toggleMovementTrigger();

  }

  if (type == "R") { // disconnect the wifi to force reconnect
    WIFIdisconnect();
  }
  if (type == "S") { // Status request
    sendMessage ("", "S");
  }

  //Serial.print("l: ");
  //Serial.println(counter);
  // check for packet loss
  //  if ((data.toInt() - counter) > lastdrop) {
  //    Serial.print("drop");
  //    Serial.println(data.toInt() - counter);
  //    //sprintf(str, "%d", data.toInt()-counter);
  //    //sendMessage(str);
  //    lastdrop = data.toInt() - counter;
  //  }
  //  sprintf(str, "%d", counter);
  //  //sendMessage(str);
  //  sprintf(str, "%d", bellid);
  //sendMessage (str, "A");
  counter++;
}

void triggerBell() {
  // red LED
  digitalWrite(0, LOW);
  //if (bellmute == 0) {
  // Solenoid PIN
  digitalWrite(15, HIGH);
  delay(30);
  digitalWrite(0, HIGH);
  digitalWrite(15, LOW);
  delay(10);
  //}
  if (DEBUG_MODE) {
    Serial.println("DING");
  }
}

void triggerMultipleBell(int nbr) {
  for (int i = 0; i <= nbr; i++) {
    // red LED
    digitalWrite(0, LOW);
    //if (bellmute == 0) {
    // Solenoid PIN
    digitalWrite(15, HIGH);
    delay(30);
    digitalWrite(0, HIGH);
    digitalWrite(15, LOW);
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

void tiltSwitch() {
  reading = digitalRead(12);
  if (DEBUG_MODE) {
    // Serial.println(reading);
  }


  if (lastReading != reading) {
    lastReading = reading;
    triggerBell();
    if (DEBUG_MODE) {
      Serial.println("tilt activated");
    }
    delay (300);
  }


}

void toggleMovementTrigger() {
  if (movementTrigger == 1) {
    movementTrigger = 0;
    sendMessage ("0", "T");
    //digitalWrite(0, HIGH);
  } else {
    movementTrigger = 1;
    sendMessage ("1", "T");
    //digitalWrite(0, LOW);
  }

}

void battery_level() {

  // read the battery level from the ESP8266 analog in pin.
  // analog read level is 10 bit 0-1023 (0V-1V).
  // our 1M & 220K voltage divider takes the max
  // lipo value of 4.2V and drops it to 0.758V max.
  // this means our min analog read value should be 580 (3.14V)
  // and the max analog read value should be 774 (4.2V).
  int level = analogRead(A0);
  //Serial.print("Analog pin: "); Serial.println(level);

  // convert battery level to percent
  batterylevel = map(level, 580, 773, 0, 100);

  if (DEBUG_MODE) {
    //Serial.print("Battery level: "); Serial.print(batterylevel); Serial.println("%");
  }



}
// ## EEPROM
void readEEPROM() {
  int addr = 0;
  byte value;
  EEPROM.begin(512);
  // read a byte from the current address of the EEPROM
  value = EEPROM.read(addr);
  /*if (DEBUG_MODE) {
    Serial.print("address: ");
    Serial.println(addr);
    Serial.print("value: ");
    Serial.println(value, DEC);
    }*/
  bellid = (int) value;

  delay (500);
}


void writeEEPROM(int val) {
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
  EEPROM.end();
  delay(500);
}


void clearEEPROM() {
  EEPROM.begin(512);
  // write a 0 to all 512 bytes of the EEPROM
  for (int i = 0; i < 512; i++)
    EEPROM.write(i, 0);
  EEPROM.end();
  delay(500);
}

void isr_every_seconds() {
  // every 10 seconds
  sendMessage ("", "S");
  // every 5 seconds
  if (seconds % 5) {
    battery_level();
    // Get BSSID (Mac from router)
    bssid = WiFi.BSSID();
    //Serial.print("bssid ");
    bssid_last = String(bssid[5], HEX);
  }
  seconds = seconds + 1;
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
      tiltSwitch();
    }
  }
  // check for switch
  buttonReading = digitalRead(13);
  longPressTimer = 0;
  button_trigerred = 0;
  digitalWrite(2, LOW);
  while (digitalRead(13) == HIGH)
  {
    delay(100);
    longPressTimer++;
    if (longPressTimer < 20 && button_trigerred == 0) {
      button_trigerred = 1;
      triggerBell();
      delay(100);
    } else if (longPressTimer == 25) {
      // 2 seconds has passed, note that the button may still be down
      ESP.deepSleep(0); // must use the hardware reset button to wake-up
      break;
    }
  }
  //  if (buttonReading != lastButtonState) {
  //    longPressTimer = 0;
  //    if (buttonReading == HIGH) {
  //
  //      lastButtonState = 1;
  //      if (DEBUG_MODE) {
  //        Serial.println("button activated");
  //      }
  //      triggerBell();
  //      delay(100);
  //    } else {
  //      lastButtonState = 0;
  //      delay(100);
  //    }
  //  }
  // check if WIFI lost
  if (WiFi.status() != WL_CONNECTED && useWifi == true ) {
    if (DEBUG_MODE) {
      Serial.println("WiFi disconnected, try to reconnect");
    }
    WIFIconnect();
    delay(500);
  }

  int packetSize = Udp.parsePacket();
  //Serial.println(packetSize);
  if (packetSize) {
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = 0;
    //Serial.println(packetBuffer);
    handleMessage(String(packetBuffer));
  }
  saved_millis = milliseconds;

}


