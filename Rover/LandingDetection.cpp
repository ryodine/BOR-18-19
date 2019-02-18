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
    if (positionController->getErrorFlags() & ALTIMITER_ERROR_MASK) {
      if (!altimiter_failed_in_this_state) {
        dstream->print("Altimiter Broke! Fallback to time for this landing detection state:");
        dstream->println(state);
        altimiter_failed_in_this_state = true;
      }
    } 
  }

  // STATES
  switch (state) {
    case NO_TAKEOFF:
      if (alt_avg->getAverage() > LD_GROUND_TRIG_DEADBAND || (altimiter_failed_in_this_state && (millis()-state_started_ago) > LD_TIME_FALLBACK_PRE_TAKEOFF * 1000)) {
        state = ASCENDING;
        state_started_ago = millis();
        altimiter_failed_in_this_state = false;
      }
      break;
    case ASCENDING:
      if (alt_avg->getAverage() > LD_APOGEE_MINIMUM || (altimiter_failed_in_this_state && (millis()-state_started_ago) > LD_TIME_FALLBACK_APOGEE * 1000)) {
        state = DESCENDING;
        state_started_ago = millis();
        altimiter_failed_in_this_state = false;
      }
      break;
    case DESCENDING:
      if (alt_avg->getAverage() <= LD_GROUND_TRIG_DEADBAND || (altimiter_failed_in_this_state && (millis()-state_started_ago) > LD_TIME_FALLBACK_DESCEND * 1000)) {
        state = LANDING_WAIT;
        state_started_ago = millis();
        steady_sample = alt_avg->getAverage();
        sample_at = millis();
        altimiter_failed_in_this_state = false;
      }
      break;
    case LANDING_WAIT:
      if (altimiter_failed_in_this_state) {
        if ((millis() - state_started_ago) > LD_TIME_FALLBACK_GROUND_DELAY * 1000) {
          state = LANDED;
          state_started_ago = millis();
          altimiter_failed_in_this_state = false;
        }
      } else if ((millis() - sample_at) > (LD_GROUND_DELAY * 1000)) {
        if (abs(alt_avg->getAverage() - steady_sample) < LD_GROUND_STEADY_DEADBAND) {
          state = LANDED;
          state_started_ago = millis();
          altimiter_failed_in_this_state = false;
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
      dstream->println("Don't know how to handle this state");
  }
  
}
