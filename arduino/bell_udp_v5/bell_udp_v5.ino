/*---------------------------------------------------------------------------------------------
  Ring The cows project
  Firmware
  © Alain Bellet 2016

  V4
  --------------------------------------------------------------------------------------------- */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h> // for saving to EEPROM

char ssid[] = "cowbell";    // your network SSID (name)
char pass[] = "##bellbell##";  // your network password
bool DEBUG_MODE = 1;
const int FIRMWARE = 5;
unsigned long milliseconds;
unsigned long loopcount = 0;

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
int movementTrigger = 1;
int bellmute = 0;
boolean useWifi = true; // turn off for other test to avoid connection trial
int lastButtonState = 0;
int buttonReading = 0;
uint8_t *bssid;

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

// ## SETUP
void setup() {




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

  // write EEPROM for flashing
  writeEEPROM(0);

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
  WiFi.begin(ssid, pass);
  // wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  digitalWrite(2, LOW); //ON
  if (DEBUG_MODE) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

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

  if (DEBUG_MODE) {
    Udp.write("#IP: ");
    Udp.print(WiFi.localIP());
    Udp.write("#DBm: ");
    Udp.print(rssi);
    Udp.write(" #drop: ");
    Udp.print(lastdrop);
    Udp.write(" #count: ");
  }
  if (type = "S") { // Status message
    Udp.write("S,");
    Udp.print(WiFi.localIP());
    Udp.write(",");
    Udp.print(rssi);
    Udp.write(",");
    Udp.print(bellid);
    Udp.write(",");
    Udp.print(batterylevel);
    Udp.write(",");
    Udp.print(FIRMWARE);
    Udp.write(",");
  }
  if (type = "B") { // reply to bang recieved
    Udp.write("B");
  }
  //Udp.println(data);
  Udp.endPacket();
}



void handleMessage(String data) {
  // split counter and note value
  // p.ex 02637 (the note is the first char)
  String type = data.substring(0, 1);
  data = data.substring(1, data.length());
  //Serial.print("r: ");
  //Serial.println(data);
  if (type == "A") { // Alive request for Status update
    sendMessage ("", "S");
  }
  if (type == "B") { // Bang
    // turn movementTrigger off (must be better intagrated)
    movementTrigger = 0;
    triggerBell();
    sendMessage ("", "B");
  }
  if (type == "T") { // Tilt switch activation
    // turn movementTrigger off (must be better intagrated)
    if (data == "1") {
      movementTrigger = 1;
      sendMessage ("", "S");
    } else {
      movementTrigger = 0;
      sendMessage ("", "S");
    }
    if (type == "M") { // Mute
      // turn movementTrigger off (must be better intagrated)
      if (data == "1") {
        bellmute = 1;
        sendMessage ("", "S");
      } else {
        bellmute = 0;
        sendMessage ("", "S");
      }

    }

    if (data == "reset") {
      counter = 0;
      lastdrop = 0;
    }

    //Serial.print("l: ");
    //Serial.println(counter);
    // check for packet loss
    if ((data.toInt() - counter) > lastdrop) {
      Serial.print("drop");
      Serial.println(data.toInt() - counter);
      //sprintf(str, "%d", data.toInt()-counter);
      //sendMessage(str);
      lastdrop = data.toInt() - counter;
    }
    sprintf(str, "%d", counter);
    //sendMessage(str);
    sprintf(str, "%d", bellid);
    sendMessage (str, "A");
    counter++;
  }
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

void tiltSwicth() {
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
    //delay (1000);
  }


}

void toggleMovementTrigger() {

}

void battery_level() {

  // read the battery level from the ESP8266 analog in pin.
  // analog read level is 10 bit 0-1023 (0V-1V).
  // our 1M & 220K voltage divider takes the max
  // lipo value of 4.2V and drops it to 0.758V max.
  // this means our min analog read value should be 580 (3.14V)
  // and the max analog read value should be 774 (4.2V).
  int level = analogRead(A0);
  Serial.print("Analog pin: "); Serial.println(level);

  // convert battery level to percent
  batterylevel = map(level, 580, 773, 0, 100);

  if (DEBUG_MODE) {
    Serial.print("Battery level: "); Serial.print(batterylevel); Serial.println("%");
  }

// Get BSSID (Mac from router)
 bssid = WiFi.BSSID();
 Serial.print("bssid ");
  Serial.print(bssid[5],HEX);

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

// #LOOP
void loop() {
  milliseconds = millis();
  if ((milliseconds % 10000) == 0 && milliseconds != saved_millis) {
    battery_level();
  }
  // for Battery life test (activate solenoid every 200ms)
//  if ((milliseconds % 300) == 0 && milliseconds != saved_millis) {
//    triggerBell();
//  }



  if (DEBUG_MODE) {
    if (milliseconds - saved_millis > 1) {
      Serial.println(milliseconds - saved_millis);
    }
  }
  if ((milliseconds % 100) == 0) {
    // check for tilt switch
    if (movementTrigger == 1) {
      tiltSwicth();
    }
  }
  // check for switch
  buttonReading = digitalRead(13);
  if (buttonReading != lastButtonState) {
    if (buttonReading == HIGH) {
      lastButtonState = 1;
      if (DEBUG_MODE) {
        Serial.println("button activated");
      }
      triggerBell();
      delay(100);
    } else {
      lastButtonState = 0;
      delay(100);
    }
  }
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


