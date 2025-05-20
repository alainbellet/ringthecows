class BellManager {


  int nrOfBells;
  int csv_count;
  int[] realBellNumbers = {8, 10, 12, 13, 15, 16, 17, 18, 19, 20};
  Bell[] bells;

  BellManager() {
    //init constructor
    println("init Manager");
    csv_count = 0;
  }
  public  void createBells(int nr) {
    bells = new Bell[nr];
    for (int i = 0; i < nr; i = i+1) {
      bells[i] = new Bell(i, realBellNumbers[i]);
    }
  }
  public  void update() {
    for (Bell bell : bells) {
      bell.update();
    }
  }


  public  void setStatus(int id, String ip, int rssi, int batterylevel, String bssid, int firmware, int autoTrigger) {
    csv_count++;
    bells[id].setStatus(ip, getWifiQuality(rssi), batterylevel, bssid, firmware, autoTrigger);
    //println(bssid);
    //String csv = str(csv_count)+","+str(id)+"," +ip+ "," + str(rssi)+","+ str(getWifiQuality(rssi));
    
    //myUdpIOManager.appendTextToFile("rssi_log.csv", csv);
    
  }
  public void heartbeat(int id, String ip) {
  }
  public int getWifiQuality(int dBm) {
    int quality;
    // dBm to Quality:
    if (dBm <= -100) {
      quality = 0;
    } else if (dBm >= -50) {
      quality = 100;
    } else {
      quality = 2 * (dBm + 100);
    }
    return quality; // in percentage
  }
  public  void createGUI() {
    int posX = 10;
    int posY = 100;
    // GENERAL CONTROLLERS
    // MAtrix
    int nx = 30;
    int ny = 10;
    List listBroadcastIP = Arrays.asList("192.168.1.255", "192.168.0.255");

    cp5.addMatrix("myMatrix")
      .setPosition(posX+90, posY+ 50)
      .setSize(300, 350)
      .setGrid(nx, ny)
      .setGap(1, 1)
      .setInterval(200)
      .setMode(ControlP5.MULTIPLES)
      .setColorBackground(color(120))
      .setBackground(color(40))
      .setLabel("")
      .plugTo(this)
      ;

    // TOGGLE UDP OUTPUT
    cp5.addToggle("toggle_all_output")
      .setPosition(posX+15, posY)
      .setSize(10, 30)
      .setLabel("UDP")
      .plugTo(this)
      ;
    // TOGGLE MUTE
    cp5.addToggle("toggle_all_mute")
      .setPosition(posX+30, posY)
      .setSize(10, 30)
      .setLabel("M")
      .plugTo(this)
      ;
    // TOGGLE MIDI
    cp5.addToggle("toggle_midi")
      .setPosition(100, posY)
      .setSize(300, 30)
      .setLabel("MIDI")
      .plugTo(this)
      ;
    // TOGGLE DEBUG CONSOLE
    cp5.addToggle("toggle_debug_console")
      .setPosition(10, 520)
      .setSize(20, 20)
      .setLabel("DEBUG CONSOLE")
      .plugTo(this)
      ;
    // TOGGLE LOG MIDI CONSOLE
    cp5.addToggle("toggle_log_midi")
      .setPosition(100, 520)
      .setSize(20, 20)
      .setLabel("LOG MIDI")
      .plugTo(this)
      ;
    // TOGGLE SLEEP MODE FOR ALL BOARDS
    cp5.addToggle("toggle_sleep")
      .setPosition(630, 50)
      .setSize(20, 20)
      .setLabel("SLEEP BOARDS")
      .plugTo(this)
      ;
    // TOGGLE Auto
    cp5.addButton("tilt_all")
      .setPosition(posX+ 620, posY)
      .setSize(25, 30)
      .setLabel("A")
      .plugTo(this)
      .setColorBackground(greycolor)
      .setColorActive(greencolor)
      ;
    // TOGGLE Deepsleep
    cp5.addButton("deepsleep_all")
      .setPosition(posX+ 650, posY)
      .setSize(25, 30)
      .setLabel("Z")
      .plugTo(this)
      .setColorBackground(greycolor)
      .setColorActive(greencolor)
      ;
    // Restart Wifi
    cp5.addButton("restart_wifi_all")
      .setPosition(posX+ 530, posY)
      .setSize(25, 30)
      .setLabel("W")
      .plugTo(this)
      .setColorBackground(greycolor)
      .setColorActive(greencolor)
      ;
    cp5.addScrollableList("dropdownIP") // choose broadcast IP (depend on wifi routeur used)
      .setPosition(26, 20)
      .setSize(100, 70)
      .setBarHeight(20)
      .setItemHeight(20)
      .setLabel("IP BROADCAST")
      .setOpen(false)
      .plugTo(this)
      .addItems(listBroadcastIP)
      ;

    posY = posY+50;
    // BELL CONTROLLERS

    for (Bell bell : bells) {
      bell.createGUI(posX, posY);
      posY = posY+35;
    }

    cp5.get(Toggle.class, "toggle_all_output").setState(false);
    cp5.get(Toggle.class, "toggle_midi").setState(false);
    cp5.get(Toggle.class, "toggle_debug_console").setState(true);
    cp5.get(Toggle.class, "toggle_log_midi").setState(true);
    dropdownIP(1);
  }

  public void controlEvent(ControlEvent theEvent) {
    boolean val = false;
    if (theEvent.getController().getName().equals("toggle_all_output")) {
      if (theEvent.getController().getValue() == 1) {
        val = true;
      } else {
        val = false;
      }

      for (Bell bell : bells) {
        bell.toggleOutput(val);
      }
    }
    if (theEvent.getController().getName().equals("toggle_midi")) {
      if (theEvent.getController().getValue() == 1) {
        toggleMidi(true);
      } else {
        toggleMidi(false);
      }
    }
    if (theEvent.getController().getName().equals("toggle_sleep")) {
      if (theEvent.getController().getValue() == 1) {
        println("SLEEP MODE ON" );
        sleepModeOn = true;
      } else {
        println("SLEEP MODE OFF (waiting for boards to wake up)" );
        sleepModeOn = false;
      }
    }
    if (theEvent.getController().getName().equals("tilt_all")) {

      for (Bell bell : bells) {
        bell.sendToggleTilt();
      }
    }
    if (theEvent.getController().getName().equals("deepsleep_all")) {

      for (Bell bell : bells) {
        bell.sendToggleDeepSleep();
      }
    }
    if (theEvent.getController().getName().equals("restart_wifi_all")) {

      for (Bell bell : bells) {
        bell.sendRestartWifi();
      }
    }

    if (theEvent.getController().getName().equals("toggle_debug_console")) {
      if (theEvent.getController().getValue() == 1) {
        debugConsole = true;
        println("console ON");
      } else {
        debugConsole = false;
        println("console OFF");
      }
    }

    if (theEvent.getController().getName().equals("toggle_log_midi")) {
      if (theEvent.getController().getValue() == 1) {
        logMidi = true;
        println("Log MIDI ON");
      } else {
        logMidi = false;
        println("Log MIDI OFF");
      }
    }
  }
  public void toggleMidi(boolean val) {
    if (val == true) {
      cp5.get(Matrix.class, "myMatrix").pause();
      playMidi = true;
    } else {
      cp5.get(Matrix.class, "myMatrix").play();
      playMidi = false;
    }
  }

  public void myMatrix(int theX, int theY) {
    //println("got it: "+theX+", "+theY);
    sendBang(theY);
    //d[theX][theY].update();
  }

  public void dropdownIP(int n) {
    /* request the selected item based on index n */
    myIPBroadcast = cp5.get(ScrollableList.class, "dropdownIP").getItem(n).get("text").toString();
    println("IP Broadcast Update: "+myIPBroadcast);
  }

  public void playNote(int note) {
    if (logMidi) {
      println("MIDI note on " + note );
    }

    switch(note) {
    case 83: // for control (toggle tilt auto)
      for (Bell bell : bells) {
        // bell.sendToggleTilt();
      }
      break;
    case 84: // for control (toggle midi input)
      // toggleMidi();
      break;
    }

    if ( playMidi == true) {
      switch(note) {
      case 60:
        bells[9].bang();
        break;
      case 62:
        bells[8].bang();
        break;
      case 64:
        bells[7].bang();
        break;
      case 65:
        bells[6].bang();
        break;
      case 67:
        bells[5].bang();
        break;
      case 69:
        bells[4].bang();
        break;
      case 71:
        bells[3].bang();
        break;
      case 72:
        bells[2].bang();
        break;
      case 74:
        bells[1].bang();
        break;
      case 76:
        bells[0].bang();
        break;
      default:
        if (logMidi) {
          println("MIDI note Out of range" );
        }
      }
    }
  }

  public  void setIp(String ip, int id) {
    //IPArray = append(IPArray, "192.168.1.11");
    //bells[id].setIp(ip);
  }
  public  void sendBang(int id) {
    //IPArray = append(IPArray, "192.168.1.11");
    bells[id].bang();
  }
}
