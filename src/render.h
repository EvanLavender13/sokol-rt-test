#ifndef LAVS_RENDER_H_
#define LAVS_RENDER_H_

#include "stdlib.h"
#include "sokol_gfx.h"
#include "sx/sx/math.h"
#include "camera.h"
#include "scene.h"

sx_color trace_ray(Scene *scene, Ray *ray);

static uint32_t convert_to_RGBA(sx_color color)
{
    // uint8_t r = (uint8_t) (color.r * 255.0f);
    // uint8_t g = (uint8_t) (color.g * 255.0f);
    // uint8_t b = (uint8_t) (color.b * 255.0f);
    // uint8_t a = (uint8_t) (color.a * 255.0f);

    return (color.a << 24) | (color.b << 16) | (color.g << 8) | color.r;
}

static void clamp_color(sx_color color)
{
    sx_clamp(color.r, 0.0f, 1.0f);
    sx_clamp(color.g, 0.0f, 1.0f);
    sx_clamp(color.b, 0.0f, 1.0f);
    sx_clamp(color.a, 0.0f, 1.0f);
}

void render(uint32_t *pixels, Scene *scene, Camera *camera)
{
    Ray ray;
    ray.origin = camera->position;

    int width = camera->viewport_width;
    int height = camera->viewport_height;
    for (int y = 0; y < height; y++) 
    {
        for (int x = 0; x < width; x++) 
        {
            ray.direction = camera->ray_directions[x + y * width];
            sx_color color = trace_ray(scene, &ray);
            clamp_color(color);
            pixels[x + y * width] = convert_to_RGBA(color);
        }
    }
}

sx_color trace_ray(Scene *scene, Ray *ray)
{
    Sphere *closest_sphere = NULL;
    float hit_distance = SX_FLOAT_MAX;

    for (int i = 0; i < NUM_SPHERES; i++)
    {
        Sphere *sphere = &scene->spheres[i];
        sx_vec3 origin = sx_vec3_sub(ray->origin, sphere->position);

        float a = sx_vec3_dot(ray->direction, ray->direction);
        float b = 2.0f * sx_vec3_dot(origin, ray->direction);
        float c = sx_vec3_dot(origin, origin) - sphere->radius * sphere->radius;

        float discriminant = b * b - 4.0f * a * c;
        if (discriminant < 0.0f)
        {
            // return SX_COLOR_BLACK;
            continue;
        }

        float t0 = (-b - sx_sqrt(discriminant)) / (2.0f * a);
        // float t1 = (-b + sx_sqrt(discriminant)) / (2.0f * a); // not used
        if (t0 < hit_distance)
        {
            hit_distance = t0;
            closest_sphere = sphere;
        }
    }

    if (closest_sphere == NULL)
    {
        return SX_COLOR_BLACK;
    }

    sx_vec3 origin = sx_vec3_sub(ray->origin, closest_sphere->position);
    sx_vec3 hit_point = sx_vec3_add(origin, sx_vec3_mulf(ray->direction, hit_distance));
    sx_vec3 normal = sx_vec3_norm(hit_point);
    
    sx_vec3 light_direction = sx_vec3_norm(sx_vec3f(-1.0f, 1.0f, 0.0f));
    float light_intensity = sx_max(0.0f, sx_vec3_dot(normal, sx_vec3_neg(light_direction)));

    sx_vec3 sphere_color = sx_vec3_mulf(closest_sphere->albedo, light_intensity);
    return sx_color4f(sphere_color.x, sphere_color.y, sphere_color.z, 1.0f);
}

#endif