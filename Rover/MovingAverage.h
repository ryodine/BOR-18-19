#ifndef BoR_1819_MOVING_AVERAGE_GUARD_H
#define BoR_1819_MOVING_AVERAGE_GUARD_H

class MovingAverage {
  public:
    MovingAverage(float startingPoint, float a) : avg(startingPoint), alpha(a){};
    void addPoint(float point) {
      avg = point * alpha + avg * (1.0 - alpha);
    };
    float getAverage() { return avg; };
  private:
    float alpha; // Lots of smoothing is lower
    float avg;
};

#endif
