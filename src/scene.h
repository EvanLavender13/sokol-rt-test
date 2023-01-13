#ifndef LAVS_SCENE_H_
#define LAVS_SCENE_H_

#include "sx/math-vec.h"

#define NUM_SPHERES 2

typedef struct
{
    sx_vec3 position;
    float radius;

    sx_vec3 albedo;
} Sphere;

typedef struct
{
    Sphere spheres[NUM_SPHERES];
} Scene;

#endif