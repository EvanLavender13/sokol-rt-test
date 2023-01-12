#ifndef LAVS_CAMERA_H_
#define LAVS_CAMERA_H_

#include "sx/sx/math-vec.h"
#include "config.h"
#include "ray.h"

typedef struct 
{
    // mat4 projection;
    // mat4 view;
    // mat4 inv_projection;
    // mat4 inv_view;

    // float vertical_fov;
    // float near_clip;
    // float far_clip;

    sx_vec3 position;
    // vec3 forward;

    uint32_t viewport_width;
    uint32_t viewport_height;

    Ray rays[IMAGE_WIDTH * IMAGE_HEIGHT];
} Camera;

void calculate_projection(Camera *camera);
void calculate_view(Camera *camera);
void calculate_rays(Camera *camera);

void camera_init(Camera *camera)
{
    // camera->vertical_fov = 45.0f;
    // camera->near_clip = 0.1f;
    // camera->far_clip = 100.0f;

    // glm_vec3_copy((vec3) { 0.0f, 0.0f, -1.0f }, camera->forward);
    // glm_vec3_copy((vec3) { 0.0f, 0.0f, 3.0f}, camera->position);
}

void camera_update(Camera *camera, uint32_t width, uint32_t height)
{
    if (width == camera->viewport_width && height == camera->viewport_height)
    {
        return;
    }

    camera->viewport_width = width;
    camera->viewport_height = height;

    calculate_projection(camera);
    calculate_view(camera);
    calculate_rays(camera);
}

void calculate_projection(Camera *camera)
{
    // float aspect_ratio = camera->viewport_width / camera->viewport_height;
    // glm_perspective(camera->vertical_fov, aspect_ratio, camera->near_clip, camera->far_clip, camera->projection);
    // glm_mat4_inv(camera->projection, camera->inv_projection);
}

void calculate_view(Camera *camera)
{
    // vec3 center;
    // glm_vec3_add(camera->position, camera->forward, center);
    // glm_lookat(camera->position, center, (vec3) { 0.0f, 1.0f, 0.0f }, camera->view);
    // glm_mat4_inv(camera->view, camera->inv_view);
}

void calculate_rays(Camera *camera)
{
    // for (uint32_t y = 0; y < camera->viewport_height; y++)
    // {
    //     for (uint32_t x = 0; x < camera->viewport_width; x++)
    //     {
    //         float x_coord = (float) x / camera->viewport_width * 2.0f - 1.0f;
    //         float y_coord = (float) y / camera->viewport_height * 2.0f - 1.0f;

    //         vec3 _target;
    //         vec4 target;
    //         glm_mat4_mulv(camera->inv_projection, (vec4) { x_coord, y_coord, 1.0f, 1.0f }, target);
    //         glm_vec3(target, _target);

    //         vec4 _direction;
    //         vec3 direction;
    //         glm_vec3_divs(_target, target[3], _target);
    //         glm_vec3_normalize(_target);
    //         glm_vec4(_target, 0.0f, _direction);
    //         glm_mat4_mulv(camera->inv_view, _direction, _direction);
    //         glm_vec3(_direction, direction);

    //         glm_vec3_copy(direction, camera->rays[x + y * camera->viewport_width].direction);
    //     }
    // }
}

#endif