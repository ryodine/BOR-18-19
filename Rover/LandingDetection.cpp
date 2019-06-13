#include "LandingDetection.h"

void LandingDetection::tick() {
  // !! Assume Position controller is being "ticked" external to this

  // ALTIMITER MANAGEMENT
  if (alt_avg == NULL) {
    alt_avg = new MovingAverage(positionController->getAltitude(), LD_EWMA_SMOOTHING_ALPHA);
  } else {
    alt_avg->addPoint(positionController->getAltitude());
  }
  if (positionController->hasError()) {
    if (positionController->getErrorFlags() & (ALTIMITER_ERROR_MASK | ALTIMITER_FAIL_MASK)) {
      if (!altimiter_failed_in_this_state) {
        dstream->println("Altimiter Broke! Fallback to time for all future landing detection states");
        altimiter_failed_in_this_state = true;
      }
    } 
  }

  if (photoResistorArmed && !digitalRead(PHOTORESISTOR_IN) && state != DESCENDING && state != LANDING_WAIT && state != LANDED) {
    state = DESCENDING;
    photoResistorArmed = false;
    state_started_ago = millis();
  }

  // STATES
  switch (state) {
    case NO_TAKEOFF:
      if (alt_avg->getAverage() > LD_GROUND_TRIG_DEADBAND/* || (altimiter_failed_in_this_state && (millis()-state_started_ago) > LD_TIME_FALLBACK_PRE_TAKEOFF * 1000)*/) {       // Pre-takeoff timing is sketchy
        state = ASCENDING;
        dstream->println("NOW ASCENDING");
        state_started_ago = millis();
        //altimiter_failed_in_this_state = false;
      }
      break;
    case ASCENDING:
      if ((altimiter_failed_in_this_state && (millis()-state_started_ago) > LD_TIME_FALLBACK_APOGEE * 1000) || (!altimiter_failed_in_this_state && (alt_avg->getAverage() > LD_APOGEE_MINIMUM))) {
        dstream->println("NOW DESCENDING");
        state = DESCENDING;
        state_started_ago = millis();
        //altimiter_failed_in_this_state = false;
      }
      break;
    case DESCENDING:
      if ((!altimiter_failed_in_this_state && (alt_avg->getAverage() <= LD_GROUND_TRIG_DEADBAND)) || (altimiter_failed_in_this_state && ((millis()-state_started_ago) > LD_TIME_FALLBACK_DESCEND * 1000))) {
        state = LANDING_WAIT;
        dstream->println("NOW WAITING");
        state_started_ago = millis();
        steady_sample = alt_avg->getAverage();
        sample_at = millis();
        //altimiter_failed_in_this_state = false;
      }
      break;
    case LANDING_WAIT:
      if (altimiter_failed_in_this_state) {
        if ((millis() - state_started_ago) > LD_TIME_FALLBACK_GROUND_DELAY * 1000) {
          state = LANDED;
          state_started_ago = millis();
          //altimiter_failed_in_this_state = false;
        }
      } else if ((millis() - sample_at) > (LD_GROUND_DELAY * 1000)) {
        if (abs(alt_avg->getAverage() - steady_sample) < LD_GROUND_STEADY_DEADBAND) {
          state = LANDED;
          state_started_ago = millis();
          //altimiter_failed_in_this_state = false;
        } else {
          steady_sample = alt_avg->getAverage();
          sample_at = millis();
        }
      }
      
      break;
    case LANDED:
      // hooray!
      break;
    default:
      if (dstream != NULL)
        dstream->println("Don't know how to handle this state");
  }
  
}

void LandingDetection::begin() {
  pinMode(PHOTORESISTOR_IN, INPUT);
}
