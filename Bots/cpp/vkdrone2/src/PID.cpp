#include <algorithm>
#include <iostream>
#include <cmath>

#include "PID.h"

PID::PID(float kp, float ki, float kd, float outMin, float outMax, float dt,
         bool filterDerivativeSpike) {
    this->Kp = kp;
    this->Ki = ki * dt;
    this->Kd = kd / dt;
    this->Min = outMin;
    this->Max = outMax;
    this->Dt = dt;
    this->DFilter = filterDerivativeSpike;
    this->iTerm = 0.0;
    this->lastInput = 0.0;
    this->firstRun = false;
}

float PID::compute(float setpoint, float input, bool debug) {
    // Proportional term.
    float error = setpoint - input;
    float pTerm = Kp * error;

    // Integral term.
    iTerm += Ki * error;
    iTerm = std::max(std::min(iTerm, Max), Min);

    // Derivative term.
    float dInput = kickFilter(input, setpoint, debug);
    float dTerm = Kd * dInput;
    lastInput = input;

    // Result.
    float output = pTerm + iTerm - dTerm;
    if(debug)std::cout<<" ["<<output<<" = "<<pTerm<<" + "<<iTerm<<" - "<<dTerm<<"] ";
    output = std::max(std::min(output, Max), Min);

    return output;
}

float PID::kickFilter(float input, float setpoint, bool debug) {
    float dInput = input - lastInput;
    if (DFilter && fabs(dInput) >= 0.25 * fabs(lastInput)) {
        if(debug)std::cout<<"<in: "<<input<<" lin: "<<lastInput<<" d1: "<<dInput;
        dInput = 0.01 * fabs(lastInput);
        if (input < lastInput) {
            dInput *= -1.0;
        }
        if(debug)std::cout<<" d2: "<<dInput<<">";
    }
    if (firstRun) {
        dInput = 0.0;
        firstRun = false;
        if(debug)std::cout<<"<in: "<<input<<" lin: "<<lastInput<<">";
    }
    return dInput;
}

void PID::reset() {
    iTerm = 0.0;
    lastInput = 0.0;
    firstRun = true;
}
