/**
 * Wheel Reresents a physical wheel on the rover.
 * 
 * @author Ryan D. Johnson
 * 
 * WPI Battle of the Rockets 2018-2019
 * Team rocket Powered Locomotive
 */

#include "Wheel.h"

/**
 * Setup the wheel (Run at the setup function in main)
 */
void Wheel::setup() {
  this->motor.attach(pin, 1000, 2000);
}

void Wheel::invert() {
  this->inverted = !this->inverted;
}

/**
 * Increment the controller - run this iteratively. (In void loop)
 * 
 * This function either calculates the next PID output value or 
 * sets the open loop control
 */
void Wheel::tick() {
  double thisTime = millis();
  double rot = getRotations();
  double rpm;
  if ((thisTime-lastTime) > RPM_AVG_MILLIS) {
    double rpm_t = (60000.0) * ((rot - lastRotation) / (thisTime - lastTime));
    
    if (isnan(rpm_t) || isinf(rpm_t)) {
      rpm = lastRPM;
    } else {
      rpm = rpm_t;
    }
  } else {
    rpm = lastRPM;
  }
  switch (mode) {
    case RPM: {
        double delta = rpm - lastRPM;
        double error = setpoint - lastRPM;
        if (!isnan(error))
          errorAccumulator += error;

        errorAccumulator = max(-ERROR_BAND, errorAccumulator);
        errorAccumulator = min(ERROR_BAND, errorAccumulator);

        double pid = consts.Kf + consts.Kp * error + consts.Ki * errorAccumulator + consts.Kd * delta;
        pid = max(pid, -90);
        pid = min(pid, 90);
        motor.write(90 + ((this->inverted)? -1.0 : 1.0) *pid);
      }
      break;
    case PercentVBus: {
        errorAccumulator = 0;
        motor.write( 90 + setpoint);
      }
      break;
  }

  
  if ((thisTime-lastTime) > RPM_AVG_MILLIS) {
    lastTime = thisTime;
    lastRotation = rot;
  }
  lastRPM = rpm;
}

/**
 * Sets the controller to open loop (Percent vBus mode)
 * Speed is -90 to 90.
 */
void Wheel::setOpenLoop(double speed) {
  mode = PercentVBus;
  setpoint = ((this->inverted)? -1.0 : 1.0) * speed;
  
}

/**
 * Sets the controller to RPM PID control mode.
 */
void Wheel::setRPM(double rpm) {
  mode = RPM;
  setpoint = rpm;
  errorAccumulator = 0;
}

/**
 * Sets the controller to open loop (Percent vBus mode) and the speed to 0
 */
void Wheel::stop() {
  mode = PercentVBus;
  setpoint = 0;
}

/**
 * Gets the number of rotations the encoder has moved since the start
 * of the program or the last time it has been reset.
 */
double Wheel::getRotations() {
  return ((this->inverted)? -1.0 : 1.0) * encoder.read() / (60.0 * 180.0);
}


/**
 * Resets the encoder / rotation count
 */
void Wheel::resetEncoder() {
  encoder.write(0);
}
