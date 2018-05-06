/*
  ==============================================================================

  Class handling one audio file and its playback and routing.
  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
/**
   Class description
*/
class OSCInterface : AudioTransportSource
{
public:
  //==============================================================================
  OSCInterface();
  ~OSCInterface();

  //==============================================================================

  void init(const File &file);

private:


  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCInterface)
};
