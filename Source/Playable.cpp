/*
  ==============================================================================
  A class that describes an audio file and its output connections

  Copyright (c) 2018 Henrik Frisk
  ------------------------------------------------------------------------------


  ==============================================================================
*/

#include "Playable.h"

//==============================================================================
Playable::Playable() : mapper(&transportSource, true)
{
  formatManager.registerBasicFormats();
}

Playable::Playable(const File &file) : mapper(&transportSource, true)
{
  formatManager.registerBasicFormats();
  init(file);
}

Playable::~Playable() {}

/** 
 *  Initialize the object.
 */
void Playable::init(const File &file)
{
  reader = formatManager.createReaderFor(file);
  std::cout << file.getFileName() << std::endl;
  if(reader != nullptr)
    {
      std::cout << "Now here" << std::endl;
      channels = reader->numChannels;
      ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource (reader, true);
      transportSource.setSource(newSource, 0, nullptr, reader->sampleRate);
      readerSource = newSource.release();
    }
  //  AudioTransportSource::prepareToPlay()
}

