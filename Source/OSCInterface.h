/*
  ==============================================================================

  Class handling one audio file and its playback and routing.
  ==============================================================================
*/
#ifndef OSCINTERFACE_H_INCLUDED
#define OSCINTERFACE_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/**
   Class description
*/
class OSCInterface : private OSCReceiver,
		     private OSCReceiver::ListenerWithOSCAddress<OSCReceiver::MessageLoopCallback>
{
  //==============================================================================
  
public:
  OSCInterface();
  ~OSCInterface();

  //==============================================================================

  //  void init(const File &file);

private:
  void oscMessageReceived (const OSCMessage& message) override;
  // void showConnectionErrorMessage (const String& messageText);
  void 

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCInterface)
};

#endif  // OSCINTERFACE_H_INCLUDED
