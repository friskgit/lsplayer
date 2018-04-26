#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"



class MainContentComponent   : public AudioAppComponent,
			       public ChangeListener,
			       public Button::Listener,
			       private Label::Listener
{
public:
  MainContentComponent()
    :   state (Stopped),
	backgroundThread ("Audio Recorder Thread"), sampleRate (44100), nextSampleNum (0), activeWriter (nullptr)

  {
    backgroundThread.startThread();

    dataDir = "~/rosenberg";
    addAndMakeVisible (titleLabel);
    titleLabel.setFont (Font ("Geneva", 36.0f, Font::bold));
    titleLabel.setText("Folk Song Lab", dontSendNotification);
    titleLabel.setColour (Label::textColourId, Colours::white);
    titleLabel.setJustificationType (Justification::centred);

    addAndMakeVisible(tryLabel);
    tryLabel.setFont(Font ("Geneva", 14.0f, Font::plain));
    tryLabel.setText("Prova igen!", dontSendNotification);
    tryLabel.setColour(Label::textColourId, Colours::white);
    tryLabel.setJustificationType(Justification::centred);
        
    addAndMakeVisible (&playButton);
    playButton.setButtonText ("Starta");
    playButton.addListener (this);
    playButton.setColour (TextButton::buttonColourId, Colours::green);
    playButton.setSize(200, 200);
    playButton.setEnabled (true);

    //   addAndMakeVisible (&stopButton);
   stopButton.setButtonText ("Stop");
   stopButton.addListener (this);
   stopButton.setColour (TextButton::buttonColourId, Colours::red);
   stopButton.setEnabled (false);

   //    addAndMakeVisible (&recButton);
    recButton.setButtonText ("Rec");
    recButton.addListener (this);
    recButton.setColour (TextButton::buttonColourId, Colours::red);
    recButton.setEnabled (false);

    addAndMakeVisible (infoLabel1);
    addAndMakeVisible (infoLabel2);
    addAndMakeVisible (infoLabel3);
    addAndMakeVisible (infoLabel4);
    addAndMakeVisible (infoLabeli);
    String infoText1;
    String infoText2;
    String infoText3;
    String infoText4;
    String infoTexti;
    
    infoText1 << "Tryck på starta för att \n";
    infoText1 << "börja experimentet.";
    infoText2 << "Sjung med i melodin så \n";
    infoText2 << "samtidigt som du kan.";
    infoText3 << "Din röst kommer att spelas in. Gör gärna flera försök ";
    infoText3 << "genom att trycka på starta igen.";
    infoText4 << "Tack för din hjälp!";

    infoTexti << "Genom att du trycker på starta så godkänner du att din/dina";
    infoTexti << "inspelningar kan användas i forskningssyfte, men du är ";
    infoTexti << "försäkrad om att deltagandet är helt anonymt.";
	
    infoLabel1.setText (infoText1, dontSendNotification);
    infoLabel1.setColour (Label::backgroundColourId, Colours::darkgrey);
    infoLabel1.setFont(Font ("Geneva", 24.0f, Font::plain));

    infoLabel2.setText (infoText2, dontSendNotification);
    infoLabel2.setColour (Label::backgroundColourId, Colours::darkgrey);
    infoLabel2.setFont(Font ("Geneva", 24.0f, Font::plain));

    infoLabel3.setText (infoText3, dontSendNotification);
    infoLabel3.setColour (Label::backgroundColourId, Colours::darkgrey);
    infoLabel3.setFont(Font ("Geneva", 14.0f, Font::plain));

    infoLabel4.setText (infoText4, dontSendNotification);
    infoLabel4.setColour (Label::backgroundColourId, Colours::darkgrey);
    infoLabel4.setFont(Font ("Geneva", 14.0f, Font::plain));
    
    infoLabeli.setText (infoTexti, dontSendNotification);
    infoLabeli.setColour (Label::backgroundColourId, Colours::darkgrey);
    infoLabeli.setFont(Font ("Geneva", 14.0f, Font::plain));   
 
    setSize (800, 600);
        
    formatManager.registerBasicFormats();       // [1]
    transportSource.addChangeListener (this);   // [2]

    setAudioChannels (2, 2);
    //    deviceManager.addAudioCallback (&recorder);

    //recorder.startRecording (file);
    File dir("~/rosenberg/audio");
    files = dir.findChildFiles(2, false, "*.wav");
    for(int i = 0; i < files.size(); i++)
      std::cout << files[i].getFileName() << std::endl;
  }
    
  ~MainContentComponent()
  {
    shutdownAudio();
  }

      void paint (Graphics& g) override
    {
         g.fillAll (Colours::darkgrey);
    }

    void labelTextChanged (Label* label) override
    {
 //       if (label == &inputText)
  //          uppercaseText.setText (inputText.getText().toUpperCase(), dontSendNotification);
    }
  void prepareToPlay (int samplesPerBlockExpected, double sR) override
  {
    transportSource.prepareToPlay (samplesPerBlockExpected, sR);
    sampleRate = sR;
  }

  void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
  {
    AudioIODevice* device = deviceManager.getCurrentAudioDevice();
    const BigInteger activeInputChannels = device->getActiveInputChannels();
    const BigInteger activeOutputChannels = device->getActiveOutputChannels();
    const int maxInputChannels = activeInputChannels.getHighestBit() + 1;
    const int maxOutputChannels = 1; //activeOutputChannels.getHighestBit() + 1;

    //    const float** inputChannelData
    const ScopedLock sl(writerLock);
    //    std::cout << "output channels: " << maxOutputChannels << std::endl;

    // Clear buffers
    if (readerSource == nullptr)
      {
    	bufferToFill.clearActiveBufferRegion();
    	return;
      }
    
    for (int channel = 0; channel < maxOutputChannels; ++channel)
      {
	// Clear buffers
        if ((! activeOutputChannels[channel]) || maxInputChannels == 0)
	  {
            bufferToFill.buffer->clear (channel, bufferToFill.startSample, bufferToFill.numSamples);
	  }
        else
	  {
            const int actualInputChannel = channel % maxInputChannels; // [1]
            
            if (! activeInputChannels[channel]) // [2]
	      {
                bufferToFill.buffer->clear (channel, bufferToFill.startSample, bufferToFill.numSamples);
	      }
            else 
	      {
		if(activeWriter != nullptr)
		  {
		    
		    // const float* inBuffer = bufferToFill.buffer->getReadPointer (actualInputChannel,
		    // 								 bufferToFill.startSample);
		    const float** inBuffer = bufferToFill.buffer->getArrayOfReadPointers();
		    activeWriter->write(inBuffer, bufferToFill.numSamples);
		    float* outBuffer = bufferToFill.buffer->getWritePointer (channel,
									     bufferToFill.startSample);

		    nextSampleNum += bufferToFill.numSamples;

		  }
	      }
	  }
      }
    // The play routine
    // Get samples from the file to play back.
    transportSource.getNextAudioBlock (bufferToFill);
    				    // const float** both;
		    // both[0] = inBuffer;
		    // both[1] = inBuffer;

		    //		    activeWriter->writeFromAudioSampleBuffer(rec, bufferToFill.numSamples, 2048);
		    //		    AudioBuffer<float*> myBuffer(inBuffer, 1, bufferToFill.numSamples);
		    
		    //		    
		    //		    AudioBuffer<float> buffer (const_cast<float*> (inBuffer), 1, bufferToFill.numSamples);
				//for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
		// outBuffer[sample] = inBuffer[sample] * random.nextFloat();
		//outBuffer[sample] =
		// We need to clear the output buffers, in case they're full of junk..
		//		  if (outBuffer != nullptr)
		    //		    FloatVectorOperations::clear(outBuffer, bufferToFill.numSamples);
  }

  void releaseResources() override
  {
    transportSource.releaseResources();
    sampleRate = 0;
  }

  void resized() override
  {
    //    titleLabel.centreWithSize(400, 400);
    titleLabel.setBounds(10, 0, 200, 100);
    playButton.centreWithSize(80, 50);
    tryLabel.setSize(getWidth()-100, 60);
    tryLabel.setCentreRelative(0.5f, 0.59f);
    infoLabel1.setBounds (600, 0, 400, 100);
    infoLabel2.setBounds (600, 80, 400, 60);
    infoLabel3.setBounds (600, 170, 300, 30);
    infoLabel4.setBounds (600, 200, 300, 30);
    infoLabeli.setBounds (10, getHeight()-100, 300, 100);
    //    infoLabel.setTopRightPosition(getWidth(), -140);
  }
    
  void changeListenerCallback (ChangeBroadcaster* source) override
  {
    if (source == &transportSource)
      {
	if (transportSource.isPlaying())
	  changeState (Playing);
	else
	  changeState (Stopped);
      }
  }

  void buttonClicked (Button* button) override
  {
    if (button == &openButton)  openButtonClicked();
    if (button == &playButton)  playButtonClicked();
    if (button == &stopButton)  stopButtonClicked();
    if (button == &recButton) recButtonClicked();
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
	    transportSource.setPosition (0.0);
	    stopRecording();
	    tryLabel.setVisible(true);
	    break;
                    
	  case Starting:                        
	    playButton.setEnabled (true);
	    transportSource.start();
	    break;
                    
	  case Playing:                         
	    stopButton.setEnabled (true);
	    playButton.setEnabled (false);
	    tryLabel.setVisible(false);
	    break;
                    
	  case Stopping:                        
	    transportSource.stop();
	    break;
	  }
      }
  }
    
  void openButtonClicked()
  {
    if(transportSource.isPlaying()) {
      changeState(Stopped);
    }

    AudioFormatReader* reader;

    if (reader != nullptr)
      {
	ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource (reader, true); 
	transportSource.setSource (newSource, 0, nullptr, reader->sampleRate);
	playButton.setEnabled (true);
	readerSource = newSource.release();
      }
  }

    
  void playButtonClicked()
  {

    AudioFormatReader* reader;

    //    auto file = File::getSpecialLocation (File::userDocumentsDirectory).getNonexistentChildFile ("AudioRecording", ".wav");
    int numOfFiles = files.size();
    int f = std::rand()%numOfFiles;
    reader = formatManager.createReaderFor(files[f]);

    startRecording(f);      
    if (reader != nullptr)
      {
	ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource (reader, true); 
	transportSource.setSource (newSource, 0, nullptr, reader->sampleRate);
	playButton.setEnabled (true);
	readerSource = newSource.release();
      }
    changeState (Starting);
  }
    
  void stopButtonClicked()
  {
    changeState (Stopping);
  }

  void recButtonClicked()
  {
    //    std::cout << "Record" << std::endl;
  }

  void startRecording(int f)
  {
    String fileName = files[f].getFileNameWithoutExtension();
    String index = String(fileIndex) += String("-");
    String recName = index += fileName;
    //    auto file = File::getSpecialLocation(File::userDocumentsDirectory).getNonexistentChildFile (recName, ".wav");
    auto file = File(dataDir).getNonexistentChildFile (recName, ".wav");
    //    recorder.startRecording(file);
    stopRecording();
    if (sampleRate > 0)
      {
	// Create output stream
	file.deleteFile();
	ScopedPointer<FileOutputStream> fileStream(file.createOutputStream());
	if(fileStream != nullptr) {
	  WavAudioFormat wavFormat;
	  AudioFormatWriter* writer = wavFormat.createWriterFor(fileStream, sampleRate, 1, 16, StringPairArray(), 0);
	  if(writer != nullptr)
	    {
	      // Responsibility for deleting is left to the writer.
	      fileStream.release();
	      threadedWriter = new AudioFormatWriter::ThreadedWriter(writer, backgroundThread, 32768);
	      const ScopedLock sl(writerLock);
	      activeWriter = threadedWriter;
	    }
	}
      }
    fileIndex++;
  }

  void stopRecording()
  {
    // First, clear this pointer to stop the audio callback from using our writer object..
    {
      const ScopedLock sl (writerLock);
      activeWriter = nullptr;
    }
    std::cout << "Stopped recording..." << std::endl;
    // Now we can delete the writer object. It's done in this order because the deletion could
    // take a little time while remaining data gets flushed to disk, so it's best to avoid blocking
    // the audio callback while this happens.
    threadedWriter.reset();
    //    recorder.stop();
  }

  //==========================================================================
  TextButton openButton;
  TextButton playButton;
  TextButton stopButton;
  TextButton recButton;
  Label titleLabel;
  Label tryLabel;
  Label infoLabel1;
  Label infoLabel2;
  Label infoLabel3;
  Label infoLabel4;
  Label infoLabeli;
  String dataDir;
  double sampleRate;
  int64 nextSampleNum;
  CriticalSection writerLock;
  ScopedPointer<AudioFormatWriter::ThreadedWriter> threadedWriter; 
  AudioFormatWriter::ThreadedWriter* volatile activeWriter;
  AudioFormatManager formatManager;
  ScopedPointer<AudioFormatReaderSource> readerSource;
  AudioTransportSource transportSource;
  TransportState state;
  TimeSliceThread backgroundThread; // the thread that will write our audio data to disk
  Array<File> files;
  int fileIndex = 0;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

Component* createMainContentComponent()     { return new MainContentComponent(); }

#endif  // MAINCOMPONENT_H_INCLUDED
