#pragma once

#include "vec4.h"
#include "colour.h"

// keep light straightforward - struct for storing information
struct Light {
    vec4 omega_i; // light direction
    color L; // light colour
    color ambient; // ambient light component 
};

