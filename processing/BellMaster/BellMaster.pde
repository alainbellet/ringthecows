/**
 * Bell Controller
 *
 *
 * V1.0
 * A. Bellet 2016 / Ring The cows
 * Edited 2025 / Klangwelt
 */
import controlP5.*;
import hypermedia.net.*;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.util.*;
import rwmidi.*;

ControlP5 cp5;
MidiInput mymididevice; 
String[] IPArray = new String[0];
int greycolor = color(50, 50, 50);
int redcolor = color(255, 0, 0);
int greencolor = color(0, 255, 0);
int blackcolor = color(0, 0, 0);
int yellowcolor = color(231, 213, 20);
int orangecolor = color(231, 119, 20);
boolean playMidi = false;
String myIPBroadcast;
Textlabel myTextlabel;

BellManager myBellManager;
ConfigurationManager myConfigManager;
UdpIOManager myUdpIOManager;
MidiManager myMidiManager;
Textarea myTextarea; // for console
Println console; // for console

boolean debugConsole = false;
boolean logMidi = false;


// midiBus
//MidiBus myMidiBus; // The MidiBus

void setup() {
  background(30);
  size(680, 810);
  frameRate(60);
  noSmooth(); 

  //PFont font = createFont("verdana", 14, true); // suppimé pour optimistaion mémoire
  //textFont(font);  // suppimé pour optimistaion mémoire
  cp5 = new ControlP5(this);
  //cp5.addFrameRate().setInterval(10).setPosition(0, height - 10);
  // console
  myTextarea = cp5.addTextarea("txt")
    .setPosition(10, 560)
    .setSize(650, 230)
    .setLineHeight(14)
    .setColor(greencolor)
    .setColorBackground(color(0, 100))
    .setColorForeground(color(255, 100));
  ;
  console = cp5.addConsole(myTextarea);//
  console.setMax(15);
  myTextlabel = cp5.addTextlabel("label")
                    .setText("BELL MASTER 1.2 / RING THE COWS KLANGWELT")
                    .setPosition(410,20)
                    .setColorValue(0xffffffff)
                    ;
  
                    
  myUdpIOManager = new UdpIOManager();

  myBellManager = new BellManager();
  myBellManager.createBells(10);
  myBellManager.createGUI();
  myConfigManager = new ConfigurationManager();
  myConfigManager.loadConfig("");

  // MIDI
  myMidiManager = new MidiManager(this);
  // MidiBus.list();
}

void keyPressed() {
  switch(key) {
    case('1'):
    myBellManager.sendBang(0);
    break;
    case('2'):
    myBellManager.sendBang(1);
    break;
    case('3'):
    myBellManager.sendBang(2);
    break;
    case('4'):
    myBellManager.sendBang(3);
    break;
    case('5'):
    myBellManager.sendBang(4);
    break;
    case('6'):
    myBellManager.sendBang(5);
    break;
    case('7'):
    myBellManager.sendBang(6);
    break;
    case('8'):
    myBellManager.sendBang(7);
    break;
    case('9'):
    myBellManager.sendBang(8);
    break;
    case('0'):
    myBellManager.sendBang(9);
    break;
  }
}

void draw() {
  
  if (frameCount % 5 == 0) {// have to be done less often
    myBellManager.update();
  }
  if (frameCount % 20 == 0) {// have to be done less often // changed it was 60 in fribourg
    myUdpIOManager.sendBroadcastMessage("P");
  }
  if (frameCount % 300 == 0) {// have to be done less often
    myUdpIOManager.sendBroadcastMessage("S");
  }
}
