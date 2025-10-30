#pragma once
#include "matrix.h"
#include "RNG.h"

// Utility function to generate a random rotation matrix
// No input variables
static matrix makeRandomRotation() {
	RandomNumberGenerator& rng = RandomNumberGenerator::getInstance();
	unsigned int r = rng.getRandomInt(0, 3);

	switch (r) {
	case 0: return matrix::makeRotateX(rng.getRandomFloat(0.f, 2.0f * M_PI));
	case 1: return matrix::makeRotateY(rng.getRandomFloat(0.f, 2.0f * M_PI));
	case 2: return matrix::makeRotateZ(rng.getRandomFloat(0.f, 2.0f * M_PI));
	default: return matrix::makeIdentity();
	}
}


