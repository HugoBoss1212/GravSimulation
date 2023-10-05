#pragma once

#include <cmath>
#include <vector>

#include "HugoStable.h"
#include "Particle.h"

class Particle {
public:
    Particle(double A, double W, double x_offset, double y_offset, ParticleType type);
    Particle(double A, double W, double x_offset, double y_offset, double velocity_x, double velocity_y, ParticleType type);
    double valueAt(double x, double y, double t) const;
    void updatePosition(const std::vector<Particle*>& allPeaks, double t, double k, double result[HUGO_STABLE][HUGO_STABLE], double tk);
    static double g0(double x, double y, const std::vector<Particle>& particles, double t);
    
    std::vector<std::pair<double, double>> getHistory() const;
    double getX() const;
    double getY() const;
    double getVY() const;
    double getVX() const;
    double getA() const;
    double getW() const;
    ParticleType getType() const;
    void reset(double A, double W, double x_offset, double y_offset);
    void reset(double A, double W, double x_offset, double y_offset, double velocity_x, double velocity_y);

    static const size_t MAX_HISTORY_SIZE = 1000;

private:
    double A_, W_, x_offset_, y_offset_;
    double velocity_x_ = 0.0, velocity_y_ = 0.0;
    std::vector<std::pair<double, double>> history_;
    ParticleType type_;
    double velocityLockX_ = 0.0; // velocity locks for x and y
    double velocityLockY_ = 0.0;
    Spin spin_;
    double spin_strength_;
    bool isLocked = false;
    double lockedMagnitude = 0.0;
};
