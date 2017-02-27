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


/** TODO
   - gestion hauteur : directe ou par accélération ? directe pour l'instant
**/
class LeapInputReader
{
public:

    LeapInputReader();
	~LeapInputReader();

    void Update();
    
    inline float GetHeight()    { return m_height; }
    inline float GetSpeed()     { return m_speed; }
    inline float GetDirection() { return m_direction; }
    inline bool  GetShoot()     { return m_shoot; }
    inline bool  GetHand()      { return m_hand; }

protected:

    SampleListener listener;
    Leap::Controller controller;

    float m_height = 0.f;
    float m_speed = 0.f;
    float m_direction = 0.f;
    bool  m_shoot = false;
    bool  m_hand = false; // présence de la main au dessus du leap ou non
};

#endif