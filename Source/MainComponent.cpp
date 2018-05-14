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

    setSize (1200, 600);

    ////////////////////////////////////////
    // Set up audio and load files
    setAudioChannels (2, maxChannels);
    //    loadSoundFiles();

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
    localOsc = new OSCInterface(this);
    //std::unique_ptr<OSCInterface> t (new OSCInterface(this));
    
    //    OSCInterface *osc = new OSCInterface(this);
    //    localOsc->test();
    //    OSCInterface *t = static_cast<OSCInterface*>(localOsc);
    //    t->test();

  }
    
  ~MainContentComponent()
  {
    formatManager.clearFormats();
    //    localOsc = nullptr;
    OSCInterface *osc = static_cast<OSCInterface*>(localOsc);
    delete osc;
    localOsc = nullptr;
    readerSources.clear();
    transports.clear();
    files.clear();
    deviceManager.removeChangeListener (this);
    shutdownAudio();
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
    mixer.releaseResources();
  }

  void paint (Graphics& g) override
  {
        g.setColour (Colours::grey);
        //g.fillRect(getLocalBounds().removeFromRight(proportionOfWidth (0.4f)).withTrimmedBottom(180));
  }
  
  void resized() override
  {
    auto left = xPosInterface+76;
    auto vert = yPosInterface; // 360
    positionSlider.setBounds (left+-117, yPosInterface-40, 351, 20);
    
    titleLabel.setBounds(10, 0, 200, 100);
    conButton.setBounds(left-117, vert, 80, 50);
    playButton.setBounds(left-28, vert, 80, 50);
    stopButton.setBounds(left+61, vert, 80, 50);
    openButton.setBounds(left+150, vert, 80, 50);
// audioSetupComp.setItemHeight(23); 
        audioSetupComp.setBounds(0, 13, 500, 180);    
       
    auto rect = getLocalBounds();
    //audioSetupComp.setBounds(rect.removeFromLeft (480));
    rect.removeFromLeft (480);
    rect.reduce (14, 10);

    // auto topLine (rect.removeFromTop (0));
    // rect.removeFromTop (0);

    diagnosticsBox.setBounds(488, 27, 300, 388);

    ////////////////////////////////////////
    // Place the soundfile names in the interface
    int yPos = vert + 70; // Start position
    for(int i=0; i<fileNameLabels.size(); i++) {
      Label *l = fileNameLabels[i];
      int ySpace = 20*channelsPerFile[i]+5; // Space between items
      //      logMessage("Ch.names:"+std::to_string(ySpace * channelsPerFile[i]+5));
      if(l != nullptr) {
    	l->setBounds(left-260, yPos, 140, 50);
    	l->setFont(Font ("Geneva", 12.0f, Font::plain));
    	l->setJustificationType(Justification::centredRight);
	yPos += ySpace;
      }
    }

  }
    
  void changeListenerCallback (ChangeBroadcaster* source) override
  {
    if(source == &deviceManager)
      logMessage("Yes");
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

  enum SpeakerSetup
    {
     Dome,
     Studio114,
     Studio118
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
	std::cout << "Starting source: " << String() << std::endl;
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
      std::cout << "files: " << String(files.size()) << std::endl;
      std::cout << "transports: " << String(transports.size()) << std::endl;
    }
    if(files.size() > 0 && sema == 0) {
      std::cout << "are we here: " << std::endl;
      double length = 0;
      // Geometry
      int startYPos = yPosInterface+80;
      int startXPos = xPosInterface-30;
      int columnWidth = 50;
      int rowHeight = 20;
      int spaceBetwFiles = 5;
      int calcYPos = startYPos;
      String label = "Ch.";
      // Make headers for the output channel numbers
      for(int g = 0; g<getNumberOfHardwareOutputs(); g++) {
	addAndMakeVisible(channelNames[g]);
	channelNames[g].setText(label+std::to_string(g+1), dontSendNotification);
	channelNames[g].setFont(Font ("Geneva", 12.0f, Font::plain));
	channelNames[g].setBounds(startXPos+(columnWidth*g)-3, startYPos-20, 40, 20);
      }
      // Prepare the file names for the interface
      // These are place in the function resized() above.
      for(int i = 0; i < files.size(); i++) {
	if(i < maxNumberOfFiles) {
	  createReader(files[i], i);
	  if(length < transports[i]->getLengthInSeconds())
	    length = transports[i]->getLengthInSeconds();
	  //	  logMessage(files[i].getFileName());
	  fileNameLabels.add(new Label(files[i].getFileName(), files[i].getFileName()));
	  addAndMakeVisible(fileNameLabels[i]);

	  ////////////////////////////////////////
	  // Make channel mapping interface
	  for(int j = 0; j<channelsPerFile[i]; j++) {
	    for(int k = 0;k<getNumberOfHardwareOutputs(); k++) {
	      //	      logMessage(std::to_string(j)+"-"+std::to_string(k)+"-"+std::to_string(calcYPos));
	      ToggleButton *b = new ToggleButton();
	      addAndMakeVisible(b);
	      b->setBounds(startXPos+(columnWidth*k), calcYPos, 30, 30);
	      b->setTooltip("Route channel "+std::to_string(j)+" to output "+std::to_string(k));
	      b->addListener(this);
	      routeChannel[i][j][k] = b;
	    }
	    calcYPos = calcYPos+(rowHeight);
	  }
	  calcYPos += spaceBetwFiles;
	}
      }
      //      createDefaultMapping();
      //      createStereoMapping(1, 90);
      resized();
      sliderSetRange(0, length);
      sliderEnabled(true);
      playButton.setEnabled (true);
      openButton.setButtonText ("Unload");
      sema = 1;
    }
    else {
      std::cout << "calling unload" << std::endl;
      unloadAudioFiles();
      openButton.setButtonText ("Load");
      sema = 0;
    }
  }

  /**
   *  Create a default linear routing for loaded soundfiles.
   */
  void createDefaultMapping() {
    int runningOutputCount = 0;
    int numberOfOutputs = getNumberOfHardwareOutputs();
    int localOutput = 0;
    if(files.size() > 0) {
      for(int f = 0; f < files.size(); f++) {
	for(int s = 0; s < channelsPerFile[f]; s++) {
	  //	    std::cout << "Output count: " << String(runningOutputCount%numberOfOutputs) << std::endl;
	  //	    std::cout << "Mapper: " << String(f)+"-" << String(s)+"-" << String(runningOutputCount%numberOfOutputs) << std::endl;
	    localOutput = runningOutputCount%numberOfOutputs;
	    routeChannel[f][s][localOutput]->setToggleState(true, sendNotification);
	    runningOutputCount++;
	}
      }
    }
  }

  /**
   *  Put the sound of the file in fileIndex at position 
   *  centerPosition.
   */
  void createStereoMapping(int fileIndex, int centerPosition, float radius) {
    std::pair<int, int> stereoPair;
    calculatePosition(centerPosition, radius, &stereoPair, Dome);
    if(fileIndex < files.size()) {
      // Wrap position around the actual number of speakers.
      int spkrL = stereoPair.first%getNumberOfHardwareOutputs();
      int spkrR = stereoPair.second%getNumberOfHardwareOutputs();
      routeChannel[fileIndex][0][spkrL]->setToggleState(true, sendNotification);
      routeChannel[fileIndex][1][spkrR]->setToggleState(true, sendNotification);
      std::cout << "First: " << String(stereoPair.first) << std::endl;
      std::cout << "Second: " << String(stereoPair.second) << std::endl;
    }
  }

  void calculatePosition(int angle, float radius, std::pair<int, int> *spkr, SpeakerSetup s) {
    int lowerRing = 16;
    int middleRing = 8;
    int topRing = 4;
    float degreesPerSpeakerLow = 360/lowerRing;
    float degreesPerSpeakerMid = 360/middleRing;
    float degreesPerSpeakerTop = 360/topRing;
    float spread = .5;
    float startPositionL = 29.29;
    float startPositionM = 22;
    float startPositionT = 45;
    float adjustedAngle = 0;
    std::cout << "Radius: " << radius << std::endl;
    switch(s) {	       
    case Dome:
      // VOG only
      if(radius == 0) {
	spkr->first += 28;
	spkr->second += 28;
	break;
      }
      // Upper ring
      else if(radius > 0 && radius < 0.33333) {
	std::cout << "Upper" << radius << std::endl;
	adjustedAngle = angle+startPositionT;
	// subtract/add spread to get neighbouring speaker, add 1 (speakers start at 1), wrap to ring.
	spkr->first = (int)((adjustedAngle/degreesPerSpeakerTop))%topRing;
	spkr->second = (int)((adjustedAngle/degreesPerSpeakerTop)+1)%topRing;
	spkr->first += lowerRing+middleRing;
	spkr->second += lowerRing+middleRing;
	break;
      }
      //middle ring
      else if(radius > 0.3333 && radius < 0.6666) {
      	adjustedAngle = angle+startPositionM;
      	std::cout << "Middle" << radius << std::endl;
      	// subtract/add spread to get neighbouring speaker, add 1 (speakers start at 1), wrap to ring.
      	spkr->first = (int)(adjustedAngle/degreesPerSpeakerMid)%middleRing;
      	spkr->second = (int)((adjustedAngle/degreesPerSpeakerMid)+1)%middleRing;
      	spkr->first += lowerRing;
      	spkr->second += lowerRing;
      	break;
      }
      // lower ring
      else if(radius > 0.6666) {
      	std::cout << "Lower" << radius << std::endl;
      	adjustedAngle = angle+startPositionL;
      	// subtract/add spread to get neighbouring speaker, add 1 (speakers start at 1), wrap to ring.
      	spkr->first = (int)((adjustedAngle/degreesPerSpeakerLow)-spread+1)%lowerRing; 
      	spkr->second = (int)((adjustedAngle/degreesPerSpeakerLow)+spread+1)%lowerRing;
      	break;
      }
    }
  }
  
  /**
   * Create the transport and mapper objects that hold the file reader object
   * and eventually gets added to the corresponding mapper object. 
   */
  void createTransports()
  {
    for(int i = 0; i < files.size(); ++i) {
      transports.add(new AudioTransportSource());
      transports[i]->addChangeListener(this);
      mapper.set(i, new ChannelRemappingAudioSource(transports[i], true));
      mapper[i]->setNumberOfChannelsToProduce(2); // Fix this
      mixer.addInputSource(mapper[i], true);
    }
  }

  /**
   * Create the transport and mapper object for this particular
   * index.
   */
  void createTransportFor()
  {
    if(transports.size() < maxChannels) {
      transports.add(new AudioTransportSource());
      int i = transports.indexOf(transports.getLast());
      transports[i]->addChangeListener(this);
      mapper.set(i, new ChannelRemappingAudioSource(transports[i], true));
      mapper[i]->setNumberOfChannelsToProduce(2); // Fix this
      mixer.addInputSource(mapper[i], true);
    }
    else
      std::cout << "Too many tranports loaded" << std::endl;
  }

  /**
   * Load all soundfiles from the default directory. Only has support for WAV.
   */
  void loadSoundFiles()
  {
    File dir(defaultDirectory);
    files = dir.findChildFiles(2, false, "*.wav");   
    for(int i = 0; i < files.size(); i++)
      std::cout << files[i].getFileName() << std::endl;
    ////////////////////////////////////////
    // Create TransportSources and Labels for all the files
    createTransports();
  }

  /**
   * Load a single soundfile from the default directory.
   * Each newly loaded file will replace all current files.
   */
  void loadSoundFile(const String name)
  {
    if(files.size() > 0)
      unloadAudioFiles();
    File directory(defaultDirectory);
    const String *f = new String(directory.getFullPathName()+"/"+name);
    std::cout << *f << std::endl;
    File *audioFile = new File(*f);
    if(audioFile->exists()) {
      std::cout << " size before: " << String(files.size()) << std::endl;
      files.add(*audioFile);
      std::cout << " size after: " << String(files.size()) << std::endl;
      for(int i = 0; i < files.size(); i++)
	std::cout << files[i].getFileName() << std::endl;
      // Create transports for the file loaded
      createTransports();
      openButtonClicked();
    }
    OSCInterface *osc = static_cast<OSCInterface*>(localOsc);
    osc->sendMessage("/player/stdout", String("The file ")+name+String(" has been loaded"));
    osc = nullptr;
  }

  /** 
   * Append a file at the end of existing files by deleting all items
   * and writing them back in again.
   */
  void appendSoundFileSwap(const String name)
  {
    File directory(defaultDirectory);
    const String *f = new String(directory.getFullPathName()+"/"+name);
    File *audioFile = new File(*f);
    Array<File> n;
    if(audioFile->exists()) {
      if(files.size() > 0) {
	n.addArray(files);
	std::cout << " size files: " << String(files.size()) << std::endl;
	std::cout << " size n: " << String(n.size()) << std::endl;
	unloadAudioFiles();
	files.swapWith(n);
	}
      files.add(*audioFile);
      createTransports();
      openButtonClicked();
    }
  }

  /** 
   * Append a file at the end of existing files.
   */
  void appendSoundFile(const String name)
  {
    File directory(defaultDirectory);
    const String *f = new String(directory.getFullPathName()+"/"+name);
    File *audioFile = new File(*f);
    Array<File> n;
    if(audioFile->exists()) {
      files.add(*audioFile);
      createTransportFor();
      openButtonClicked();
      sema = 0;
    }    
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
    mapper.clear(false);
    // Clear the transports
    transports.clear(false);
    
    std::cout << "transports size:" << std::endl;
    std::cout << std::to_string(transports.size()) << std::endl;
    std::cout << "readerSources size:" << std::endl;
    std::cout << std::to_string(readerSources.size()) << std::endl;
    std::cout << "files size:" << std::endl;
    std::cout << std::to_string(files.size()) << std::endl;
    std::cout << "mapper size:" << std::endl;
    std::cout << std::to_string(mapper.size()) << std::endl;

    openButton.setButtonText ("Load");
    sema = 0;
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
	std::unique_ptr<AudioFormatReaderSource> source (new AudioFormatReaderSource(reader, true));
	transports[i]->setSource(source.get(), 0, nullptr, reader->sampleRate);
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
    osc = nullptr;
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

  /**
   * Get the number of hardware channels currently active.
   */
  int getNumberOfHardwareOutputs()
  {
    auto* device = deviceManager.getCurrentAudioDevice();
    const BigInteger ch = device->getActiveOutputChannels();
    int activeChannels = ch.getHighestBit() + 1;
    // Fix this.
    if(activeChannels > maxChannels) {
      //      setAudioChannels (2, maxChannels);
      return maxChannels;
    }
    else {
      //      setAudioChannels(2, activeChannels);
      return activeChannels;
    }
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
  const static int maxChannels = 64;
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
  int yPosInterface = 360;
  int sema = 0;
  
  AudioDeviceSelectorComponent audioSetupComp;
  Label cpuUsageLabel;
  Label cpuUsageText;
  Label channelNames[maxChannels];
  TextEditor diagnosticsBox;
  ToggleButton *routeChannel[maxNumberOfFiles][sourceChannels][maxChannels];
  void* localOsc;

  String defaultDirectory = "~/lsaudio";
  JUCE_DECLARE_NON_COPYABLE (MainContentComponent)
  //    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)

  class OSCInterface : private OSCReceiver,
		       private OSCReceiver::ListenerWithOSCAddress<OSCReceiver::MessageLoopCallback>
  {
    //==============================================================================
    
  public:
    OSCInterface(MainContentComponent *m) : receivePort(9005),
					    sendPort(9000)
    {
      main = m;

      rFileName = new OSCAddress(oscRoot+"/file");
      aFile = new OSCAddress(oscRoot+"/append");
      rPlay = new OSCAddress(oscRoot+"/play");
      rClear = new OSCAddress(oscRoot+"/clear");
      rPosition = new OSCAddress(oscRoot+"/stereoMap");
      if (! isValidOscPort(receivePort)) {
	handleInvalidPortNumberEntered();
	return;
      }
      if (connect(receivePort)) {
	std::cout << "Ready to receive osc" << std::endl;
      }
       else {
	 showConnectionErrorMessage("Error: could not connect to UDP port "+String(receivePort));
       }
      addListener(this, *rPlay);
      addListener(this, *rFileName);
      addListener(this, *aFile);
      addListener(this, *rClear);
      addListener(this, *rPosition);

      oscSend.reset(new OSCSender());
    }
    
    ~OSCInterface()
    {
      std::cout << "Here" << std::endl;
      delete rFileName;
      delete aFile;
      delete rPlay;
      delete rClear;
      delete rPosition;
      oscSend->disconnect();
      disconnect();
      oscSend = nullptr;
      main = nullptr;
    }
    
    //==============================================================================

    void test()
    {
      std::cout << "Funktar" << std::endl;
    }

    void sendMessage(const String address, const String message)
    {
      if(isConnected()) {
	if (! oscSend->send (address, (String)message)) {
	  showConnectionErrorMessage ("Error: could not send OSC message.");
	}
      }
    }
    
    int connectButtonClicked()
    {
      if(!isConnected()) {
	connectToServer();
	if (! oscSend->send (oscRoot+"/message", (String)"Yep, I'm connected now."))
	  showConnectionErrorMessage ("Error: could not send OSC message.");
	currentPortNumber = sendPort;
	return 1;
      }
      else {
	if(oscSend->disconnect()) {
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
      if (! oscSend->connect (oscAddress, sendPort))
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
      std::cout << message.getAddressPattern().toString() << std::endl;
      // Get file name.
      if (message.size() > 0 && message[0].isString()) {
	// Responds to /lsplayer/file s "MySoundfile.waw"
	if(rFileName->toString().compare(message.getAddressPattern().toString()) == 0) {
	  for(int i = 0; i < message.size(); i++) {
	    main->loadSoundFile(message[i].getString());
	    std::cout << message[i].getString() << std::endl;
	  }
	}
	// Responds to /lsplayer/append s "MySoundfile.waw"
	if(aFile->toString().compare(message.getAddressPattern().toString()) == 0) {
	  for(int i = 0; i < message.size(); i++) {
	    main->appendSoundFileSwap(message[i].getString());
	    std::cout << message[i].getString() << std::endl;
	  }
	}
      }
      // Start or stop playback.
      // Responds to /lsplayer/play i 1|0
      if (message.size() == 1 && message[0].isInt32()) {
	if(rPlay->toString().compare(message.getAddressPattern().toString()) == 0) {
	  if(message[0].getInt32() == 1) {
	    main->playButtonClicked();
	    std::cout << "play" << std::endl;
	  }
	  else {
	    main->stopButtonClicked();
	    std::cout << "stop" << std::endl;
	  }
	}
	// Responds to /lsplayer/clear i 1
	if(rClear->toString().compare(message.getAddressPattern().toString()) == 0) {
	  std::cout << message.getAddressPattern().toString() << std::endl;
	  if(message[0].getInt32() == 1) {
	    main->unloadAudioFiles();
	  }
	}
      }
      // Responds to /lsplayer/stereoMap iif index angle radius
      if(message.size() == 3 && message[0].isInt32() && message[1].isInt32() && message[2].isFloat32()) {
	if(rPosition->toString().compare(message.getAddressPattern().toString()) == 0) {
	  main->createStereoMapping(message[0].getInt32(), message[1].getInt32(), message[2].getFloat32());
	}
      }
    }

    //==============================================================================
    const int receivePort;
    const int sendPort;
    int currentPortNumber = -1;
    const String oscRoot = "/lsplayer";
    const OSCAddress *rFileName; // Add a file
    const OSCAddress *aFile; //Append a file
    const OSCAddress *rPlay; //Play
    const OSCAddress *rClear; //Clear playlist
    const OSCAddress *rPosition; // Position sound
    std::unique_ptr<OSCSender> oscSend;
    MainContentComponent *main;
    String oscAddress = "127.0.0.1";

    // Pointer to the sendOSC callback
    //    typedef void *(MainContentComponent::OSCInterface::*sendOSCCallback)(String, String);
    //    sendOSCCallback sosc = &MainContentComponent::OSCInterface::sendMessage;

  };
};

Component* createMainContentComponent()     { return new MainContentComponent(); }

#endif  // MAINCOMPONENT_H_INCLUDED
