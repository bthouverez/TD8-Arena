#include "leapjoystick.h"
/*
LeapJoystick::LeapJoystick()
{    
    ActivationDiskRadius = 20.0;
    MovementDiskHeight = 0.7;
    MovementDiskRadius = 10.0;
    MovementDiskDonutHoleRadius = 2.0;
    TurnAngleThreshold = 10.0;
    TurnRateOffset = 0.0;
    MaxTurnRate = 1.5;
    TurnAngleToRateScale = 6.0;
    DeactivationBufferHeight = 10.0;
    SpeedScalingFactor = 1.0;
    
    ForwardMovement = 0.f;
    RightMovement = 0.f;
    TurnRate = 0.f;
    IsActivated = false;
    
    ActivationDiskLocation = Vector(60.0, -10.0, 45.0);
}

LeapJoystick::~LeapJoystick()
{

}

float LeapJoystick::GetForwardMovement() {
    return ForwardMovement;
}

float LeapJoystick::GetRightMovement() {
    return RightMovement;
}

float LeapJoystick::GetTurnRate() {
    return TurnRate;
}

void LeapJoystick::CalculateMovementFromHandLocation(Vector PalmLocation, Vector FingerLocation) {
    
    // Next figure out if movement disk is activated
    if (FingerLocation.z > ActivationDiskLocation.z) {
        if (!IsActivated) {
            // for unactivating crossing the Z plane is fine, but for activating want to make sure we are within the disk.
            Vector MovementFingerCharacterLocationOnDisk = FingerLocation;
            MovementFingerCharacterLocationOnDisk.z = ActivationDiskLocation.z;
            if (distance(MovementFingerCharacterLocationOnDisk, ActivationDiskLocation) < ActivationDiskRadius) {
                IsActivated = true;
                DiskLocation = Vector(PalmLocation.x, PalmLocation.y, ActivationDiskLocation.z);
            }
        }
    }
    else {
        if (IsActivated && FingerLocation.z < (ActivationDiskLocation.z - DeactivationBufferHeight)) { // have some small buffer for finger going below disk if it's already activated, to make it more robust
            IsActivated = false;
            TurnRate = 0.0;
            ForwardMovement = 0.0;
            RightMovement = 0.0;
        }
    }
    
    // Next draw Leap Motion Donut
    Vector CylinderStart;
    Vector CylinderEnd;
    Color CylinderColor = White();
    float CylinderRadius;
    if (IsActivated) {
        // transform back to world space so can draw
        CylinderStart = Character->GetTransform().TransformPosition(DiskLocation); // this is top of cylinder
        CylinderEnd = CylinderStart - (Character->GetActorUpVector() * MovementDiskHeight); // this is bottom of cylinder
        CylinderRadius = MovementDiskRadius;
        // draw inner "donut hole" cylinder only in activated case
        DrawDebugCylinder(Character->GetWorld(), CylinderStart, CylinderEnd, MovementDiskDonutHoleRadius, 12, CylinderColor);
    }
    else {
        // transform back to world space so can draw
        CylinderStart = Character->GetTransform().TransformPosition(ActivationDiskLocation); // this is top of cylinder
        CylinderEnd = CylinderStart - (Character->GetActorUpVector() * MovementDiskHeight); // this is bottom of cylinder
        CylinderRadius = ActivationDiskRadius;
    }
    DrawDebugCylinder(Character->GetWorld(), CylinderStart, CylinderEnd, CylinderRadius, 12, CylinderColor);
    DrawDebugLine(Character->GetWorld(),
                  CylinderEnd - (Character->GetActorRightVector() * CylinderRadius),
                  CylinderEnd + (Character->GetActorRightVector() * CylinderRadius),
                  CylinderColor);
    DrawDebugLine(Character->GetWorld(),
                  CylinderEnd - (Character->GetActorForwardVector() * CylinderRadius),
                  CylinderEnd + (Character->GetActorForwardVector() * CylinderRadius),
                  CylinderColor);
    
    // TODO: add proper input mapping so that it can be configured in DefaultInput.ini as a gamepad input would
    // Finally check if we need to add character movement based on movement hand/finger position
    if (IsActivated) {
        // project the movement hand palm location onto the plane of the top of the donut
        Vector MovementHandCharacterLocationOnCylinderTop = PalmLocation;
        MovementHandCharacterLocationOnCylinderTop.z = DiskLocation.z;
        Vector CursorVectorFromCylinderOrigin = MovementHandCharacterLocationOnCylinderTop - DiskLocation;
        Vector MovementCursorPositionCharacter = DiskLocation + CursorVectorFromCylinderOrigin.ClampMaxSize(CylinderRadius);
        // transform "cursor" position back to world space so can draw
        Vector MovementCursorPositionWorld = Character->GetTransform().TransformPosition(MovementCursorPositionCharacter);
        DrawDebugSphere(Character->GetWorld(), MovementCursorPositionWorld, 0.4, 12, Color::Blue);
        // now calculate forward and right movement
        float ForwardPosition = MovementHandCharacterLocationOnCylinderTop.x - DiskLocation.x;
        float RightPosition = MovementHandCharacterLocationOnCylinderTop.y - DiskLocation.y;
        ForwardMovement = CalculateSpeed(ForwardPosition);
        RightMovement = CalculateSpeed(RightPosition);
        // now calculate angle from palm to middle finger and use that to set turn rate
        float MovementHandAngleZ = (FingerLocation.z - PalmLocation.z);
        float MovementHandAngleY = (FingerLocation.y - PalmLocation.y);
        float MovementHandAngle = (atan2(MovementHandAngleY, MovementHandAngleZ) * (180 / PI));
        TurnRate = 0.0;
        if (abs(MovementHandAngle) >= TurnAngleThreshold) { // only turn if angle is greater than some threshold
            float PercentageRotation =  (abs(MovementHandAngle) - TurnAngleThreshold) / 90.0;
            TurnRate = pow(PercentageRotation, 1.5) * TurnAngleToRateScale * copysign(1.0, MovementHandAngle);
            if (TurnRate > MaxTurnRate) {
                TurnRate = MaxTurnRate;
            }
        }
    }
    
}

// TODO: tweak non-linear speed function to make it easier to control movement, especially at slower speeds
float LeapJoystick::CalculateSpeed(float PositionOnMotionDonutAxis) {
    float Speed = 0.0;
    if ((abs(PositionOnMotionDonutAxis) > MovementDiskDonutHoleRadius)) {  // only add forward movement if we are outside of the donut hole
        float PercentagePosition = (abs(PositionOnMotionDonutAxis) - MovementDiskDonutHoleRadius) / (MovementDiskRadius - MovementDiskDonutHoleRadius);
        float SpeedFunctionValue = pow(PercentagePosition, 2.0); // for now just square it to make it non-linear.  TODO: need to tweak
        Speed = SpeedFunctionValue * copysign(SpeedScalingFactor, PositionOnMotionDonutAxis); // apply scaling factor and restore the positive/negative direction sign
    }
    return Speed;
}

*/