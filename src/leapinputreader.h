#ifndef LEAP_INPUTREADER_H
#define LEAP_INPUTREADER_H

#include <iostream>
#include <cstring>

#include <Leap.h>
#include <LeapMath.h>

#include "vec.h"
#include "color.h"

const std::string fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const std::string boneNames[] = {"Metacarpal", "Proximal", "Middle", "Distal"};
const std::string stateNames[] = {"STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END"};

class SampleListener : public Leap::Listener {
public:
    virtual void onInit(const Leap::Controller&);
    virtual void onConnect(const Leap::Controller&);
    virtual void onDisconnect(const Leap::Controller&);
    virtual void onExit(const Leap::Controller&);
    virtual void onFrame(const Leap::Controller&);
    virtual void onFocusGained(const Leap::Controller&);
    virtual void onFocusLost(const Leap::Controller&);
    virtual void onDeviceChange(const Leap::Controller&);
    virtual void onServiceConnect(const Leap::Controller&);
    virtual void onServiceDisconnect(const Leap::Controller&);
};

class LeapInputReader
{
public:

    LeapInputReader();
	~LeapInputReader();

    void UpdateHandLocations();
    
    Leap::Vector GetLeftPalmLocation();
    Leap::Vector GetLeftFingerLocation();
    Leap::Vector GetRightPalmLocation();
    Leap::Vector GetRightFingerLocation();
    bool IsValidInputLastFrame();
    /*
     Offset to account for position of Leap mount on Oculus headset
     NOTE: for simplicity this is still in Leap coordinates!  This is intended to first correct the Leap coordinates for the fact that the mount positions the Leap a few inches in front of the eyes.  
     */
    Leap::Vector LeapMountOffset;

    /*
     Offset to account for the fact the Leap is head mounted so the proper location to draw the hand to look natural will require some trial and error.
     */
    Leap::Vector LeapHandOffset;

    bool LeapDrawSimpleHands;
    
protected:

    SampleListener listener;
    Leap::Controller controller;

    bool ValidInputLastFrame;
    Leap::Vector LeftPalmLocation;
    Leap::Vector LeftFingerLocation;
    Leap::Vector RightPalmLocation;
    Leap::Vector RightFingerLocation;
};

#endif