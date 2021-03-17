
#include <stdio.h>

#include "interface.hpp"

double clamp_zero_one(double value) {
    return clamp(value, 0.0, 1.0);
}
