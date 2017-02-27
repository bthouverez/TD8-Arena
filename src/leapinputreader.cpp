#include "leapinputreader.h"

LeapInputReader::LeapInputReader()
{
    controller.addListener(listener);
}

LeapInputReader::~LeapInputReader()
{
    controller.removeListener(listener);
}



void LeapInputReader::Update()
{
    m_shoot = false;
    Leap::Bone::Type boneType;
    // Get the most recent frame and report some basic information
    const Leap::Frame frame = controller.frame();

    if(frame.hands().count() != 1) {
        m_hand = false;
    } else {        
        m_hand = true;
        if(!frame.isValid()) return; // leave if invalid frame

        Leap::HandList hands = frame.hands();

        for (Leap::HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
            /** Get data from Leap Motion **/

            // Get the first hand
            const Leap::Hand hand = *hl;

            Leap::Vector palm_pos = hand.palmPosition();
            const Leap::FingerList fingers = hand.fingers();
            // Get fingers

            // Thumb finger
            const Leap::Finger thumb_finger = fingers[0];
            boneType = static_cast<Leap::Bone::Type>(0);
            Leap::Bone thumb_metacarpal_bone = thumb_finger.bone(boneType);
            boneType = static_cast<Leap::Bone::Type>(3);
            Leap::Bone thumb_distal_bone = thumb_finger.bone(boneType);
            Leap::Vector thumb_begin_pos = thumb_metacarpal_bone.prevJoint();
            Leap::Vector thumb_end_pos = thumb_distal_bone.nextJoint();
            Leap::Vector thumb_vector = (thumb_end_pos - thumb_begin_pos).normalized();

            // Index finger
            const Leap::Finger index_finger = fingers[1];
            boneType = static_cast<Leap::Bone::Type>(0);
            Leap::Bone index_metacarpal_bone = index_finger.bone(boneType);
            boneType = static_cast<Leap::Bone::Type>(3);
            Leap::Bone index_distal_bone = index_finger.bone(boneType);
            Leap::Vector index_begin_pos = index_metacarpal_bone.prevJoint();
            Leap::Vector index_end_pos = index_distal_bone.nextJoint();
            Leap::Vector index_vector = index_end_pos - index_begin_pos;

            // Ring finger
            const Leap::Finger ring_finger = fingers[3];
            boneType = static_cast<Leap::Bone::Type>(0);
            Leap::Bone ring_metacarpal_bone = ring_finger.bone(boneType);
            boneType = static_cast<Leap::Bone::Type>(3);
            Leap::Bone ring_distal_bone = ring_finger.bone(boneType);
            Leap::Vector ring_begin_pos = ring_metacarpal_bone.prevJoint();
            Leap::Vector ring_end_pos = ring_distal_bone.nextJoint();
            Leap::Vector ring_vector = ring_end_pos - ring_begin_pos;
            // main_vector is the mean of index vector and ring vector
            Leap::Vector main_vector = (ring_vector + index_vector).normalized();

            float cos_thumb = thumb_vector.dot(main_vector);
            cos_thumb = cos_thumb < 0 ? -cos_thumb : cos_thumb;

            Leap::Vector transv_begin = ring_begin_pos - index_begin_pos;
            Leap::Vector transv_end = ring_end_pos - index_end_pos;
            Leap::Vector transversal = (transv_begin + transv_end).normalized();   

            /** Compute control values **/

            // HAUTEUR
            // TODO si hauteur par acceleration, gestion d'un milieu et clamp entre 1 et -1
            m_height = (palm_pos.y - 50) / 300;
            m_height = m_height > 1.0000001 ? 1.0 : m_height;
            m_height = m_height < -0.0000001 ? 0.0 : m_height;

            // TIR
            if(cos_thumb < 0.8) {
              //std::cout << "SHOOT " << std::endl;
                m_shoot = true;;
            }

            // VITESSE
            if(main_vector.y > -0.1 && main_vector.y < 0.1) {
                //std::cout << "Stationnaire" << std::endl;
                m_speed = 0.f;
            } else {
                if(main_vector.y < -0.1) {
                    m_speed = -main_vector.y;
                } else if(main_vector.y > 0.1) {
                    m_speed = -main_vector.y;
                } 
            }

            // DIRECTION
            if(transversal.y > -0.1 && transversal.y < 0.1) {
                //std::cout << "Tout droit" << std::endl;
                m_direction = 0.f;
            } else {
                if(transversal.y < -0.1) {
                    m_direction = -transversal.y;
                } else if(transversal.y > 0.1) {
                    m_direction = -transversal.y;
                }
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


}