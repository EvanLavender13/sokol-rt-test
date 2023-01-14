#ifndef LAVS_RNG_H_
#define LAVS_RNG_H_

#include "stdlib.h"
#include "sx/math-vec.h"
#include "sx/rng.h"

static float random_float()
{
    return (float) rand() / (float) RAND_MAX;
}

// TODO: use sx rng?
static sx_vec3 random_vec3(float min, float max)
{
    return sx_vec3f(
        random_float() * (max - min) + min,
        random_float() * (max - min) + min,
        random_float() * (max - min) + min
    );
}

#endif