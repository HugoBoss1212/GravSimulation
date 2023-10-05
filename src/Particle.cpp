#include <cmath>
#include <vector>
#include <iostream>
#include <random>

#include "Particle.h"
#include "Quadtree.h"
#include "Timer.h"

Particle::Particle(double A, double W, double x_offset, double y_offset, ParticleType type)
    : A_(A), W_(W), x_offset_(x_offset), y_offset_(y_offset), velocity_x_(0.0), velocity_y_(0.0), type_(type) {}

Particle::Particle(double A, double W, double x_offset, double y_offset, double velocity_x, double velocity_y, ParticleType type)
    : A_(A), W_(W), x_offset_(x_offset), y_offset_(y_offset), velocity_x_(velocity_x), velocity_y_(velocity_y), type_(type) {
        std::random_device rd;
        std::mt19937 mt(rd());
        std::uniform_int_distribution<size_t> dist(1, 0);

        spin_ = dist(mt) == 1 ? LEFT : RIGHT;
        if(type == ::A1) { // medium
            velocityLockX_ = 1;
            velocityLockY_ = 1;
            spin_strength_ = 0.01;
        } else if(type == ::A2) { // small
            velocityLockX_ = 30; 
            velocityLockY_ = 30;
            spin_strength_ = 0.01;
        } else if(type == ::A3) {   //chunki boi
            velocityLockX_ = 0.001;
            velocityLockY_ = 0.001;
            spin_strength_ = 0.0001;
        }
    }

//KL ----------------------------------------------<<<<<<<<<<<<<<<<
double Particle::valueAt(double x, double y, double t) const {
    return A_ * A_ * exp(-(pow(x - (x_offset_), 2) + pow(y - (y_offset_), 2)) / (2 * W_ * W_));
}

