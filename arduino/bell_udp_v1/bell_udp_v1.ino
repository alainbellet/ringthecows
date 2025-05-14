/*---------------------------------------------------------------------------------------------
  Ring The cows project
  
  © Alain Bellet 2016

  V1
--------------------------------------------------------------------------------------------- */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h> // for saving to EEPROM

char ssid[] = "AWIFIHOME";    // your network SSID (name)
char pass[] = "##christ0##";  // your network password
bool DEBUG_MODE = 1; 


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
int saved_millis;
int lastdrop = 0;
int32_t rssi;


void setup() {
  if (DEBUG_MODE){
    Serial.begin(115200);
  }
  delay(100);

  // We start by connecting to a WiFi network
  Serial.println("");
  Serial.println("connecting to wifi");
  //WiFi.begin("AGUEST", "barbarossa");
  WiFi.begin("AWIFIHOME", "##christ0##");
  Udp.begin(localPort);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // Blue LED
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  // Red LED
  pinMode(0, OUTPUT);
  digitalWrite(0, HIGH);
  // solenoid pin
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  counter = 0;
}

void WIFIconnect() {
  digitalWrite(2, HIGH); // blue led off
  // Connect to WiFi network
  if (DEBUG_MODE) {
    Serial.print("Connecting to ");
    Serial.println(ssid);
  }
  WiFi.begin(ssid, pass);
  // wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  digitalWrite(2, LOW); //ON
  if (DEBUG_MODE) {
    Serial.println("");
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

}


void sendMessage(char data[]) {
    rssi = WiFi.RSSI();//signal strentgh
    
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write("#IP: ");
    Udp.print(WiFi.localIP());
    Udp.write("#DBm: ");
    Udp.print(rssi);
    Udp.write(" #drop: ");
    Udp.print(lastdrop);
    Udp.write(" #count: ");
    Udp.println(data);
    Udp.endPacket(); 
}

void handleMessage(String data) {
  // split counter and note value
  // p.ex 02637 (the note is the first char)
  String note = data.substring(0, 1);
  data = data.substring(1, data.length());
  Serial.print("r: ");
  Serial.println(data); 
  if (note == "1") {
    digitalWrite(0, LOW);
    digitalWrite(15, HIGH);
    delay(10);
    digitalWrite(0, HIGH);
    digitalWrite(15, LOW);
    Serial.println("ON ");
    Serial.println(counter);
    //sendMessage("on");
  }
  if (data == "off") {
    digitalWrite(0, HIGH);
    Serial.println("OFF ");
    Serial.println(counter);
    //sendMessage("off");
  }
  if (data == "reset") {
    counter = 0;
    lastdrop = 0;
  }
  
  //Serial.print("l: ");
  //Serial.println(counter);
  // check for packet loss
  if ((data.toInt()-counter)>lastdrop){
    Serial.print("drop");
    Serial.println(data.toInt()-counter);
    //sprintf(str, "%d", data.toInt()-counter);
    //sendMessage(str);
    lastdrop = data.toInt()-counter;
  }
  sprintf(str, "%d", counter);
  sendMessage(str);
  counter++;
}



void loop() {
  //Serial.println("loop: ");
  
  if (millis()-saved_millis > 1){
  Serial.println(millis()-saved_millis);
  }
   
  
  int packetSize = Udp.parsePacket();
  //Serial.println(packetSize);
  if (packetSize) {
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = 0;
    //Serial.println(packetBuffer);
    handleMessage(String(packetBuffer));
  }
  saved_millis = millis();
}


