#ifndef PID_
#define PID_

class PID {
  public:
    PID(float kp, float ki, float kd, float outMin, float outMax, float dt,
        bool filterDerivativeSpike = false);
    float compute(float setpoint, float input, bool debug = false);
    float kickFilter(float input, float setpoint, bool debug = false);
    void reset();

  private:
    float Kp;
    float Ki;
    float Kd;
    float Min;
    float Max;
    float Dt; // In seconds.
    bool  DFilter;

    float iTerm;
    float lastInput;
    bool firstRun;
};

#endif
