public class MidiManager {
  int channel = 2;
  int pitch = 64;
  int velocity = 127;
  int number = 0;
  int value = 90;
  int good_midi_input = 0;
  String good_midi_input_name = "undefined";

  MidiManager(PApplet p) {
    //init constructor
    println("init MIDI Manager");
    MidiInputDevice devices[] = RWMidi.getInputDevices();
    for (int i = 0; i < devices.length; i++) {
      println(i + ": " + devices[i].getName());
      if (devices[i].getName().indexOf("RTC") != -1) { // Auto select the RTC input
        good_midi_input = i;
        good_midi_input_name = devices[i].getName();
      }
    }
    //p.registerMethod("noteOnReceived", this);
    //p.registerMethod("noteOffReceived", this);
    //mymididevice = RWMidi.getInputDevices()[1].createInput(this); // select device
    mymididevice = RWMidi.getInputDevices()[good_midi_input].createInput(this); // select device
    println ("SELECTED MIDI INPUT : " + good_midi_input + " - " + good_midi_input_name);
    println ("MIDI CHANNEL : 2");
  }
  //Note ON recieved
  public void noteOnReceived(Note myreceivednote) {
    //println("note on " + myreceivednote.getChannel() + "  " + myreceivednote.getPitch()+ "  " + myreceivednote.getVelocity());
    if (myreceivednote.getChannel() == channel - 1){ // we make -1 because channel index start at 0 in library...
      // limit to messages from channel 2
    myBellManager.playNote(myreceivednote.getPitch());
    }
  }

  // Note Off recieved
  public void noteOffReceived(Note myreceivednote) {
    //println("note off " + myreceivednote.getChannel() + "  " + myreceivednote.getPitch()+ "  " + myreceivednote.getVelocity());
  }
  // Control Change recieved
  public void controllerChangeReceived(rwmidi.Controller cc) {
    //println("cc channell is: " + cc.getChannel() + "  " + "cc number is: " + cc.getCC() + "  " + "cc value is:  " + cc.getValue() );
  }
}
