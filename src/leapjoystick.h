#ifndef LEAP_JOYSTICK_H
#define LEAP_JOYSTICK_H

#include <vec.h>
#include <color.h>

class LeapJoystick
{
public:
    /*
     Constructor takes Character as parameter since the actual virtual joystick is positioned relative to the character
     */
    LeapJoystick();
    ~LeapJoystick();
    /*
     This method calculates the forward and right movement speeds as well as the turn rate.  The intended use is that this method should be called first, and then the values retrieved through the 3 corresponding Getter methods.
     */
    void CalculateMovementFromHandLocation(Vector PalmLocation, Vector FingerLocation);
    
    float GetForwardMovement();
    float GetRightMovement();
    float GetTurnRate();
    
    float ActivationDiskRadius;
    float MovementDiskHeight;
    float MovementDiskRadius;
    float MovementDiskDonutHoleRadius;
    float TurnAngleThreshold;
    float TurnRateOffset;
    float MaxTurnRate;
    float TurnAngleToRateScale;
    float DeactivationBufferHeight;
    float SpeedScalingFactor;
    /*
     Location to draw activation disk
     */
    Vector ActivationDiskLocation;
    
protected:
    
    /*
     The speed function is nonlinear so that the character can move fluidly either fast or slow.
     */
    float CalculateSpeed(float PositionOnMotionDonutAxis);
    float ForwardMovement;
    float RightMovement;
    float TurnRate;
    bool IsActivated;
    Vector DiskLocation; // in Character space
};

#endif