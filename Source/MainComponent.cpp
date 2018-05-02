#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"



class MainContentComponent : public AudioAppComponent,
			     public ChangeListener,
			     public Button::Listener,
			     private Label::Listener
			     //private Timer
{
public:
  MainContentComponent() : state (Stopped),
			   sampleRate (44100),
			   nextSampleNum (0),
			   activeWriter (nullptr),
			   //			   mapper(&transportSource, true),
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
    //backgroundThread.startThread();

    // addAndMakeVisible (titleLabel);
    // titleLabel.setFont (Font ("Geneva", 36.0f, Font::bold));
    // titleLabel.setText("Folk Song Lab", dontSendNotification);
    // titleLabel.setColour (Label::textColourId, Colours::white);
    // titleLabel.setJustificationType (Justification::centred);
        
    addAndMakeVisible (&playButton);
    playButton.setButtonText ("Play");
    playButton.addListener (this);
    playButton.setColour (TextButton::buttonColourId, Colours::green);
    // playButton.setSize(200, 200);
    playButton.setEnabled (true);

    addAndMakeVisible (&stopButton);
    stopButton.setButtonText ("Stop");
    stopButton.addListener (this);
    stopButton.setColour (TextButton::buttonColourId, Colours::red);
    stopButton.setEnabled (false);

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

    deviceManager.addChangeListener (this);
    // Datadirectory
    dataDir = "~/rosenberg";

    setSize (800, 600);
    
    
    formatManager.registerBasicFormats();       // [1]
    //    ts.addChangeListener (this);   // [2]
    transports.add(new AudioTransportSource());
    transports[0]->addChangeListener(this);
    
    // Set the number of channels needed for this source.
    setAudioChannels (2, 2);
    //    mapper.setNumberOfChannelsToProduce(2);
    //recorder.startRecording (file);
    File dir("~/rosenberg/audio");
    files = dir.findChildFiles(2, false, "*.wav");
    for(int i = 0; i < files.size(); i++)
      std::cout << files[i].getFileName() << std::endl;
    int numOfSourceChannels = 2;
    
    // Make channel mapping interface
    int startYPos = 420;
    int startXPos = 100;
    int Xindent = 100;
    int Yindent = 30;
    for(int i = 0; i<numOfSourceChannels; ++i) {
      addAndMakeVisible(channelNames[i]);
      channelNames[i].setText("Channel "+std::to_string(i), dontSendNotification);
      channelNames[i].setBounds(startXPos+(Xindent*i), startYPos-10, 80, 30);
      for(int j = 0;j<getNumberOfHardwareOutputs(); ++j) {
	addAndMakeVisible(routeChannel[i][j]);
	routeChannel[i][j].setButtonText(TRANS("Output "+std::to_string(j)));
	routeChannel[i][j].addListener(this);
	routeChannel[i][j].setBounds(startXPos+(Xindent*i), startYPos+(Yindent*j), 80, 50);
      }
    }

  }
    
  ~MainContentComponent()
  {
    deviceManager.removeChangeListener (this);
    shutdownAudio();
  }

  void labelTextChanged (Label* label) override
  {
  }
  void prepareToPlay (int samplesPerBlockExpected, double sR) override
  {
    // transportSource.prepareToPlay (samplesPerBlockExpected, sR);
    // transportSourcei.prepareToPlay (samplesPerBlockExpected, sR);
    //    mapper.prepareToPlay (samplesPerBlockExpected, sR);
    //    mapper.setInputChannelMapping(0, 1);
    //    mapper.setInputChannelMapping(1, 1);
    //    mapper.setOutputChannelMapping(0, 0);
    //    mapper.setOutputChannelMapping(1, 1);
    // mixer.addInputSource(&transportSource, false);
    // mixer.addInputSource(&transportSourcei, false);
    //    ts.prepareToPlay (samplesPerBlockExpected, sR);
    transports[0]->prepareToPlay(samplesPerBlockExpected, sR);
    sampleRate = sR;
  }

  void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
  {
    auto* device = deviceManager.getCurrentAudioDevice();
    
    const BigInteger activeInputChannels = device->getActiveInputChannels();
    const BigInteger activeOutputChannels = device->getActiveOutputChannels();
    const int maxInputChannels = activeInputChannels.getHighestBit() + 1;
    const int maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

    //    const float** inputChannelData
    //    const ScopedLock sl(writerLock);
    //    std::cout << "output channels: " << maxOutputChannels << std::endl;
    //    std::cout << "input channels: " << maxInputChannels << std::endl;
    // if (readerSource.get() == nullptr)
    // {
    //     bufferToFill.clearActiveBufferRegion();
    //     return;
    // }

    // The play routine
    // Get samples from the file to play back.
    transports[0]->getNextAudioBlock(bufferToFill);
    //    transportSource.getNextAudioBlock(bufferToFill);
  }

  void releaseResources() override
  {
    //    mapper.releaseResources();
    //transportSource.releaseResources();
    transports[0]->releaseResources();
      //    ts.releaseResources();
    sampleRate = 0;
  }

  void paint (Graphics& g) override
  {
    g.setColour (Colours::grey);
    g.fillRect (getLocalBounds().removeFromRight (proportionOfWidth (0.4f)));
  }
  
  void resized() override
  {
    //    titleLabel.centreWithSize(400, 400);
    titleLabel.setBounds(10, 0, 200, 100);
    //    playButton.centreWithSize(80, 50);
    playButton.setBounds(100, 300, 80, 50);
    //    stopButton.centreWithSize(80, 50);
    stopButton.setBounds(100, 360, 80, 50);
    auto rect = getLocalBounds();

    audioSetupComp.setBounds (rect.removeFromLeft (proportionOfWidth (0.6f)));
    rect.reduce (10, 10);

    auto topLine (rect.removeFromTop (20));
    cpuUsageLabel.setBounds (topLine.removeFromLeft (topLine.getWidth() / 2));
    cpuUsageText.setBounds (topLine);
    rect.removeFromTop (20);

    diagnosticsBox.setBounds (rect);
  }
    
  void changeListenerCallback (ChangeBroadcaster* source) override
  {
    if (source == &ts)
      {
	if (ts.isPlaying())
	  changeState (Playing);
	else
	  changeState (Stopped);
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
    //    if (button == &recButton) recButtonClicked();

    // Routing happens here
    for(int i = 0; i<numOfSourceChannels; ++i) {
      for(int j = 0;j<getNumberOfHardwareOutputs(); ++j) {
	if(button == &routeChannel[i][j]) {
	  if(button->getToggleState()) { // Switch on routing for node
	    //	    mapper.setOutputChannelMapping(i, j);
	    logMessage("--------------------------------------");
	    logMessage("Audio channel "+std::to_string(i)+" mapped to output "+std::to_string(j));
	  }
	    //	    logMessage("Yes"+std::to_string(i)+std::to_string(j));
	  else {
	    //	    mapper.setOutputChannelMapping(i, outOfBounds);
	  }
	    //logMessage("No");
	}
    	//	routeChannel[i][j].setBounds(startXPos+(Xindent*i), startYPos+(Yindent*j), 80, 50);
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
	    transports[0]->setPosition (0.0);
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
	    //	    ts.stop();
	    transports[0]->stop();
	    //	    transportSourcei.stop();
	    break;
	  }
      }
  }

  void startSources()
  {
    //    ScopedLock lock(deviceManager.getAudioCallbackLock());
    //    ts.start();

    transports[0]->start();
    //    transportSourcei.start();
  }
  void openButtonClicked()
  {

  }

    
  void playButtonClicked()
  {

    //    AudioFormatReader* reader;
    // AudioFormatReader* readeri;
    //    auto file = File::getSpecialLocation (File::userDocumentsDirectory).getNonexistentChildFile ("AudioRecording", ".wav");
    int numOfFiles = files.size();
    int f = std::rand()%numOfFiles;

    //    createReader(files[0]);
    //    reader = formatManager.createReaderFor(files[0]);
    // readeri = formatManager.createReaderFor(files[0]);

    // if (reader != nullptr)
    //   {
    // 	int fileChannels = reader->numChannels;
    // 	std::cout << fileChannels << std::endl;
    // 	ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource (reader, true);
    // 	ts.setSource(newSource, 0, nullptr, reader->sampleRate);
    // 	readerSource = newSource.release();
    // 	playButton.setEnabled(true);
    //   }

    // Array version
    int index = 0;
    AudioFormatReader* r;
    r = formatManager.createReaderFor(files[index]);
    readers.add(r);
    //    transports.add(new AudioTransportSource());
    if (r != nullptr)
      {
	ScopedPointer<AudioFormatReaderSource> s = new AudioFormatReaderSource(r, true);
	transports[0]->setSource(s, 0, nullptr, r->sampleRate);
	readerSources.add(s.release());
	playButton.setEnabled(true);
      }
    changeState(Starting);
  }

  void createReader(const File &file)
  {
    AudioFormatReader* reader;
    reader = formatManager.createReaderFor(file);
    if(reader != nullptr)
      {
	ScopedPointer<AudioFormatReaderSource> source = new AudioFormatReaderSource(reader, true);
	ts.setSource(source, 0, nullptr, reader->sampleRate);
	rs = source.release();
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
    return activeChannels;
  }
  
  void logMessage (const String& m)
  {
    diagnosticsBox.moveCaretToEnd();
    diagnosticsBox.insertTextAtCaret (m + newLine);
  }
  //==========================================================================
  TextButton openButton;
  TextButton playButton;
  TextButton stopButton;
  Label titleLabel;
  String dataDir;
  double sampleRate;
  int64 nextSampleNum;
  CriticalSection writerLock;
  ScopedPointer<AudioFormatWriter::ThreadedWriter> threadedWriter; 
  AudioFormatWriter::ThreadedWriter* volatile activeWriter;
  AudioFormatManager formatManager;
  ScopedPointer<AudioFormatReaderSource> readerSource;
  ScopedPointer<AudioFormatReaderSource> readerSourcei;
  ScopedPointer<AudioFormatReaderSource> rs;
  AudioTransportSource transportSource;
  AudioTransportSource* transportSourcei;
  AudioTransportSource ts;

  int maxNumberOfFiles = 1;
  OwnedArray<AudioTransportSource> transports;
  OwnedArray<AudioFormatReader> readers;
  OwnedArray<AudioFormatReaderSource> readerSources;
  
  MixerAudioSource mixer;
  //  AudioSource audioSource;
  TransportState state;
  //  juce::ChannelRemappingAudioSource mapper;
  //  TimeSliceThread backgroundThread; // the thread that will write our audio data to disk
  Array<File> files;
  int fileIndex = 0;
  int channelsInFile = 0;
  const static int maxChannels = 64;
  const static int sourceChannels = 2;

  AudioDeviceSelectorComponent audioSetupComp;
  Label cpuUsageLabel;
  Label cpuUsageText;
  Label channelNames[sourceChannels];
  TextEditor diagnosticsBox;
  ToggleButton routeChannel[sourceChannels][maxChannels];

  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

Component* createMainContentComponent()     { return new MainContentComponent(); }

#endif  // MAINCOMPONENT_H_INCLUDED
