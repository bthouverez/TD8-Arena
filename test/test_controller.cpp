/******************************************************************************\
* Copyright (C) 2012-2014 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#include <iostream>
#include <cstring>
#include "Leap.h"

using namespace Leap;

class SampleListener2 : public Listener {
  public:
    virtual void onInit(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
    virtual void onDeviceChange(const Controller&);
    virtual void onServiceConnect(const Controller&);
    virtual void onServiceDisconnect(const Controller&);

  private:
};

const std::string fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const std::string boneNames[] = {"Metacarpal", "Proximal", "Middle", "Distal"};
const std::string stateNames[] = {"STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END"};

void SampleListener2::onInit(const Controller& controller) {
  std::cout << "Initialized" << std::endl;
}

void SampleListener2::onConnect(const Controller& controller) {
  std::cout << "Connected" << std::endl;
  controller.enableGesture(Gesture::TYPE_CIRCLE);
  controller.enableGesture(Gesture::TYPE_KEY_TAP);
  controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
  controller.enableGesture(Gesture::TYPE_SWIPE);
}

void SampleListener2::onDisconnect(const Controller& controller) {
  // Note: not dispatched when running in a debugger.
  std::cout << "Disconnected" << std::endl;
}

void SampleListener2::onExit(const Controller& controller) {
  std::cout << "Exited" << std::endl;
}

void SampleListener2::onFrame(const Controller& controller) {
    Bone::Type boneType;
    // Get the most recent frame and report some basic information
    const Frame frame = controller.frame();
    /*
    std::cout << "Frame id: " << frame.id()
    << ", timestamp: " << frame.timestamp()
    << ", hands: " << frame.hands().count()
    << ", extended fingers: " << frame.fingers().extended().count()
    << ", tools: " << frame.tools().count()
    << ", gestures: " << frame.gestures().count() << std::endl;
    */
    HandList hands = frame.hands();

    for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
        // Get the first hand
        const Hand hand = *hl;

        Vector palm_pos = hand.palmPosition();
        const FingerList fingers = hand.fingers();
        // Get fingers

        // Thumb finger
        const Finger thumb_finger = fingers[0];
        boneType = static_cast<Bone::Type>(0);
        Bone thumb_metacarpal_bone = thumb_finger.bone(boneType);
        boneType = static_cast<Bone::Type>(3);
        Bone thumb_distal_bone = thumb_finger.bone(boneType);
        Vector thumb_begin_pos = thumb_metacarpal_bone.prevJoint();
        Vector thumb_end_pos = thumb_distal_bone.nextJoint();
        Vector thumb_vector = (thumb_end_pos - thumb_begin_pos).normalized();

        // Index finger
        const Finger index_finger = fingers[1];
        boneType = static_cast<Bone::Type>(0);
        Bone index_metacarpal_bone = index_finger.bone(boneType);
        boneType = static_cast<Bone::Type>(3);
        Bone index_distal_bone = index_finger.bone(boneType);
        Vector index_begin_pos = index_metacarpal_bone.prevJoint();
        Vector index_end_pos = index_distal_bone.nextJoint();
        Vector index_vector = index_end_pos - index_begin_pos;

        // Ring finger
        const Finger ring_finger = fingers[3];
        boneType = static_cast<Bone::Type>(0);
        Bone ring_metacarpal_bone = ring_finger.bone(boneType);
        boneType = static_cast<Bone::Type>(3);
        Bone ring_distal_bone = ring_finger.bone(boneType);
        Vector ring_begin_pos = ring_metacarpal_bone.prevJoint();
        Vector ring_end_pos = ring_distal_bone.nextJoint();
        Vector ring_vector = ring_end_pos - ring_begin_pos;
        // main_vector is the mean of index vector and ring vector
        Vector main_vector = (ring_vector + index_vector).normalized();

        float cos_thumb = thumb_vector.dot(main_vector);
        cos_thumb = cos_thumb < 0 ? -cos_thumb : cos_thumb;

        Vector transv_begin = ring_begin_pos - index_begin_pos;
        Vector transv_end = ring_end_pos - index_end_pos;
        Vector transversal = (transv_begin + transv_end).normalized();

        // std::cout << "Index" << std::endl;
        // std::cout << "Begin " << index_begin_pos << std::endl;
        // std::cout << "End " << index_end_pos << std::endl;
        // std::cout << "Vec " << index_vector.normalized() << std::endl;
        // std::cout << std::endl;
        // std::cout << "Ring" << std::endl;
        // std::cout << "Begin " << ring_begin_pos << std::endl;
        // std::cout << "End " << ring_end_pos << std::endl;
        // std::cout << "Vec " << ring_vector.normalized()  << std::endl;
        // std::cout << std::endl;
        // std::cout << "Main" << std::endl;
        // std::cout << "Begin " << ring_begin_pos << std::endl;
        // std::cout << "End " << ring_end_pos << std::endl;
        // std::cout << "Vec " << ring_vector.normalized()  << std::endl;
        // std::cout << std::endl;
        // std::cout << std::endl;


        // HAUTEUR
        //std::cout << "Hauteur " << palm_pos.y << std::endl;
        if(cos_thumb < 0.8) {
            std::cout << "SHOOT " << std::endl;
        }
        //main_vector.y = pow(main_vector.y,3);
        //std::cout << "v = "<< main_vector.y << std::endl;
        if(main_vector.y > -0.1 && main_vector.y < 0.1) {
            std::cout << "Stationnaire" << std::endl;

        } else {
            // VITESSE
            if(main_vector.y < -0.1) {
                if(main_vector.y > -0.3) {
                    std::cout << "Accelere doucement" << std::endl;
                } else if(main_vector.y > -0.5) {
                    std::cout << "Accelere moyen" << std::endl;
                } else if(main_vector.y > -0.7) {
                    std::cout << "Accelere fort" << std::endl;
                } else  {
                    std::cout << "ENVOIE LA GOMME MARCEL" << std::endl;
                }
            } else if(main_vector.y > 0.1) {
                if(main_vector.y < 0.3) {
                    std::cout << "Decelere doucement" << std::endl;
                } else if(main_vector.y < 0.5) {
                    std::cout << "Decelere moyen" << std::endl;
                } else if(main_vector.y < 0.7) {
                    std::cout << "Decelere fort" << std::endl;
                } else  {
                    std::cout << "ARRETE TOIIII ZEUBII" << std::endl;
                }
            } else {
                std::cout << "Autre..." << std::endl;
            }
        }
        // DIRECTION
        //std::cout << "d = "<< transversal.y << std::endl;
        if(transversal.y > -0.1 && transversal.y < 0.1) {
            std::cout << "Tout droit" << std::endl;

        } else {
            // VITESSE
            if(transversal.y < -0.1) {
                if(transversal.y > -0.3) {
                    std::cout << "Tourne droite leger" << std::endl;
                } else if(transversal.y > -0.5) {
                    std::cout << "Tourne droite moyen" << std::endl;
                } else if(transversal.y > -0.7) {
                    std::cout << "Tourne droite fort" << std::endl;
                } else  {
                    std::cout << "A BABORD TOUTE" << std::endl;
                }
            } else if(transversal.y > 0.1) {
                if(transversal.y < 0.3) {
                    std::cout << "Tourne gauche leger" << std::endl;
                } else if(transversal.y < 0.5) {
                    std::cout << "Tourne gauche moyen" << std::endl;
                } else if(transversal.y < 0.7) {
                    std::cout << "Tourne gauche fort" << std::endl;
                } else  {
                    std::cout << "A TRIBORD TOUTE" << std::endl;
                }
            } else {
                std::cout << "Autre..." << std::endl;
            }
        }

        std::cout << std::endl;
    }
}

