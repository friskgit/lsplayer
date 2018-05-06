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
  MainContentComponent();
  ~MainContentComponent();

  void prepareToPlay (int samplesPerBlockExpected, double sR) override;
  
  void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;

  void releaseResources() override;

  void paint (Graphics& g) override;
  
  void resized() override;

  void changeListenerCallback (ChangeBroadcaster* source) override;

  void buttonClicked (Button* button) override;

private:
  enum TransportState
    {
     Stopped,
     Starting,
     Playing,
     Stopping
    };

  void changeState (TransportState newState);

  void startSources();
  
  void setPosition(float pos);

  void openButtonClicked();
    
  void playButtonClicked();

  void createReader(const File &file, int i);
  
  void stopButtonClicked();

  static String getListOfActiveBits (const BigInteger& b);

  void dumpDeviceInfo();

  int getNumberOfHardwareOutputs();
  
  void logMessage (const String& m);

  void sliderValueChanged(Slider *slider) override;
  
  void 	sliderDragStarted (Slider *slider) override;

  void sliderDragEnded (Slider *slider) override;

  void sliderSetRange(double min, double max);
  
  void sliderEnabled(bool value);

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

//Component* createMainContentComponent()     { return ; }

#endif  // MAINCOMPONENT_H_INCLUDED
