#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class MainContentComponent : public AudioAppComponent,
			     public ChangeListener,
			     public Button::Listener,
			     private Label::Listener,
			     public Slider::Listener
{
public:
  MainContentComponent() : state (Stopped),
			   audioSetupComp (deviceManager,
			   		   0,     // minimum input channels
			   		   maxChannels,   // maximum input channels
			   		   0,     // minimum output channels
			   		   maxChannels,   // maximum output channels
			   		   false, // ability to select midi inputs
			   		   false, // ability to select midi output device
			   		   false, // treat channels as stereo pairs
			   		   false) // hide advanced options

  {
    // Keep for the label.
    // addAndMakeVisible (titleLabel);
    // titleLabel.setFont (Font ("Geneva", 36.0f, Font::bold));
    // titleLabel.setText("Folk Song Lab", dontSendNotification);
    // titleLabel.setColour (Label::textColourId, Colours::white);
    // titleLabel.setJustificationType (Justification::centred);

    /////////////////////////////////////////
    // Interface
    addAndMakeVisible (&playButton);
    playButton.setButtonText ("Play");
    playButton.addListener (this);
    playButton.setColour (TextButton::buttonColourId, Colours::green);
    playButton.setEnabled (false);

    addAndMakeVisible (&stopButton);
    stopButton.setButtonText ("Stop");
    stopButton.addListener (this);
    stopButton.setColour (TextButton::buttonColourId, Colours::red);
    stopButton.setEnabled (false);

    addAndMakeVisible (&openButton);
    openButton.setButtonText ("Load");
    openButton.addListener (this);
    openButton.setColour (TextButton::buttonColourId, Colours::green);
    openButton.setEnabled (true);

    addAndMakeVisible (&conButton);
    conButton.setButtonText ("Connect");
    conButton.addListener (this);
    conButton.setColour (TextButton::buttonColourId, Colours::red);
    conButton.setEnabled (true);

    //////////////////////
    // Diagnostics interface
    addAndMakeVisible (audioSetupComp);
    addAndMakeVisible (diagnosticsBox);

    diagnosticsBox.setMultiLine (true);
    diagnosticsBox.setReturnKeyStartsNewLine (true);
    diagnosticsBox.setReadOnly (true);
    diagnosticsBox.setScrollbarsShown (true);
    diagnosticsBox.setCaretVisible (false);
    diagnosticsBox.setPopupMenuEnabled (true);
    diagnosticsBox.setColour (TextEditor::backgroundColourId, Colour (0x32ffffff));
    diagnosticsBox.setColour (TextEditor::outlineColourId, Colour (0x1c000000));
    diagnosticsBox.setColour (TextEditor::shadowColourId, Colour (0x16000000));

    cpuUsageLabel.setText ("CPU Usage", dontSendNotification);
    cpuUsageText.setJustificationType (Justification::right);
    addAndMakeVisible (&cpuUsageLabel);
    addAndMakeVisible (&cpuUsageText);
    
    setSize (800, 600);

    ////////////////////////////////////////
    // Set up audio and load files
    loadSoundFiles();
    setAudioChannels (2, 2);

    ////////////////////////////////////////
    // Create TransportSources and Labels for all the files
    createTransports();

    ////////////////////////////////////////
    // Register listeners
    deviceManager.addChangeListener (this);
    formatManager.registerBasicFormats();

    ////////////////////////////////////////
    // Set up the file position slider
    addAndMakeVisible(positionSlider);
    positionSlider.setRange(0, 600);
    positionSlider.setTextValueSuffix("sec");
    positionSlider.addListener(this);
    positionSlider.setValue(0);
    positionSlider.setNumDecimalPlacesToDisplay(2);

    addAndMakeVisible(currentPosition);
    currentPosition.setText("Position:", dontSendNotification);
    currentPosition.attachToComponent(&positionSlider, true);

    sliderEnabled(false);

    ////////////////////////////////////////
    // Set up OSC
    //    OSCInterface *osc = new OSCInterface(this);
    localOsc = new OSCInterface(this);
    //    localOsc->test();
    //    OSCInterface *t = static_cast<OSCInterface*>(localOsc);
    //    t->test();
  }
    
  ~MainContentComponent()
  {
    //    readerSources.clear();
    //    transports.clear();
    //    deviceManager.removeChangeListener (this);
    //    shutdownAudio();
  }

  void labelTextChanged (Label* label) override
  {
    
  }
  
  void prepareToPlay (int samplesPerBlockExpected, double sR) override
  {
    ////////////////////////////////////////
    // Mixer
    // mixer.addInputSource(&transportSource, false);
    // mixer.addInputSource(&transportSourcei, false);

    for(int i = 0; i < mapper.size(); i++) {
      mixer.addInputSource(mapper[i], true);
    }
    mixer.prepareToPlay(samplesPerBlockExpected, sR);
  }

  void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
  {
    auto* device = deviceManager.getCurrentAudioDevice();
    ////////////////////////////////////////
    // Active channels in hardware
    const BigInteger activeInputChannels = device->getActiveInputChannels();
    const BigInteger activeOutputChannels = device->getActiveOutputChannels();
    const int maxInputChannels = activeInputChannels.getHighestBit() + 1;
    const int maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

    ////////////////////////////////////////
    // Cancel out data in inactive buffer
    // if (readerSource.get() == nullptr)
    // {
    //     bufferToFill.clearActiveBufferRegion();
    //     return;
    // }

    ////////////////////////////////////////
    // Get samples from the file to play back.

    //    transports[0]->getNextAudioBlock(bufferToFill);
    mixer.getNextAudioBlock(bufferToFill);
  }

  void releaseResources() override
  {
    //    mapper.releaseResources();
    //    transports[0]->releaseResources();
    mixer.releaseResources();
  }

  void paint (Graphics& g) override
  {
    g.setColour (Colours::grey);
    g.fillRect (getLocalBounds().removeFromRight (proportionOfWidth (0.4f)));
  }
  
  void resized() override
  {
    auto left = xPosInterface;
    auto vert = yPosInterface;; 
    positionSlider.setBounds (left+1, 268, getWidth() - left - 337, 20);
    
    titleLabel.setBounds(10, 0, 200, 100);
    conButton.setBounds(left-104, vert, 80, 50);
    playButton.setBounds(left, vert, 80, 50);
    stopButton.setBounds(left+104, vert, 80, 50);
    openButton.setBounds(left+207, vert, 80, 50);

    auto rect = getLocalBounds();
    audioSetupComp.setBounds (rect.removeFromLeft (proportionOfWidth (0.6f)));
    rect.reduce (10, 10);

    auto topLine (rect.removeFromTop (20));
    cpuUsageLabel.setBounds (topLine.removeFromLeft (topLine.getWidth() / 2));
    cpuUsageText.setBounds (topLine);
    rect.removeFromTop (20);

    diagnosticsBox.setBounds (rect);

    ////////////////////////////////////////
    // Place the soundfile names in the interface
    int yPos = vert + 60; // Start position
    for(int i=0; i<fileNameLabels.size(); i++) {
      Label *l = fileNameLabels[i];
      int ySpace = 20; // Space between items
      ySpace = ySpace * channelsPerFile[i]; // Adjusted to number of channels in current file
      int ySpaceOffset = 10 * channelsPerFile[i]; //ySpace / 2;
      logMessage(std::to_string(ySpace * channelsPerFile[i]));
      if(l != nullptr) {
	l->setBounds(left-150, (yPos+ySpaceOffset)+(ySpace*i), 140, 50);
	l->setFont(Font ("Geneva", 12.0f, Font::plain));
	l->setJustificationType(Justification::centredRight);
      }
    }
  }
    
  void changeListenerCallback (ChangeBroadcaster* source) override
  {
    int sourcesPlaying = 0;
    for(int i = 0; i < files.size(); ++i) {
      if (source == transports[i]) {
	for(int j = 0; j < files.size(); ++j) {
	  if(transports[j]->isPlaying()) {
	    changeState(Playing);
	    sourcesPlaying += 1;
	    break;
	  }
	}
	if(sourcesPlaying == 0) {
	  changeState(Stopped);
	  dumpDeviceInfo();
	}
	break;
      }
    }
  }

  void buttonClicked (Button* button) override
  {
    int outOfBounds = 256;
    if (button == &openButton)  openButtonClicked();
    if (button == &playButton)  playButtonClicked();
    if (button == &stopButton)  stopButtonClicked();
    if (button == &conButton) conButtonClicked();

    ////////////////////////////////////////
    // Routing happens here
    for(int i = 0; i<files.size(); i++) { // Per sound file
      for(int j = 0;j<channelsPerFile[i]; j++) { // Per channel of sound file
	for(int k = 0; k<getNumberOfHardwareOutputs(); k++) { // Per physical audio output
	  if(button == routeChannel[i][j][k]) {
	    if(button->getToggleState()) { // Switch on routing for node
	      logMessage("--------------------------------------");
	      logMessage("Audio channel "+std::to_string(j)+
			 " of file "+std::to_string(i)+
			 " mapped to output "+std::to_string(k));
	      mapper[i]->setOutputChannelMapping(j, k);
	    }
	    else {
	      mapper[i]->setOutputChannelMapping(j, outOfBounds);
	    }
	  }
	}
      }
    }
  }

private:
  enum TransportState
    {
     Stopped,
     Starting,
     Playing,
     Stopping
    };

  void changeState (TransportState newState)
  {
    if (state != newState)
      {
	state = newState;
            
	switch (state)
	  {
	  case Stopped:                         
	    stopButton.setEnabled (false);
	    playButton.setEnabled (true);
	    //	    sliderEnabled(false);
	    setPosition(0.0);
	    break;
                    
	  case Starting:                        
	    playButton.setEnabled (true);
	    sliderEnabled(true);
	    startSources();
	    break;
                    
	  case Playing:                         
	    stopButton.setEnabled (true);
	    playButton.setEnabled (false);
	    break;
                    
	  case Stopping:
	    for(int i = 0; i < files.size(); ++i) {
	      if(i < maxNumberOfFiles)
		transports[i]->stop();
	    }
	    break;
	  }
      }
  }

  void startSources()
  {
    ScopedLock lock(deviceManager.getAudioCallbackLock());
    for(int i = 0; i < files.size(); ++i) {
      if(i < transports.size()) {
	transports[i]->start();
      }
    }
  }

  void setPosition(float pos)
  {
    for(int i = 0; i < files.size(); ++i) {
      if(i < transports.size())
	transports[i]->setPosition(pos);
    }
  }

  void openButtonClicked()
  {    
    if(files.size() == 0) {
      loadSoundFiles();
      createTransports();
      std::cout << "files: " << String(files.size()) << std::endl;
      std::cout << "transports: " << String(transports.size()) << std::endl;
    }
    if(files.size() > 0 && sema == 0) {
      double length = 0;
      // Geometry
      int startYPos = yPosInterface+80;
      int startXPos = xPosInterface;
      int columnWidth = 50;
      int rowHeight = 20;
      int calcYPos = startYPos;
      String label = "Ch.";
      // Make headers for the output channel numbers
      for(int g = 0; g<getNumberOfHardwareOutputs(); g++) {
	if(channelNames[g].isVisible()) {
	  addAndMakeVisible(channelNames[g]);
	  channelNames[g].setText(label+std::to_string(g+1), dontSendNotification);
	  channelNames[g].setFont(Font ("Geneva", 12.0f, Font::plain));
	  channelNames[g].setBounds(startXPos+(columnWidth*g)-3, startYPos-20, 40, 20);
	}
      }
      // Print the filenames
      //    int rows = files.size() * 
      for(int i = 0; i < files.size(); i++) {
	if(i < maxNumberOfFiles) {
	  createReader(files[i], i);
	  if(length < transports[i]->getLengthInSeconds())
	    length = transports[i]->getLengthInSeconds();
	  logMessage(files[i].getFileName());
	  fileNameLabels.add(new Label(files[i].getFileName(), files[i].getFileName()));
	  addAndMakeVisible(fileNameLabels[i]);

	  ////////////////////////////////////////
	  // Make channel mapping interface
	  for(int j = 0; j<channelsPerFile[i]; j++) {
	    calcYPos = calcYPos+(rowHeight*j);
	    for(int k = 0;k<getNumberOfHardwareOutputs(); k++) {
	      logMessage(std::to_string(j)+"-"+std::to_string(k)+"-"+std::to_string(calcYPos));
	      ToggleButton *b = new ToggleButton();
	      addAndMakeVisible(b);
	      b->setBounds(startXPos+(columnWidth*k), calcYPos, 30, 30);
	      b->setTooltip("Route channel "+std::to_string(j)+" to output "+std::to_string(k));
	      b->addListener(this);
	      routeChannel[i][j][k] = b;
	    }
	  }
	  calcYPos += rowHeight;
	}
      }
      resized();
      sliderSetRange(0, length);
      sliderEnabled(true);
      playButton.setEnabled (true);
      openButton.setButtonText ("Unload");
      sema = 1;
    }
    else {
      unloadAudioFiles();
      openButton.setButtonText ("Load");
      sema = 0;
    }
  }

  void createTransports()
  {
    for(int i = 0; i < files.size(); ++i) {
      transports.add(new AudioTransportSource());
      transports[i]->addChangeListener(this);
      mapper.add(new ChannelRemappingAudioSource(transports[i], false));
      mapper[i]->setNumberOfChannelsToProduce(2);
      ////////////////////////////////////////
      // Default mapping should go here.
      //      mapper[i]->setOutputChannelMapping(1, 1);
    }
  }
  
  void loadSoundFiles()
  {
    File dir("~/rosenberg/audio");
    files = dir.findChildFiles(2, false, "*.wav");   
    for(int i = 0; i < files.size(); i++)
      std::cout << files[i].getFileName() << std::endl;
  }
    
  void unloadAudioFiles()
  {
    for(int h = 0; h<transports.size();h++) {
      transports[h]->removeChangeListener(this);
    }
    // Clear the file names off the GUI
    fileNameLabels.clear();
    // Clear the ceckboxes in the GUI
    for(int i = 0; i < files.size(); i++) {
      for(int j = 0; j<channelsPerFile[i]; j++) {
	for(int k = 0;k<getNumberOfHardwareOutputs(); k++) {
	  delete routeChannel[i][j][k];
	}
      }
      playButton.setEnabled (false);
    }
    // Clear the headers off of the GUI
    for(int g = 0; g<getNumberOfHardwareOutputs(); g++) {
      channelNames[g].setVisible(false);
    }
    //Clear all the sources loaded in the mixer
    mixer.removeAllInputs();
    std::cout << "transports first size:" << std::endl;
    std::cout << std::to_string(transports.size()) << std::endl;
    std::cout << "mapper first size:" << std::endl;
    std::cout << std::to_string(mapper.size()) << std::endl;
    // Clear the reader sources
    readerSources.clear();
    //Empty the files Array
    files.clear();
    // Clear mappers
    mapper.clear(true);
    // Clear the transports
    transports.clear(false);
    
    std::cout << "transports size:" << std::endl;
    std::cout << std::to_string(transports.size()) << std::endl;
    std::cout << "readerSources size:" << std::endl;
    std::cout << std::to_string(readerSources.size()) << std::endl;
    std::cout << "files size:" << std::endl;
    std::cout << std::to_string(files.size()) << std::endl;

    sliderEnabled(false);
  }
  
  void playButtonClicked()
  {
    changeState(Starting);
  }

  void createReader(const File &file, int i)
  {
    AudioFormatReader* reader;
    reader = formatManager.createReaderFor(file);
    channelsPerFile[i] = reader->numChannels;
    logMessage("Channels for file: "+std::to_string(channelsPerFile[i]));
    if (reader != nullptr)
      {
	ScopedPointer<AudioFormatReaderSource> source = new AudioFormatReaderSource(reader, true);
	transports[i]->setSource(source, 0, nullptr, reader->sampleRate);
	readerSources.insert(i, source.release());
      }
  }
  
  void stopButtonClicked()
  {
    changeState (Stopping);
  }

  void conButtonClicked()
  {
    OSCInterface *osc = static_cast<OSCInterface*>(localOsc);
    int x = osc->connectButtonClicked();
    //    std::cout << std::to_string(x) << std::endl;
    if(x > 0) {
      conButton.setButtonText("Disconnect");
      conButton.setColour (TextButton::buttonColourId, Colours::green);
    }
    else {
      conButton.setButtonText("Connect");
      conButton.setColour (TextButton::buttonColourId, Colours::red);
    }
  }

  // void setConButtonText()
  // {
  //   conButton.setButtonText(
  // 	b->setButtonText("Disconnect");
  // }
  
  static String getListOfActiveBits (const BigInteger& b)
  {
    StringArray bits;
    
    for (auto i = 0; i <= b.getHighestBit(); ++i)
      if (b[i])
	bits.add (String (i));
    
    return bits.joinIntoString (", ");
  }

  void dumpDeviceInfo()
  {
    logMessage ("--------------------------------------");
    logMessage ("Current audio device type: " + (deviceManager.getCurrentDeviceTypeObject() != nullptr
						 ? deviceManager.getCurrentDeviceTypeObject()->getTypeName()
						 : "<none>"));

    if (auto* device = deviceManager.getCurrentAudioDevice())
      {
	logMessage ("Current audio device: "   + device->getName().quoted());
	logMessage ("Sample rate: "    + String (device->getCurrentSampleRate()) + " Hz");
	logMessage ("Block size: "     + String (device->getCurrentBufferSizeSamples()) + " samples");
	logMessage ("Bit depth: "      + String (device->getCurrentBitDepth()));
	logMessage ("Input channel names: "    + device->getInputChannelNames().joinIntoString (", "));
	logMessage ("Active input channels: "  + getListOfActiveBits (device->getActiveInputChannels()));
	logMessage ("Output channel names: "   + device->getOutputChannelNames().joinIntoString (", "));
	logMessage ("Active output channels: " + getListOfActiveBits (device->getActiveOutputChannels()));
      }
    else
      {
	logMessage ("No audio device open");
      }
    logMessage((String)getNumberOfHardwareOutputs());
  }

  int getNumberOfHardwareOutputs()
  {
    auto* device = deviceManager.getCurrentAudioDevice();
    const BigInteger ch = device->getActiveOutputChannels();
    int activeChannels = ch.getHighestBit() + 1;
    if(activeChannels > maxChannels)
      return maxChannels;
    else 
      return activeChannels;
  }
  
  void logMessage (const String& m)
  {
    diagnosticsBox.moveCaretToEnd();
    diagnosticsBox.insertTextAtCaret (m + newLine);
  }
  //==========================================================================

  void sliderValueChanged(Slider *slider) override
  {
  }
  
  void 	sliderDragStarted (Slider *slider) override
  {

  }

  void sliderDragEnded (Slider *slider) override
  {
    if(slider == &positionSlider) {
      setPosition(slider->getValue());
    }
  }

  void sliderSetRange(double min, double max)
  {
    positionSlider.setRange(min, max);
    positionSlider.setNumDecimalPlacesToDisplay(2);
  }
  
  void sliderEnabled(bool value)
  {
    positionSlider.setEnabled(value);
    currentPosition.setEnabled(value);
  }
  //==========================================================================

  // Audio
  AudioFormatManager formatManager;
  OwnedArray<AudioTransportSource> transports;
  OwnedArray<AudioFormatReaderSource> readerSources;
  OwnedArray<ChannelRemappingAudioSource> mapper;
  
  MixerAudioSource mixer;
  TransportState state;
  Array<File> files;
  int fileIndex = 0;
  int channelsInFile = 0;
  const static int maxChannels = 128;
  const static int maxNumberOfFiles = maxChannels;
  const static int sourceChannels = 16;
  int channelsPerFile[maxChannels];

  // GUI
  TextButton openButton;
  TextButton playButton;
  TextButton stopButton;
  TextButton conButton;
  Label titleLabel;
  OwnedArray<Label> fileNameLabels;
  String dataDir;
  Slider positionSlider;
  Label  currentPosition;
  int xPosInterface = 168;
  int yPosInterface = 300;
  int sema = 0;
  
  AudioDeviceSelectorComponent audioSetupComp;
  Label cpuUsageLabel;
  Label cpuUsageText;
  Label channelNames[maxChannels];
  TextEditor diagnosticsBox;
  ToggleButton *routeChannel[maxNumberOfFiles][sourceChannels][maxChannels];
  void* localOsc;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)

  class OSCInterface : private OSCReceiver,
		       private OSCReceiver::ListenerWithOSCAddress<OSCReceiver::MessageLoopCallback>
  {
    //==============================================================================
    
  public:
    OSCInterface(MainContentComponent *m) : receivePort(9005),
					    sendPort(9000)
    {
      main = m;

      rFileName = new OSCAddress("/player/file");
      rPlay = new OSCAddress("/player/play");
      rStop = new OSCAddressPattern("/player/stop");
      if (! isValidOscPort(receivePort)) {
	handleInvalidPortNumberEntered();
	return;
      }
      if (connect(receivePort)) {
	std::cout << "Ready to receive osc" << std::endl;
      }
       else {
	 showConnectionErrorMessage("Error: could not connect to UDP porttt "+String(receivePort));
       }
      addListener(this, *rPlay);
      addListener(this, *rFileName);
    }
    
    ~OSCInterface() {}
    
    //==============================================================================

    void test()
    {
      std::cout << "Funktar" << std::endl;
    }

    int connectButtonClicked()
    {
      if(!isConnected()) {
	connectToServer();
	if (! oscSend.send ("/player/message", (String)"Yep, I'm connected now."))
	  showConnectionErrorMessage ("Error: could not send OSC message.");
	currentPortNumber = sendPort;
	return 1;
      }
      else {
	if(oscSend.disconnect()) {
	  currentPortNumber = -1;
	  return 0;
	}
	else return -1;
      }
    }

    // create and send an OSC message with an address and a float value:

  private:
    void connectToServer()
    {
      if (! isValidOscPort(sendPort)) {
	handleInvalidPortNumberEntered();
	return;
      }
      if (! oscSend.connect (oscAddress, sendPort))
	showConnectionErrorMessage ("Error: could not connect to UDP port"+String(sendPort));
    }

    void handleConnectError (int failedPort)
    {
      AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
					"OSC Connection error",
					"Error: could not connect to port " + String (failedPort),
					"OK");
    }

    void showConnectionErrorMessage (const String& messageText)
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          "Connection error",
                                          messageText,
                                          "OK");
    }
    
    void handleInvalidPortNumberEntered()
    {
      AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
					"Invalid port number",
					"Error: you have entered an invalid UDP port number.",
					"OK");
    }
    
    bool isValidOscPort (int port) const
    {
      return port > 0 && port < 65536;
    }

    bool isConnected() const
    {
        return currentPortNumber != -1;
    }
    
    void oscMessageReceived (const OSCMessage& message) override
    {
      // Get file name.
      // Responds to /player/file s "MySoundfile.waw"
      if (message.size() == 1 && message[0].isString()) {
	if(rFileName->toString().compare(message.getAddressPattern().toString()) == 0)
	  for(int i = 0; i < message.size(); i++) {
	    std::cout << message[i].getString() << std::endl;
	  }
      }
      // Start or stop olayback.
      // REsponds to /player/play i 1|0
      if (message.size() == 1 && message[0].isInt32()) {
	if(rPlay->toString().compare(message.getAddressPattern().toString()) == 0)
	  for(int i = 0; i < message.size(); i++) {
	    std::cout << message[i].getInt32() << std::endl;
	  }
      }
    }
         
    //==============================================================================
    int receivePort;
    int sendPort;
    int currentPortNumber = -1;
    const OSCAddress *rFileName;
    OSCAddress *rPlay;
    OSCAddressPattern *rStop;
    OSCSender oscSend;
    MainContentComponent *main;
    String oscAddress = "127.0.0.1";
  };
};

Component* createMainContentComponent()     { return new MainContentComponent(); }

#endif  // MAINCOMPONENT_H_INCLUDED
