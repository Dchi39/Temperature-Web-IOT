#include "PIDController.h"
#include <Arduino.h>

PIDController::PIDController(double Kp_, double Ki_, double Kd_, unsigned long sampleTime_) {
    Kp = Kp_; Ki = Ki_; Kd = Kd_;
    sampleTime = sampleTime_;
    integral = 0;
    lastError = 0;
    output = 0;
    setpoint = 0;
    lastTime = millis();
}

void PIDController::setTunings(double Kp_, double Ki_, double Kd_) {
    Kp = Kp_;
    Ki = Ki_;
    Kd = Kd_;
}

void PIDController::setSetpoint(double sp) {
    setpoint = sp;
}

void PIDController::setInput(double in) {
    input = in;
}

void PIDController::setOutputLimits(double min, double max) {
    if (output < min) output = min;
    if (output > max) output = max;
}

double PIDController::compute() {
    unsigned long now = millis();
    unsigned long dt = now - lastTime;
    if (dt >= sampleTime) {
        double error = setpoint - input;
        integral += error * (dt / 1000.0);
        double derivative = (error - lastError) / (dt / 1000.0);

        output = Kp * error + Ki * integral + Kd * derivative;

        // Clamp output
        if (output > 255) output = 255;
        if (output < 0) output = 0;

        lastError = error;
        lastTime = now;
    }
    return output;
}

double PIDController::getOutput() {
    return output;
}