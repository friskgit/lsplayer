/*
  ==============================================================================
  A class that handles OSC communication

  Copyright (c) 2018 Henrik Frisk
  ------------------------------------------------------------------------------


  ==============================================================================
*/

#include "OSCInterface.h"

//==============================================================================
OSCInterface::OSCInterface()
{
  // specify here on which UDP port number to receive incoming OSC messages
  if (! connect (9001))                   // [3]
    //    showConnectionErrorMessage ("Error: could not connect to UDP port 9001.");
    std::cout << "Error: could not connect to UDP port 9001." << std::endl;
  // tell the component to listen for OSC messages matching this address:
  addListener (this, "/juce"); // [4]
}

OSCInterface::~OSCInterface() {}

void OSCInterface::oscMessageReceived(const OSCMessage& message)
{
  if (message.size() == 1 && message[0].isFloat32())
    std::cout << std::to_string(message[0].getFloat32()) << std::endl;
    //    rotaryKnob.setValue (jlimit (0.0f, 10.0f, message[0].getFloat32()));
}

// void showConnectionErrorMessage (const String& messageText)
// {
//   // AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
//   // 				    "Connection error",
//   // 				    messageText,
//   // 				    "OK");
// }
