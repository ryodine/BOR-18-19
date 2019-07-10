#include "Drivetrain.h"

void Drivetrain::setup() {
  getLeftWheel().setup();
  getRightWheel().setup();
  getRightWheel().invert();
}

void Drivetrain::tick() {
  getLeftWheel().tick();
  getRightWheel().tick();
  /*if (millis()%500) {
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
  }*/

  switch (mode) {
    case DCR_ZERO: {
      if (getLeftWheel().getControlMode() == Wheel::PercentVBus && getRightWheel().getControlMode()  == Wheel::PercentVBus) {
        stop();
      }
    } break;
    case DCR_STOP: {
      // Do nothing
    } break;
    case DCR_COMMUTATE: {
      if (getLeftWheel().getControlMode() == onCommutateFinishCommand && getRightWheel().getControlMode()  == onCommutateFinishCommand) {
        switch (onCommutateFinish) {
          case DCR_ZERO: {
            getLeftWheel().zero();
            getRightWheel().zero();
            mode = DCR_ZERO;
          } break;
          case DCR_SYNCHRO: {
            set_delta = getLeftWheel().getRotations() - getRightWheel().getRotations();
            mode = DCR_SYNCHRO;
          } break;
        }
      }
    } break;
    case DCR_SYNCHRO: {
      double positional_error = getLeftWheel().getRotations() - getRightWheel().getRotations() - set_delta;
      const double k_p = 30.0;
      double pid_result = positional_error * k_p;

      if (pid_result > setpoint) pid_result = setpoint;
      //if (pid_result < 0) pid_result = setpoint;

      getLeftWheel().setRPM(setpoint - pid_result);
      getRightWheel().setRPM(setpoint + pid_result);
      if (millis()%500) {
        Serial.print(getLeftWheel().getLastRPM());
        Serial.print(", ");
        Serial.print(getRightWheel().getLastRPM());
        Serial.print(", ");
        Serial.print(getLeftWheel().getControlMode());
        Serial.print(", ");
        Serial.print(getRightWheel().getControlMode());
        Serial.print(", ");
        Serial.print(setpoint);
        Serial.print(", ");
        Serial.println(positional_error);
      }
    } break;
  }
}

void Drivetrain::zero() {
  onCommutateFinishCommand = Wheel::PercentVBus;
  onCommutateFinish = DCR_ZERO;
  getLeftWheel().CommutateTo(onCommutateFinishCommand);
  getRightWheel().CommutateTo(onCommutateFinishCommand);
  mode = DCR_COMMUTATE;
}

void Drivetrain::stop() {
  mode = DCR_STOP;
  onCommutateFinish = DCR_STOP;
  getLeftWheel().stop();
  getRightWheel().stop();
}

void Drivetrain::sit() {
  zero();
}

void Drivetrain::setSynchronizedRPM(double rpm) {
  onCommutateFinishCommand = Wheel::RPM;
  onCommutateFinish = DCR_SYNCHRO;
  getLeftWheel().CommutateTo(onCommutateFinishCommand);
  getRightWheel().CommutateTo(onCommutateFinishCommand);
  mode = DCR_COMMUTATE;
  setpoint = rpm;
}
