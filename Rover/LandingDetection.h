#ifndef BoR_1819_LANDING_DETECTION_GUARD_H
#define BoR_1819_LANDING_DETECTION_GUARD_H

#include "MovingAverage.h"
#include "Constants.h"
#include "Position.h"

enum LandingState {
  NO_TAKEOFF,      // On the ground                    (-> When Alt is greater than ground deadband OR time elapses if alt fail)
  ASCENDING,       // Goin' up                         (-> When Alt surpasses apogee min OR time elapses if alt fail)
  DESCENDING,      // Goin' down past apogee min       (-> When Alt falls in ground deadband first time OR time elapses if alt fail)
  LANDING_WAIT,    // Waiting to stop moving           (-> When Alt some time ago equals Alt now with an allowable deadband OR time elapses if alt fail)
  LANDED           // Landed
};

class LandingDetection {
  public:
    LandingDetection(PositionSensing* pos, Stream* debugStream, DebugDisplay* display) : positionController(pos), state(NO_TAKEOFF), alt_avg(NULL), dstream(debugStream), state_started_ago(millis()), sample_at(millis()), debugDisp(display) {};
    void tick();
    LandingState getState() {
      return state;
    };

  private:
    Stream* dstream;
    DebugDisplay* debugDisp;
    PositionSensing* positionController;
    MovingAverage* alt_avg;
    boolean altimiter_failed_in_this_state = false;
    unsigned long state_started_ago;

    float steady_sample = 0.0;
    unsigned long sample_at;
    
    LandingState state;
};

#endif
