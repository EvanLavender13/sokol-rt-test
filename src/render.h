#ifndef LAVS_RENDER_H_
#define LAVS_RENDER_H_

#include "omp.h"
#include "stdlib.h"
#include "sokol_gfx.h"
#include "sx/math-vec.h"
#include "camera.h"
#include "config.h"
#include "rng.h"
#include "scene.h"

struct
{
    Scene *active_scene;
    Camera *active_camera;

    sx_vec4 accumulated[IMAGE_WIDTH * IMAGE_HEIGHT];
    uint32_t image_width;
    uint32_t image_height;

    uint32_t frame_index;
} Renderer;

typedef struct
{
    float distance;
    sx_vec3 world_position;
    sx_vec3 world_normal;
    int index;
} HitPayload;

sx_vec4 per_pixel(uint32_t x, uint32_t y);
HitPayload trace_ray(Ray *ray);
HitPayload _closest_hit(Ray *ray, float hit_distance, int index);
HitPayload _miss(Ray *ray);

static uint32_t convert_to_RGBA(sx_vec4 color)
{
    sx_color rgba = sx_color4f(color.x, color.y, color.z, color.w);
    return (rgba.a << 24) | (rgba.b << 16) | (rgba.g << 8) | rgba.r;
}

static void clamp_color(sx_vec4 color)
{
    sx_clamp(color.x, 0.0f, 1.0f);
    sx_clamp(color.y, 0.0f, 1.0f);
    sx_clamp(color.z, 0.0f, 1.0f);
    sx_clamp(color.w, 0.0f, 1.0f);
}

static sx_vec3 reflect(sx_vec3 direction, sx_vec3 normal)
{
    sx_vec3 reflected_ray = sx_vec3_mulf(normal, sx_vec3_dot(normal, direction) * 2.0f);
    reflected_ray = sx_vec3_sub(direction, reflected_ray);
    return reflected_ray;
}

void render_reset()
{
    Renderer.frame_index = 1;
}

void render(uint32_t *pixels, Scene *scene, Camera *camera)
{
    Renderer.active_scene = scene;
    Renderer.active_camera = camera;
    Renderer.image_width = camera->viewport_width;
    Renderer.image_height = camera->viewport_height;

    if (Renderer.frame_index == 1)
    {
        sx_memset(Renderer.accumulated, 0, Renderer.image_width * Renderer.image_height * sizeof(sx_vec4));
    }

    uint32_t x, y;
    // TODO: figure our openmp
    // #pragma omp parallel for private(x)
    for (y = 0; y < Renderer.image_height; y++) 
    {   
        // #pragma omp parallel for
        for (x = 0; x < Renderer.image_width; x++) 
        {
            sx_vec4 color = per_pixel(x, y);
            uint32_t index = x + y * Renderer.image_width;
            Renderer.accumulated[index] = sx_vec4_add(Renderer.accumulated[index], color);

            sx_vec4 accumulated_color = Renderer.accumulated[index];
            accumulated_color = sx_vec4_mulf(accumulated_color, 1.0f / (float) Renderer.frame_index);

            clamp_color(accumulated_color);
            pixels[index] = convert_to_RGBA(accumulated_color);
        }
    }

    Renderer.frame_index++;
}

sx_vec4 per_pixel(uint32_t x, uint32_t y)
{
    Ray ray;
    ray.origin = Renderer.active_camera->position;
    ray.direction = Renderer.active_camera->ray_directions[x + y * Renderer.image_width];

    sx_vec3 color = SX_VEC3_ZERO;
    float multiplier = 1.0f;

    int bounces = 5;
    for (int i = 0; i < bounces; i++)
    {
        HitPayload payload = trace_ray(&ray);
        if (payload.distance < 0.0f)
        {
            sx_vec3 sky_color = sx_vec3f(0.6f, 0.7f, 0.9f);
            // sx_vec3 sky_color = sx_vec3f(0.0f, 0.0f, 0.0f);
            // color = sx_vec3_add(color, sx_vec3_mulf(sky_color, multiplier));
            color = sx_vec3_add(color, sky_color);
            color = sx_vec3_mulf(color, multiplier);
            break;
        }
        
        sx_vec3 light_direction = sx_vec3_norm(sx_vec3f(-1.0f, -1.0f, -1.0f));
        float light_intensity = sx_max(0.0f, sx_vec3_dot(payload.world_normal, sx_vec3_neg(light_direction)));

        Sphere *sphere = &Renderer.active_scene->spheres[payload.index];
        Material *material = &Renderer.active_scene->materials[sphere->material_index];
        sx_vec3 sphere_color = sx_vec3_mulf(material->albedo, light_intensity);
        // color = sx_vec3_add(color, sx_vec3_mulf(sphere_color, multiplier));
        color = sx_vec3_add(color, sphere_color);
        color = sx_vec3_mulf(color, multiplier);

        multiplier *= 0.5f;

        ray.origin = sx_vec3_add(payload.world_position, sx_vec3_mulf(payload.world_normal, 0.001f));
        sx_vec3 scatter = sx_vec3_mulf(random_vec3(-0.5f, 0.5f), material->roughness);
        ray.direction = reflect(ray.direction, sx_vec3_add(payload.world_normal, scatter));
    }

    return sx_vec4v3(color, 1.0f);
}

HitPayload trace_ray(Ray *ray)
{
    int closest_sphere = -1;
    float hit_distance = SX_FLOAT_MAX;
    for (int i = 0; i < NUM_SPHERES; i++)
    {
        Sphere *sphere = &Renderer.active_scene->spheres[i];
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
        if (t0 > 0.0f && t0 < hit_distance)
        {
            hit_distance = t0;
            closest_sphere = i;
        }
    }

    if (closest_sphere < 0)
    {
        return _miss(ray);
    }

    return _closest_hit(ray, hit_distance, closest_sphere);
}

HitPayload _closest_hit(Ray *ray, float hit_distance, int index)
{
    HitPayload payload;
    payload.distance = hit_distance;
    payload.index = index;

    Sphere *closest_sphere = &Renderer.active_scene->spheres[index];
    sx_vec3 origin = sx_vec3_sub(ray->origin, closest_sphere->position);
    payload.world_position = sx_vec3_add(origin, sx_vec3_mulf(ray->direction, hit_distance));
    payload.world_normal = sx_vec3_norm(payload.world_position);
    
    payload.world_position = sx_vec3_add(payload.world_position, closest_sphere->position);

    return payload;
}

HitPayload _miss(Ray *ray)
{
    UNUSED(ray);
    HitPayload payload;
    payload.distance = -1.0f;
    return payload;
}

#endif