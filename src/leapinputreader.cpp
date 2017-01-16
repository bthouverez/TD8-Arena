#include "leapinputreader.h"

LeapInputReader::LeapInputReader()
{
    controller.addListener(listener);
    LeapDrawSimpleHands = true;
    LeapMountOffset = Leap::Vector(150.f, 0.f, -20.f);
    LeapHandOffset = Leap::Vector(10.0, 0.0, 45.0); // note: x=forward, y=right, z=up
    ValidInputLastFrame = false;
}

LeapInputReader::~LeapInputReader()
{
    controller.removeListener(listener);
}

bool LeapInputReader::IsValidInputLastFrame() {
    return ValidInputLastFrame;
}

Leap::Vector LeapInputReader::GetLeftPalmLocation() {
    return LeftPalmLocation;
}

Leap::Vector LeapInputReader::GetLeftFingerLocation() {
    return LeftFingerLocation;
}

Leap::Vector LeapInputReader::GetRightPalmLocation() {
    return RightPalmLocation;
}

Leap::Vector LeapInputReader::GetRightFingerLocation() {
    return RightFingerLocation;
}

void LeapInputReader::UpdateHandLocations()
{
    // First just get hand and finger positions and draw the hands
    Leap::Frame Frame = controller.frame();
    Leap::HandList Hands = Frame.hands();
    Leap::PointableList Pointables = Frame.pointables();
    Leap::GestureList gestures = Frame.gestures();
    Color handColor = Blue();
    Color fingertipColor = handColor;
    
    ValidInputLastFrame = false;
    for (Leap::HandList::const_iterator HandsIter = Hands.begin(); HandsIter != Hands.end(); HandsIter++) {
        ValidInputLastFrame = true; // for now valid if hands detected.  in future, will check if movement is "natural"
        Leap::Hand Hand = (Leap::Hand)(*HandsIter);
        Leap::Vector palmPosition = Hand.palmPosition();

        if (Hand.isLeft()) {
            LeftPalmLocation = palmPosition;
        }
        else {
            RightPalmLocation = palmPosition;
        }

        if (LeapDrawSimpleHands) {
            //DrawDebugSphere(World, palmPosition, 1.0, 12, handColor);
        }

        Leap::FingerList Fingers = Hand.fingers();
        for (Leap::FingerList::const_iterator FingersIter = Fingers.begin(); FingersIter != Fingers.end(); FingersIter++) {
            Leap::Finger finger = (Leap::Finger)(*FingersIter);
            Leap::Vector tipPosition = finger.tipPosition();
            if (finger.type() == Leap::Finger::TYPE_MIDDLE) { // only middle finger used for leap input
                fingertipColor = Red();
               if (Hand.isLeft()) {
                    LeftFingerLocation = tipPosition;
                }
                else {
                    RightFingerLocation = tipPosition;
                }
            }
            else {
                fingertipColor = handColor;
            }

            if (LeapDrawSimpleHands) {
                //DrawDebugSphere(World, tipPosition, 0.5, 12, fingertipColor);
                //DrawDebugLine(World, palmPosition, tipPosition, handColor);
            }
        }
    }
}

void SampleListener::onInit(const Leap::Controller& controller) {
    std::cout << "Initialized" << std::endl;
}

void SampleListener::onConnect(const Leap::Controller& controller) {
    std::cout << "Connected" << std::endl;
    controller.enableGesture(Leap::Gesture::TYPE_CIRCLE);
    controller.enableGesture(Leap::Gesture::TYPE_KEY_TAP);
    controller.enableGesture(Leap::Gesture::TYPE_SCREEN_TAP);
    controller.enableGesture(Leap::Gesture::TYPE_SWIPE);
}

void SampleListener::onDisconnect(const Leap::Controller& controller) {
    // Note: not dispatched when running in a debugger.
    std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Leap::Controller& controller) {
    std::cout << "Exited" << std::endl;
}

void SampleListener::onFocusGained(const Leap::Controller& controller) {
    std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Leap::Controller& controller) {
    std::cout << "Focus Lost" << std::endl;
}

void SampleListener::onDeviceChange(const Leap::Controller& controller) {
    std::cout << "Device Changed" << std::endl;
    const Leap::DeviceList devices = controller.devices();

    for (int i = 0; i < devices.count(); ++i) {
        std::cout << "id: " << devices[i].toString() << std::endl;
        std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
    }
}

void SampleListener::onServiceConnect(const Leap::Controller& controller) {
    std::cout << "Service Connected" << std::endl;
}

void SampleListener::onServiceDisconnect(const Leap::Controller& controller) {
    std::cout << "Service Disconnected" << std::endl;
}

