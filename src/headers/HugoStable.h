#pragma once

const unsigned int HUGO_STABLE = 1000;  //ilość iteracji musi sie równać powieszchni liczonej
const double chunkiBoi = 0.001; // This will act as a multiplier. If it's 1.0, it means no reduction. If it's 0.5, the velocity change will be halved.

enum ParticleType {
    A1, A2, A3
};

enum Spin {
    LEFT,
    RIGHT
};