#ifndef LAVS_SCENE_H_
#define LAVS_SCENE_H_

#include "sx/math-vec.h"

#define NUM_MATERIALS 2
#define NUM_SPHERES 2

typedef struct
{
    sx_vec3 albedo;
    float roughness;
    float metallic;
} Material;

typedef struct
{
    sx_vec3 position;
    sx_vec3 velocity;
    float radius;

    int material_index;
} Sphere;

typedef struct
{
    Sphere spheres[NUM_SPHERES];
    Material materials[NUM_MATERIALS];
} Scene;

#endif