void SampleListener::onFrame(const Leap::Controller& controller) {
    // Get the most recent frame and report some basic information
    /*const Leap::Frame frame = controller.frame();
    std::cout << "Frame id: " << frame.id()
            << ", timestamp: " << frame.timestamp()
            << ", hands: " << frame.hands().count()
            << ", extended fingers: " << frame.fingers().extended().count()
            << ", tools: " << frame.tools().count()
            << ", gestures: " << frame.gestures().count() << std::endl;

    Leap::HandList hands = frame.hands();
    for (Leap::HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
    // Get the first hand
    const Leap::Hand hand = *hl;
    std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
    std::cout << std::string(2, ' ') << handType << ", id: " << hand.id()
              << ", palm position: " << hand.palmPosition() << std::endl;
    // Get the hand's normal vector and direction
    const Leap::Vector normal = hand.palmNormal();
    const Leap::Vector direction = hand.direction();

    // Calculate the hand's pitch, roll, and yaw angles
    std::cout << std::string(2, ' ') <<  "pitch: " << direction.pitch() * Leap::RAD_TO_DEG << " degrees, "
              << "roll: " << normal.roll() * Leap::RAD_TO_DEG << " degrees, "
              << "yaw: " << direction.yaw() * Leap::RAD_TO_DEG << " degrees" << std::endl;

    // Get the Arm bone
    Leap::Arm arm = hand.arm();
    std::cout << std::string(2, ' ') <<  "Arm direction: " << arm.direction()
              << " wrist position: " << arm.wristPosition()
              << " elbow position: " << arm.elbowPosition() << std::endl;

    // Get fingers
    const Leap::FingerList fingers = hand.fingers();
    for (Leap::FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
      const Leap::Finger finger = *fl;
      std::cout << std::string(4, ' ') <<  fingerNames[finger.type()]
                << " finger, id: " << finger.id()
                << ", length: " << finger.length()
                << "mm, width: " << finger.width() << std::endl;

      // Get finger bones
      for (int b = 0; b < 4; ++b) {
        Leap::Bone::Type boneType = static_cast<Leap::Bone::Type>(b);
        Leap::Bone bone = finger.bone(boneType);
        std::cout << std::string(6, ' ') <<  boneNames[boneType]
                  << " bone, start: " << bone.prevJoint()
                  << ", end: " << bone.nextJoint()
                  << ", direction: " << bone.direction() << std::endl;
      }
    }
  }

  // Get tools
  const Leap::ToolList tools = frame.tools();
  for (Leap::ToolList::const_iterator tl = tools.begin(); tl != tools.end(); ++tl) {
    const Leap::Tool tool = *tl;
    std::cout << std::string(2, ' ') <<  "Tool, id: " << tool.id()
              << ", position: " << tool.tipPosition()
              << ", direction: " << tool.direction() << std::endl;
  }

  // Get gestures
  const Leap::GestureList gestures = frame.gestures();
  for (int g = 0; g < gestures.count(); ++g) {
    Leap::Gesture gesture = gestures[g];

    switch (gesture.type()) {
      case Leap::Gesture::TYPE_CIRCLE:
      {
        Leap::CircleGesture circle = gesture;
        std::string clockwiseness;

        if (circle.pointable().direction().angleTo(circle.normal()) <= Leap::PI/2) {
          clockwiseness = "clockwise";
        } else {
          clockwiseness = "counterclockwise";
        }

        // Calculate angle swept since last frame
        float sweptAngle = 0;
        if (circle.state() != Leap::Gesture::STATE_START) {
          Leap::CircleGesture previousUpdate = Leap::CircleGesture(controller.frame(1).gesture(circle.id()));
          sweptAngle = (circle.progress() - previousUpdate.progress()) * 2 * Leap::PI;
        }
        std::cout << std::string(2, ' ')
                  << "Circle id: " << gesture.id()
                  << ", state: " << stateNames[gesture.state()]
                  << ", progress: " << circle.progress()
                  << ", radius: " << circle.radius()
                  << ", angle " << sweptAngle * Leap::RAD_TO_DEG
                  <<  ", " << clockwiseness << std::endl;
        break;
      }
      case Leap::Gesture::TYPE_SWIPE:
      {
        Leap::SwipeGesture swipe = gesture;
        std::cout << std::string(2, ' ')
          << "Swipe id: " << gesture.id()
          << ", state: " << stateNames[gesture.state()]
          << ", direction: " << swipe.direction()
          << ", speed: " << swipe.speed() << std::endl;
        break;
      }
      case Leap::Gesture::TYPE_KEY_TAP:
      {
        Leap::KeyTapGesture tap = gesture;
        std::cout << std::string(2, ' ')
          << "Key Tap id: " << gesture.id()
          << ", state: " << stateNames[gesture.state()]
          << ", position: " << tap.position()
          << ", direction: " << tap.direction()<< std::endl;
        break;
      }
      case Leap::Gesture::TYPE_SCREEN_TAP:
      {
        Leap::ScreenTapGesture screentap = gesture;
        std::cout << std::string(2, ' ')
          << "Screen Tap id: " << gesture.id()
          << ", state: " << stateNames[gesture.state()]
          << ", position: " << screentap.position()
          << ", direction: " << screentap.direction()<< std::endl;
        break;
      }
      default:
        std::cout << std::string(2, ' ')  << "Unknown gesture type." << std::endl;
        break;
    }
  }

  if (!frame.hands().isEmpty() || !gestures.isEmpty()) {
    std::cout << std::endl;
  }*/

}