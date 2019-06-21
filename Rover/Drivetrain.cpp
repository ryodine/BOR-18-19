#include "Drivetrain.h"

void Drivetrain::setup() {
  getLeftWheel().setup();
  getRightWheel().setup();
  getRightWheel().invert();
}

void Drivetrain::tick() {
  getLeftWheel().tick();
  getRightWheel().tick();
  if (millis()%50) {
    Serial.print(getLeftWheel().getRotations());
    Serial.print(", ");
    Serial.print(getRightWheel().getRotations());
    Serial.print(", ");
    Serial.print(set_delta);
    Serial.print(", ");
    Serial.print(getLeftWheel().getControlMode());
    Serial.print(", ");
    Serial.print(getRightWheel().getControlMode());
    Serial.print(", ");
    Serial.println(getControlMode());
  }

  switch (mode) {
    case DCR_ZERO: {
      if (getLeftWheel().getControlMode() == Wheel::PercentVBus && getRightWheel().getControlMode()  == Wheel::PercentVBus) {
        stop();
      }
    } break;
    case DCR_STOP: {
      // Do nothing
    } break;
    case DCR_SYNCHRO: {
      double positional_error = getLeftWheel().getRotations() - getRightWheel().getRotations() - set_delta;
      const double k_p = 30.0;
      double pid_result = positional_error * k_p;
      if (pid_result > setpoint) pid_result = setpoint;
      getLeftWheel().setRPM(setpoint - pid_result);
      getRightWheel().setRPM(setpoint + pid_result);
    } break;
  }
}

void Drivetrain::zero() {
  getLeftWheel().zero();
  getRightWheel().zero();
  mode = DCR_ZERO;
}

void Drivetrain::stop() {
  mode = DCR_STOP;
  getLeftWheel().stop();
  getRightWheel().stop();
}

void Drivetrain::sit() {
  zero();
}

void Drivetrain::setSynchronizedRPM(double rpm) {
  set_delta = getLeftWheel().getRotations() - getRightWheel().getRotations();
  mode = DCR_SYNCHRO;
  setpoint = rpm;
}
