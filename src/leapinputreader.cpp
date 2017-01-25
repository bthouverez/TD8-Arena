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

}