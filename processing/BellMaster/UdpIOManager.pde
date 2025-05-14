public class UdpIOManager {

  String message;
  String ip;
  int portIN;
  int portOUT;

  UDP myUdp;  // define the UDP object
  int counter;

  UdpIOManager() {
    //init constructor
    println("init UDP Manager");
    // UDP PART
    // create a new datagram connection on port 6000
    // and wait for incomming message
    myUdp = new UDP(this, 8888 );
    //myUdp.log( true );     // <-- printout the connection activity
    myUdp.listen( true );

    message  = "";
    //IPArray = append(IPArray, "192.168.1.33");
    //IPArray = append(IPArray, "192.168.1.34");
    //IPArray = append(IPArray, "192.168.1.35");
    //IPArray = append(IPArray, "192.168.1.36");
    //IPArray = append(IPArray, "192.168.1.37");
    //IPArray = append(IPArray, "192.168.1.38");
    //IPArray = append(IPArray, "192.168.1.39");
    portOUT        = 9999;    // the destination port
    portIN        = 8888;    // the incoming port
    myIPBroadcast = "192.168.0.255"; // modifié, était "192.168.0.255" au Belluard
    println("IP Broadcast: "+myIPBroadcast);
    //udp.broadcast(true);
    // formats the message for Pd
    //message = message+";\n";
    counter = 0;
  }

  void receive( byte[] data, String ip, int port) {  // <-- extended handler
    // get the "real" message =
    // forget the ";\n" at the end <-- !!! only for a communication with Pd !!!
    if (data.length > 1) {
      data = subset(data, 0, data.length-2);
      String message = new String( data );
      String params[] = split(message, ",");
      // check for kind of message
      if (debugConsole) {
        // println("--"+message);
        // println(params[0]);
      }


      if (params[0].equals("S")) { // status message update
        //println(message);
        myBellManager.setStatus(int(params[3]), params[1], int(params[2]), int(params[4]), (params[5]), int(params[6]), int(params[5]));
        //println(params[1]);
      }
      if (params[0].equals("T")) { // status message update
        //println(message);
        int id =  int(params[1]);
        int value =  int(params[2]);
        myBellManager.bells[id].setAutoMode(value);
        //println(params[1]);
      }
      if (params[0].equals("P")) { // Ping back message
        //println(message);
        //myBellManager.heartbeat(int params[2], params[1]);
        if (params.length > 1) {
          int id =  int(params[1]);
          myBellManager.bells[id].heartbeat();
        }
        //println(params[1]);
      }
    }

    // print the result
    //println( "receive: \""+message+"\" from "+ip+" on port "+port + "local seq " + counter);
  }
  void sendMessage(String message, String ip) {
    //println("send");
    myUdp.send( message, ip, portOUT );
  }

  public void sendBroadcastMessage(String message) {
    // send a broadcast Request
    //println("send Broadcast Message "+message);
    myUdp.send( message, myIPBroadcast, portOUT );
  }
  //public void sendBellTrigger() {
  //  // send a broadcast Request
  //  //println("send");
  //  myUdp.send( "010", myIPBroadcast, portOUT );
  //}




  /**
   * Appends text to the end of a text file located in the data directory,
   * creates the file if it does not exist.
   * Can be used for big files with lots of rows,
   * existing lines will not be rewritten
   */
  void appendTextToFile(String filename, String text) {
    File f = new File(dataPath(filename));
    if (!f.exists()) {
      createFile(f);
    }
    try {
      PrintWriter out = new PrintWriter(new BufferedWriter(new FileWriter(f, true)));
      out.println(text);
      out.close();
    }
    catch (IOException e) {
      e.printStackTrace();
    }
  }

  /**
   * Creates a new file including all subfolders
   */
  void createFile(File f) {
    File parentDir = f.getParentFile();
    try {
      parentDir.mkdirs();
      f.createNewFile();
    }
    catch(Exception e) {
      e.printStackTrace();
    }
  }
}
