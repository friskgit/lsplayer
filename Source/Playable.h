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
class Playable : AudioTransportSource
{
public:
  //==============================================================================
  Playable();
  Playable(const File &file);
  ~Playable();

  //==============================================================================

  AudioFormatReader* getReader() {return reader;}
  AudioTransportSource* getTransportSource() {return &transportSource;}
  //  ChannelRemappingAudioSource& getMapper() {return &mapper;}
  int getNumberOfChannels() {return channels;}
  void setSourceFile(const File &file) {audioFile = &file;}
  void init(const File &file);
  String getName() {return audioFile->getFileName();}
  //  void prepareToPlay();
  
private:
  String fileName;
  int channels;
  AudioFormatManager formatManager;
  AudioFormatReader* reader;
  ScopedPointer<AudioFormatReaderSource> newSource;
  ScopedPointer<AudioFormatReaderSource> readerSource;
  AudioTransportSource transportSource;
  ChannelRemappingAudioSource mapper;
  const File *audioFile;

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Playable)
};
