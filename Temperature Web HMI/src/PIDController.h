#pragma once

class PIDController {
private:
    double Kp, Ki, Kd;
    double setpoint;
    double input;
    double output;
    double integral;
    double lastError;

    unsigned long lastTime;
    unsigned long sampleTime; // ms

public:
    PIDController(double Kp, double Ki, double Kd, unsigned long sampleTime = 500);

    void setTunings(double Kp, double Ki, double Kd);
    void setSetpoint(double sp);
    void setInput(double in);
    void setOutputLimits(double min, double max);

    double compute(); // returns output
    double getOutput();
};