class Bell {
  int myId;
  int myBellNumber;
  int aliveUpdate;
  boolean isUp;
  boolean udpOutput;
  String myIP;
  boolean notifyDown ;
  int csv_count_batt;

  Bell(int id, int bellNumber) {
    //init constructor
    //println("init bell"+id);
    myId = id;
    myBellNumber = bellNumber;
    isUp = false;
    udpOutput = false;
    notifyDown = true;
    csv_count_batt = 0;
  }

  public  void update() {
    //
    //println("update bell "+myId);
    cp5.get(Button.class, "bang"+myId).setColorBackground(greycolor);
    // check if connection lost (after ~3 seconds without messages from board)
    if (millis() - aliveUpdate >= 2100) {
      // board is down
      isUp = false;
      cp5.get(Toggle.class, "toggleAlive"+myId).setColorBackground(redcolor);
      // LOG
      if (debugConsole && notifyDown) {
        println(hour()+":"+minute()+":"+second()+" BOARD "+myId+" DOWN" );
        notifyDown = false;
      }
    } else {
      cp5.get(Toggle.class, "toggleAlive"+myId).setColorBackground(greencolor);
      //println(hour()+":"+minute()+":"+second()+" BOARD "+myId+" UP" );
      notifyDown = true;
    }
  }

  public void requestStatusFromBoard() {
    if (isUp == true) {
      myUdpIOManager.sendMessage("S", myIP);
    }
  }

  public void requestResetBoard() {
    if (isUp == true) {
      myUdpIOManager.sendMessage("R", myIP);
    }
  }

  public  void createGUI(int originX, int originY) {


    // TOGGLE Alive
    cp5.addToggle("toggleAlive"+myId)
      .setPosition(originX, originY)
      .setSize(10, 30)
      .setLabel("")
      .lock()
      .plugTo(this)
      .setColorBackground(greycolor)
      .setColorActive(redcolor)
      ;
    // TOGGLE UDP Output
    cp5.addToggle("toggleOutput"+myId)
      .setPosition(originX+15, originY)
      .setSize(10, 30)
      .setLabel("")
      .plugTo(this)
      ;
    // MUTE CHANNEL (still send messages but muted on the board)
    cp5.addToggle("muteOutput"+myId)
      .setPosition(originX+30, originY)
      .setSize(10, 30)
      .setLabel("")
      .plugTo(this)
      ;
    // BANG
    cp5.addButton("bang"+myId)
      .setPosition(originX+ 50, originY)
      .setSize(30, 30)
      .setId(myId)
      .setLabel(myId+" ("+myBellNumber+")")
      .plugTo(this)
      .setColorBackground(greycolor)
      .setColorActive(redcolor)
      ;
    // TEXT FIELD
    cp5.addTextfield("ip"+myId)
      .setPosition(originX+400, originY)
      .setSize(60, 30)
      .setAutoClear(false)
      .lock()
      .setLabel("")
      .setText("000.000.0.00")
      .setColorBackground(greycolor)
      ;
    cp5.addTextfield("wifi"+myId)
      .setPosition(originX+470, originY)
      .setSize(25, 30)
      .setAutoClear(false)
      .lock()
      .setLabel("")
      .setText("-")
      .setColorBackground(greycolor)
      ;
    cp5.addTextfield("bssid"+myId)
      .setPosition(originX+500, originY)
      .setSize(25, 30)
      .setAutoClear(false)
      .lock()
      .setLabel("")
      .setText("-")
      .setColorBackground(greycolor)
      ;
    // WIFI RECONNECT
    cp5.addButton("wifi_reconnect"+myId)
      .setPosition(originX+ 530, originY)
      .setSize(25, 30)
      .setId(myId)
      .setLabel("W")
      .plugTo(this)
      .setColorBackground(greycolor)
      .setColorActive(redcolor)
      ;
    cp5.addTextfield("battery"+myId)
      .setPosition(originX+560, originY)
      .setSize(25, 30)
      .setAutoClear(false)
      .lock()
      .setLabel("")
      .setText("-")
      .setColorBackground(greycolor)
      ;

    cp5.addTextfield("firmware"+myId)
      .setPosition(originX+590, originY)
      .setSize(25, 30)
      .setAutoClear(false)
      .lock()
      .setLabel("")
      .setText("-")
      .setColorBackground(greycolor)
      ;
    cp5.addButton("tilt"+myId)
      .setPosition(originX+ 620, originY)
      .setSize(25, 30)
      .setId(myId)
      .setLabel("A")
      .plugTo(this)
      .setColorBackground(greycolor)
      .setColorActive(greencolor)
      ;
    cp5.addButton("deepsleep"+myId)
      .setPosition(originX+ 650, originY)
      .setSize(25, 30)
      .setId(myId)
      .setLabel("Z")
      .plugTo(this)
      .setColorBackground(greycolor)
      .setColorActive(greencolor)
      ;
  }

