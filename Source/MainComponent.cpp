#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "./OSCInterface.h"

class MainContentComponent : public AudioAppComponent,
			     public ChangeListener,
			     public Button::Listener,
			     private Label::Listener,
			     public Slider::Listener
{
public:
  MainContentComponent() : state (Stopped),
			   sampleRate (44100),
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
    // Create TransportSources and Labels for all the files
    for(int i = 0; i < maxNumberOfFiles; ++i) {
      transports.add(new AudioTransportSource());
      transports[i]->addChangeListener(this);
      mapper.add(new ChannelRemappingAudioSource(transports[i], false));
      mapper[i]->setNumberOfChannelsToProduce(2);
      ////////////////////////////////////////
      // Default mapping should go here.
      //      mapper[i]->setOutputChannelMapping(0, 0);
      //      mapper[i]->setOutputChannelMapping(1, 1);
    }

    ////////////////////////////////////////
    // Register listeners
    deviceManager.addChangeListener (this);
    formatManager.registerBasicFormats();

    ////////////////////////////////////////
    // Set up audio and load files
    setAudioChannels (2, 2);
    File dir("~/rosenberg/audio");
    files = dir.findChildFiles(2, false, "*.wav");
    for(int i = 0; i < files.size(); i++)
      std::cout << files[i].getFileName() << std::endl;

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
      mixer.addInputSource(mapper[i], false);

	//    	transports[i]->prepareToPlay(samplesPerBlockExpected, sR);
	//    	mixer.addInputSource(transports[i], false);
    }
    mixer.prepareToPlay(samplesPerBlockExpected, sR);
    ////////////////////////////////////////
    // Mapper
    //    mapper->prepareToPlay (samplesPerBlockExpected, sR);
    //    transports[0]->prepareToPlay(samplesPerBlockExpected, sR);
    sampleRate = sR;
	
    //    transports[0]->prepareToPlay(samplesPerBlockExpected, sR);
    //    mixer.addInputSource(transports[0], false);
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
    transports[0]->releaseResources();
    sampleRate = 0;
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
    if (source == transports[0])
      {
	if (transports[0]->isPlaying())
	  changeState(Playing);
	else
	  changeState(Stopped);
      }
    dumpDeviceInfo();
  }

  void buttonClicked (Button* button) override
  {
    int numOfSourceChannels = 2;
    int outOfBounds = 256;
    if (button == &openButton)  openButtonClicked();
    if (button == &playButton)  playButtonClicked();
    if (button == &stopButton)  stopButtonClicked();

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
	    setPosition(0.0);
	    break;
                    
	  case Starting:                        
	    playButton.setEnabled (true);
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
      if(i < maxNumberOfFiles) {
	transports[i]->start();
      }
    }
  }

  void setPosition(float pos)
  {
    for(int i = 0; i < files.size(); ++i) {
      if(i < maxNumberOfFiles)
	transports[i]->setPosition(pos);
    }
  }

  void openButtonClicked()
  {
    int numOfFiles = files.size();
    int f = std::rand()%numOfFiles;
    double length = 0;
    int numOfSourceChannels = 2;
    int startYPos = yPosInterface+80;
    int startXPos = xPosInterface;
    int columnWidth = 50;
    int rowHeight = 20;
    int calcYPos = startYPos;
    String label = "Ch.";
    // Make headers for the output channel numbers
    for(int g = 0; g<getNumberOfHardwareOutputs(); g++) {
      addAndMakeVisible(channelNames[g]);
      channelNames[g].setText(label+std::to_string(g+1), dontSendNotification);
      channelNames[g].setFont(Font ("Geneva", 12.0f, Font::plain));
      channelNames[g].setBounds(startXPos+(columnWidth*g)-3, startYPos-20, 40, 20);
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
  int maxNumberOfFiles = 18;
  OwnedArray<AudioTransportSource> transports;
  OwnedArray<AudioFormatReaderSource> readerSources;
  OwnedArray<ChannelRemappingAudioSource> mapper;
  
  MixerAudioSource mixer;
  TransportState state;
  Array<File> files;
  int fileIndex = 0;
  int channelsInFile = 0;
  const static int maxChannels = 128;
  const static int maxFiles = maxChannels;
  const static int sourceChannels = 16;
  int channelsPerFile[maxChannels];

  // GUI
  TextButton openButton;
  TextButton playButton;
  TextButton stopButton;
  Label titleLabel;
  OwnedArray<Label> fileNameLabels;
  String dataDir;
  double sampleRate;
  Slider positionSlider;
  Label  currentPosition;
  int xPosInterface = 168;
  int yPosInterface = 300;
  
  AudioDeviceSelectorComponent audioSetupComp;
  Label cpuUsageLabel;
  Label cpuUsageText;
  Label channelNames[sourceChannels];
  TextEditor diagnosticsBox;
  ToggleButton *routeChannel[maxFiles][sourceChannels][maxChannels];

  OSCInterface osc;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

Component* createMainContentComponent()     { return new MainContentComponent(); }

#endif  // MAINCOMPONENT_H_INCLUDED