void Particle::updatePosition(const std::vector<Particle*>& allPeaks, double t, double k, double result[HUGO_STABLE][HUGO_STABLE], double tk) {
    double total_force_x = 0.0;
    double total_force_y = 0.0;

    //Timer functionTimer1;
    // 1. Construct a QuadTree for the particles
    Boundary boundary(0, 0, HUGO_STABLE, HUGO_STABLE);  // Assuming the particles are within this boundary
    QuadTreeNode qtree(boundary, 4);  // capacity of 4 is a common choice
    for(Particle* peak : allPeaks) {
        qtree.insert(peak);
    }
    //std::cout << "Construct a QuadTree completed in: " << functionTimer1.elapsed() << " microseconds." << std::endl;

    //Timer functionTimer2;
    // 2. Use the query method to get only the nearby particles ----------------------------------------------<<<<<<<<<<<<<<<<
    double querySize = 10.0;  // Define a suitable range based on your needs
    Boundary queryBoundary(x_offset_, y_offset_, querySize, querySize);
    std::vector<Particle*> nearbyParticles;
    qtree.query(queryBoundary, nearbyParticles);
    //std::cout << "get only the nearby particles completed in: " << functionTimer2.elapsed() << " microseconds." << std::endl;

    //Timer functionTimer3;
    // 3. Compute the forces based on these nearby particles    ----------------------------------------------<<<<<<<<<<<<<<<<
    const double G = 6.674e-11;  // Placeholder for Gravitational constant (You might want to adjust this for your simulation)
    for(Particle* peakPtr : nearbyParticles) {
        if(peakPtr != this) {
            double dx = x_offset_ - peakPtr->x_offset_;
            double dy = y_offset_ - peakPtr->y_offset_;

            double distanceSquared = dx*dx + dy*dy;
            double distance = sqrt(distanceSquared);

            // Avoiding division by zero
            if(distanceSquared == 0) continue;

            // Calculate "masses" using the valueAt function
            double m1 = valueAt(x_offset_, y_offset_, t);
            double m2 = peakPtr->valueAt(peakPtr->getX(), peakPtr->getY(), t);

            // Newton's gravitational force
            double force_magnitude = G * m1 * m2 / distanceSquared;

            total_force_x += force_magnitude * dx / distance;
            total_force_y += force_magnitude * dy / distance;

            std::cout << sqrt(velocity_x_ * velocity_x_ + velocity_y_ * velocity_y_) << ";" << peakPtr->getType() << ";" << tk << std::endl;
            sqrt(velocity_x_ * velocity_x_ + velocity_y_ * velocity_y_);
        }
    }
    //std::cout << "Compute the forces completed in: " << functionTimer3.elapsed() << " microseconds." << std::endl;

    int i = static_cast<int>(x_offset_ + HUGO_STABLE / 2.0);
    int j = static_cast<int>(y_offset_ + HUGO_STABLE / 2.0);

    double gradient_x = 0.0;
    double gradient_y = 0.0;

    //Timer functionTimer4;
    // Gradient computation using finite differences
    if (i > 0 && i < HUGO_STABLE - 1) {
        gradient_x = (result[i + 1][j] - result[i - 1][j]) / 2.0;
    }

    if (j > 0 && j < HUGO_STABLE - 1) {
        gradient_y = (result[i][j + 1] - result[i][j - 1]) / 2.0;
    }
    //std::cout << "Gradient computation completed in: " << functionTimer4.elapsed() << " microseconds." << std::endl;

    //Timer functionTimer5;
    // Update velocity based on gradient
    double velocityChangeX = gradient_x * t;
    double velocityChangeY = gradient_y * t;

    // Ensure velocity changes don't exceed locks
    if(abs(velocityChangeX) > velocityLockX_) {
        velocityChangeX = (velocityChangeX > 0) ? velocityLockX_ : -velocityLockX_;
    }

    if(abs(velocityChangeY) > velocityLockY_) {
        velocityChangeY = (velocityChangeY > 0) ? velocityLockY_ : -velocityLockY_;
    }

    // Adjust velocity based on spin and its strength
    double spinEffect = spin_strength_ * t;
    if(spin_ == LEFT) {
        // If spin is left, we'll decrease the x velocity and increase the y velocity
        velocity_x_ -= spinEffect;
        velocity_y_ += spinEffect;
    } else if(spin_ == RIGHT) {
        // If spin is right, we'll increase the x velocity and decrease the y velocity
        velocity_x_ += spinEffect;
        velocity_y_ -= spinEffect;
    }

    if (type_ == A3) {
        velocityChangeX *= chunkiBoi;
        velocityChangeY *= chunkiBoi;
    }

    velocity_x_ += velocityChangeX;
    velocity_y_ += velocityChangeY;
    
    // Adjust velocities if they surpass the lock
    double currentMagnitude = sqrt(velocity_x_ * velocity_x_ + velocity_y_ * velocity_y_);

    if (!isLocked) {
        if (abs(velocity_x_) > velocityLockX_ || abs(velocity_y_) > velocityLockY_) {
            isLocked = true;
            lockedMagnitude = currentMagnitude;

            // Adjust velocities proportionally
            double ratioX = velocity_x_ / currentMagnitude;
            double ratioY = velocity_y_ / currentMagnitude;
            
            velocity_x_ = ratioX * lockedMagnitude;
            velocity_y_ = ratioY * lockedMagnitude;
        }
    } else {
        // If we are locked and either velocity is now below the lock, unlock
        if (abs(velocity_x_) <= velocityLockX_ && abs(velocity_y_) <= velocityLockY_) {
            isLocked = false;
        } else {
            // If still locked, adjust velocities to keep the same locked magnitude
            double ratioX = velocity_x_ / currentMagnitude;
            double ratioY = velocity_y_ / currentMagnitude;

            velocity_x_ = ratioX * lockedMagnitude;
            velocity_y_ = ratioY * lockedMagnitude;
        }
    }

    //std::cout << "Update velocity completed in: " << functionTimer5.elapsed() << " microseconds." << std::endl;
    //Timer functionTimer6;
    // Update position based on velocity
    x_offset_ += velocity_x_ * t;
    y_offset_ += velocity_y_ * t;

    //std::cout << "Update position completed in: " << functionTimer6.elapsed() << " microseconds." << std::endl;
    history_.emplace_back(x_offset_, y_offset_);
    if (history_.size() > MAX_HISTORY_SIZE) {
        history_.erase(history_.begin());
    }
}

double Particle::getX() const { 
    return x_offset_; 
}

double Particle::getY() const { 
    return y_offset_;
}

ParticleType Particle::getType() const {
    return type_;
}

double Particle::getVY() const { 
    return velocity_x_;
}

double Particle::getVX() const { 
    return velocity_y_;
}


double Particle::getA() const { 
    return A_;
}


double Particle::getW() const { 
    return W_;
}


std::vector<std::pair<double, double>> Particle::getHistory() const { 
    return history_;
}

double Particle::g0(double x, double y, const std::vector<Particle>& particles, double t) {
    double sum = 0.0;
    for (size_t i = 0; i < particles.size(); ++i) {
        for (size_t j = i + 1; j < particles.size(); ++j) {
            //Timer functionTimer7;
            sum += particles[i].valueAt(x, y, t) * particles[j].valueAt(x, y, t);  // Fixed the order of x and y
            //std::cout << "g0 single completed in: " << functionTimer7.elapsed() << " microseconds." << std::endl;
        }
    }
    return sum;
}

void Particle::reset(double A, double W, double x_offset, double y_offset) {
    A_ = A;
    W_ = W;
    x_offset_ = x_offset;
    y_offset_ = y_offset;
    velocity_x_ = 0.0;
    velocity_y_ = 0.0;
}

void Particle::reset(double A, double W, double x_offset, double y_offset, double velocity_x, double velocity_y) {
    A_ = A;
    W_ = W;
    x_offset_ = x_offset;
    y_offset_ = y_offset;
    velocity_y_ = velocity_y;
    velocity_x_ = velocity_x;
}