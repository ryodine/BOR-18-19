#ifndef BoR_1819_DRIVETRAIN_GUARD_H
#define BoR_1819_DRIVETRAIN_GUARD_H

#include "Wheel.h"
#include "Constants.h"

class Drivetrain {
  public:
    Drivetrain() : left(LEFT_PWM,  LEFT_ENC_A,  LEFT_ENC_B,  LEFT_HALL,  leftwheel ), right(RIGHT_PWM, RIGHT_ENC_A, RIGHT_ENC_B, RIGHT_HALL, rightwheel) {};

    enum DrivetrainControlMode {DCR_STOP, DCR_SYNCHRO, DCR_ZERO};
    
    void tick();
    void setup();
    void zero();
    void stop();
    void setSynchronizedRPM(double rpm);
    void sit();
    DrivetrainControlMode getControlMode() { return mode; }
    
    Wheel& getLeftWheel()  { return left;  }
    Wheel& getRightWheel() { return right; }
  private:

    DrivetrainControlMode mode = DCR_STOP;
    double setpoint = 0.0;
    double set_delta = 0.0;
    
    // Initialize wheels based on constants
    Wheel left;
    Wheel right;
};

#endif
