#include "leapinputreader.h"

LeapInputReader::LeapInputReader(Leap::Controller* Controller)
{
    this->Controller = Controller;
    LeapDrawSimpleHands = true;
    LeapToUnrealScalingFactor = 0.1;  // Leap Unit is a millimiter and Unreal units are centimeters
    // NOTE: Mount offset is still in Leap coordinates, not Unreal units!!
    LeapMountOffset = FVector(150.f, 0.f, -20.f);
    LeapHandOffset = FVector(10.0, 0.0, 45.0); // note: x=forward, y=right, z=up
    ValidInputLastFrame = false;
}

LeapInputReader::~LeapInputReader()
{
}

bool LeapInputReader::IsValidInputLastFrame() {
    return ValidInputLastFrame;
}


FVector LeapInputReader::GetLeftPalmLocation() {
    return LeftPalmLocation;
}

FVector LeapInputReader::GetLeftFingerLocation() {
    return LeftFingerLocation;
}

FVector LeapInputReader::GetRightPalmLocation() {
    return RightPalmLocation;
}

FVector LeapInputReader::GetRightFingerLocation() {
    return RightFingerLocation;
}

void LeapInputReader::UpdateHandLocations()
{
    // First just get hand and finger positions and draw the hands
    Leap::Frame Frame = Controller->frame();
    Leap::HandList Hands = Frame.hands();
    Leap::PointableList Pointables = Frame.pointables();
    Leap::GestureList gestures = Frame.gestures();
    FColor handColor = FColor::Magenta;
    FColor fingertipColor = handColor;
    
    ValidInputLastFrame = false;
    for (Leap::HandList::const_iterator HandsIter = Hands.begin(); HandsIter != Hands.end(); HandsIter++) {
        ValidInputLastFrame = true; // for now valid if hands detected.  in future, will check if movement is "natural"
        Leap::Hand Hand = (Leap::Hand)(*HandsIter);
        Leap::Vector palmPosition = Hand.palmPosition();
        FVector palmLocation = LeapPositionToUnrealLocation(palmPosition, LeapHandOffset);
        if (Hand.isLeft()) {
            LeftPalmLocation = palmLocation;
        }
        else {
            RightPalmLocation = palmLocation;
        }
        if (LeapDrawSimpleHands) {
            DrawDebugSphere(World, palmLocation, 1.0, 12, handColor);
        }
        Leap::FingerList Fingers = Hand.fingers();
        for (Leap::FingerList::const_iterator FingersIter = Fingers.begin(); FingersIter != Fingers.end(); FingersIter++) {
            Leap::Finger finger = (Leap::Finger)(*FingersIter);
            Leap::Vector tipPosition = finger.tipPosition();
            FString fingerPosition(tipPosition.toString().c_str());
            FVector fingerLocation = LeapPositionToUnrealLocation(tipPosition, LeapHandOffset);
            if (finger.type() == Leap::Finger::TYPE_MIDDLE) { // only middle finger used for leap input
                fingertipColor = FColor::Red;
               if (Hand.isLeft()) {
                    LeftFingerLocation = fingerLocation;
                }
                else {
                    RightFingerLocation = fingerLocation;
                }
            }
            else {
                fingertipColor = handColor;
            }
            if (LeapDrawSimpleHands) {
                DrawDebugSphere(World, fingerLocation, 0.5, 12, fingertipColor);
                DrawDebugLine(World, palmLocation, fingerLocation, handColor);
            }
        }
    }
}