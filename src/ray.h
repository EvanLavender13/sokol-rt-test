#ifndef LAVS_RAY_H_
#define LAVS_RAY_H_

#include "sx/sx/math-vec.h"

typedef struct
{
    sx_vec3 origin;
    sx_vec3 direction;
} Ray;

#endif