#ifndef LAVS_SIM_H_
#define LAVS_SIM_H_

#include "flecs.h"
#include "sx/math-vec.h"
#include "scene.h"

// typedef struct
// {
//     sx_vec3 **positions;
//     sx_vec3 **velocities;
//     int num_objects;
// } SimData;

struct
{
    ecs_world_t *world;

    sx_vec3 control;
} Sim;

typedef struct
{
    sx_vec3 *current;
} Position;

typedef struct
{
    sx_vec3 *current;
} Velocity;

void Move(ecs_iter_t *iter)
{
    Position *p = ecs_term(iter, Position, 1);
    Velocity *v = ecs_term(iter, Velocity, 2);

    for (int i = 0; i < iter->count; i++)
    {
        p[i].current->x += v[i].current->x * iter->delta_time;
    }
}

void sim_init(Scene *scene)
{
    ecs_world_t *world = ecs_init();
    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_SYSTEM(world, Move, EcsOnUpdate, Position, Velocity);

    for (int i = 0; i < NUM_SPHERES; i++) 
    {
        Sphere *sphere = &scene->spheres[i];
        ecs_entity_t entity = ecs_new_id(world);
        ecs_set(world, entity, Position, { &sphere->position });
        ecs_set(world, entity, Velocity, { &sphere->velocity });
    }

    // Sim.data = sim_data;
    Sim.world = world;
}

void sim_update(float dt)
{
    ecs_progress(Sim.world, dt);
}

void sim_shutdown()
{
    ecs_fini(Sim.world);
    // free(Sim.data->positions);
    // free(Sim.data->velocities);
}

#endif