void SampleListener2::onFocusGained(const Controller& controller) {
  std::cout << "Focus Gained" << std::endl;
}

void SampleListener2::onFocusLost(const Controller& controller) {
  std::cout << "Focus Lost" << std::endl;
}

void SampleListener2::onDeviceChange(const Controller& controller) {
  std::cout << "Device Changed" << std::endl;
  const DeviceList devices = controller.devices();

  for (int i = 0; i < devices.count(); ++i) {
    std::cout << "id: " << devices[i].toString() << std::endl;
    std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
  }
}

void SampleListener2::onServiceConnect(const Controller& controller) {
  std::cout << "Service Connected" << std::endl;
}

void SampleListener2::onServiceDisconnect(const Controller& controller) {
  std::cout << "Service Disconnected" << std::endl;
}

int main(int argc, char** argv) {
  // Create a sample listener and controller
  SampleListener2 listener;
  Controller controller;

  // Have the sample listener receive events from the controller
  controller.addListener(listener);

  if (argc > 1 && strcmp(argv[1], "--bg") == 0)
    controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);

  // Keep this process running until Enter is pressed
  std::cout << "Press Enter to quit..." << std::endl;
  std::cin.get();

  // Remove the sample listener when done
  controller.removeListener(listener);

  return 0;
}
