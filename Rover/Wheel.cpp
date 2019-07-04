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
  motor.attach(pin, 1000, 2000);
}

void Wheel::invert() {
  inverted = !inverted;
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

  if (!digitalRead(hall_pin) && !hall_latch) {
    double rot = getRotations();
    rotation_bias -= (1.0*round(rot) - rot);
    hall_latch = true;
  } else if (digitalRead(hall_pin)) {
    hall_latch = false;
  }
  
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
    case Commutate: {
      if (abs(rpm) > 20) {
        mode = stateFinishMode;
      }
    } break;
    case PercentVBus: {
      errorAccumulator = 0;
      motor.write( 90 + 60.0*setpoint);
    }
    break;
    case Zero: {
      motor.write(((this->inverted)? -1.0 : 1.0) * 165);
      if (!digitalRead(hall_pin)) {
        zeroed = true;
        mode = PercentVBus;
        setpoint = 0.0;
      }
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
 * Speed is -1 to 1.
 */
void Wheel::setOpenLoop(double speed) {
  mode = PercentVBus;
  if (speed > 1.0)  speed = 1.0;
  if (speed < -1.0) speed = -1.0;
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
  return ((this->inverted)? -1.0 : 1.0) * encoder.read() / (131.0 * 64.0) - rotation_bias;
}


/**
 * Resets the encoder / rotation count
 */
void Wheel::resetEncoder() {
  encoder.write(0);
}

void Wheel::zero() {
  zeroed = false;
  mode = Zero;
}

boolean Wheel::isStowed() {
  return !digitalRead(hall_pin);
}

void Wheel::CommutateTo(ControlMode next) {
  mode = Commutate;
  stateFinishMode = next;
  lastTime = millis();
  motor.write(((this->inverted)? -1.0 : 1.0) * 135);
}