  public void setStatus(String ip, int wifiquality, int batterylevel, String bssid, int firmware, int autoTrigger) {
    cp5.get(Textfield.class, "ip"+myId).setText(ip);
    cp5.get(Textfield.class, "wifi"+myId).setText(str(wifiquality)+"%");
    cp5.get(Textfield.class, "wifi"+myId).setColorBackground(color(wifiquality*1.5, wifiquality*1.5, wifiquality*1.5));
    cp5.get(Textfield.class, "battery"+myId).setText(str(batterylevel)+"%");
    if (batterylevel < 0 ) {
      cp5.get(Textfield.class, "battery"+myId).setText("-");
    }

    if (batterylevel < 30 && batterylevel >0) {
      cp5.get(Textfield.class, "battery"+myId).setColorBackground(redcolor);
    } else {
      cp5.get(Textfield.class, "battery"+myId).setColorBackground(greycolor);
    }
    // get the last charaters of the AP mac adress to determine to which one we are connected
    cp5.get(Textfield.class, "bssid"+myId).setText(bssid);
    if (bssid.equals("de") == true) {
      cp5.get(Textfield.class, "bssid"+myId).setColorBackground(yellowcolor);
      cp5.get(Textfield.class, "bssid"+myId).setText(" AP1");
    } else if (bssid.equals("16") == true) {
      cp5.get(Textfield.class, "bssid"+myId).setColorBackground(orangecolor);
      cp5.get(Textfield.class, "bssid"+myId).setText(" AP2");
    } else {
      cp5.get(Textfield.class, "bssid"+myId).setColorBackground(greycolor);
    }

    cp5.get(Textfield.class, "firmware"+myId).setText(str(firmware));
    //cp5.get(Toggle.class, "toggleAlive"+myId).setColorBackground(greencolor);
    //muteOutput(mute);
    myIP = ip;
    
    // LOG for test
    /*csv_count_batt ++;
    if (csv_count_batt % 100 == 0) {
      String timeString = nf(hour(), 2) + ":" + nf(minute(), 2) + ":" + nf(second(), 2);
      String csv = str(myId)+"," +timeString+ "," + str(batterylevel);
      myUdpIOManager.appendTextToFile("battery_log_"+str(myId)+".csv", csv);
    }*/
  }
  public void setAutoMode(int value) {
    if (value == 1) {
      cp5.get(Button.class, "tilt"+myId).setColorBackground(greencolor);
    } else {
      cp5.get(Button.class, "tilt"+myId).setColorBackground(greycolor);
    }
  }

  public void heartbeat() {
    aliveUpdate = millis(); //store last update from board
    isUp = true;
    update();
  }
  public void toggleOutput(boolean value) {
    cp5.get(Toggle.class, "toggleOutput"+myId).setState(value);
  }
  public void muteOutput(boolean value) {
    cp5.get(Toggle.class, "muteOutput"+myId).setState(value);
  }
  public void controlEvent(ControlEvent theEvent) {
    if (theEvent.getController().getName().equals("bang"+myId)) {
      //println("bang"+myId);
      bang();
    }
    if (theEvent.getController().getName().equals("wifi_reconnect"+myId)) {
      sendRestartWifi();
    }

    if (theEvent.getController().getName().equals("toggleOutput"+myId)) {
      //println("toggle"+myId);
      if (theEvent.getController().getValue() == 1) {
        //bang();
        udpOutput = true;
      } else {
        udpOutput = false;
      }
    }
    if (theEvent.getController().getName().equals("tilt"+myId)) {
      sendToggleTilt();
    }
        if (theEvent.getController().getName().equals("deepsleep"+myId)) {
      sendToggleDeepSleep();
    }
  }
  public void sendToggleTilt() {
    //println("..");
    if (debugConsole) {
      println("toggle Tilt requested for "+myId +"("+myBellNumber+")");
    }
    myUdpIOManager.sendMessage("T", myIP);
  }

  public void sendToggleDeepSleep() {
    //println("..");
    if (debugConsole) {
      println("toggle Deeplsleep requested for "+myId +"("+myBellNumber+")");
    }
    myUdpIOManager.sendMessage("Z", myIP);
  }


  public void sendRestartWifi() {
    if (debugConsole) {
      println("wifi restart requested for "+myId +"("+myBellNumber+")");
    }
    myUdpIOManager.sendMessage("R", myIP);
  }
  public void bang() {
    cp5.get(Button.class, "bang"+myId).setColorBackground(redcolor);
    if (udpOutput && isUp) {
      myUdpIOManager.sendMessage("B", myIP);
    }
    //myUdpIOManager.sendMessage("yy", myIP);
  }
}
