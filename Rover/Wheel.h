#ifndef BoR_1819_WHEEL_CONTROLLER_GUARD_H
#define BoR_1819_WHEEL_CONTROLLER_GUARD_H

/**
 * Wheel Reresents a physical wheel on the rover.
 * 
 * @author Ryan D. Johnson
 * 
 * WPI Battle of the Rockets 2018-2019
 * Team rocket Powered Locomotive
 */

#include <Encoder.h>
#include <Servo.h>
#include "Constants.h"


class Wheel {
  public:
    Wheel(int controllerPin, uint8_t encA, uint8_t encB, uint8_t hall_effect, PIDConstant consts) 
      : encoder(encA, encB) {
        this->consts = consts;
        this->pin = controllerPin;
        this->hall_pin = hall_effect;
        pinMode(hall_pin, INPUT);
    }
    void setup();
    void zero();
    void tick();
    void moveRotations(double rpm, double rotations);
    void setRPM(double rpm);
    void setOpenLoop(double percentVbus);
    void stop();
    double getLastRPM() { return lastRPM; }
    double getRotations();
    void resetEncoder();
    void invert();
  private:
    //Config
    int pin;
    int hall_pin;
    bool hall_latch;
    double rotation_bias;
    bool zeroed;

    
    Servo motor;
    Encoder encoder;
    PIDConstant consts;
    
    //PID State
    double lastRotation = 0.0;
    unsigned long lastTime = 0;
    
    double lastRPM = 0.0;
    double errorAccumulator = 0.0;

    bool inverted = false;

    //Control Loop Mode
    enum ControlMode { PercentVBus, RPM };
    ControlMode mode = PercentVBus;
    double setpoint = 0.0;
};

#endif
