#ifndef LAVS_RENDER_H_
#define LAVS_RENDER_H_

#include "stdlib.h"
#include "sokol_gfx.h"
#include "sx/sx/math.h"
#include "camera.h"

sx_color per_pixel(Ray *ray, float x, float y);

static uint32_t convert_to_RGBA(sx_color color)
{
    // uint8_t r = (uint8_t) (color.r * 255.0f);
    // uint8_t g = (uint8_t) (color.g * 255.0f);
    // uint8_t b = (uint8_t) (color.b * 255.0f);
    // uint8_t a = (uint8_t) (color.a * 255.0f);

    return (color.a << 24) | (color.b << 16) | (color.g << 8) | color.r;
}

void render(uint32_t *pixels, Camera *camera)
{
    Ray ray;
    // ray.origin = camera->position;

    int width = camera->viewport_width;
    int height = camera->viewport_height;
    for (int y = 0; y < height; y++) 
    {
        for (int x = 0; x < width; x++) 
        {
            // ray.direction = camera->rays[x + y * width].direction;

            float x_coord = (float) x / camera->viewport_width * 2.0f - 1.0f;
            float y_coord = (float) y / camera->viewport_height * 2.0f - 1.0f;

            sx_color color = per_pixel(&ray, x_coord, y_coord);
            sx_clamp(color.r, 0.0f, 1.0f);
            sx_clamp(color.g, 0.0f, 1.0f);
            sx_clamp(color.b, 0.0f, 1.0f);
            sx_clamp(color.a, 0.0f, 1.0f);
            pixels[x + y * width] = convert_to_RGBA(color);
        }
    }
}

sx_color per_pixel(Ray *ray, float x, float y)
{
    ray->origin = sx_vec3f(0.0f, 0.0f, 1.0f);
    ray->direction = sx_vec3f(x, y, -1.0f);
    float radius = 0.5f;
    
    float a = sx_vec3_dot(ray->direction, ray->direction);
    float b = 2.0f * sx_vec3_dot(ray->origin, ray->direction);
    float c = sx_vec3_dot(ray->origin, ray->origin) - radius * radius;

    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f)
    {
        return SX_COLOR_BLACK;
    }

    float t0 = (-b - sx_sqrt(discriminant)) / (2.0f * a);
    float t1 = (-b + sx_sqrt(discriminant)) / (2.0f * a); // not used

    sx_vec3 hit_point = sx_vec3_add(ray->origin, sx_vec3_mulf(ray->direction, t0));
    sx_vec3 normal = sx_vec3_norm(hit_point);

    sx_vec3 light_direction = sx_vec3_norm(sx_vec3f(-1.0f, -1.0f, -1.0f));
    float light_intensity = sx_max(0.0f, sx_vec3_dot(normal, sx_vec3_neg(light_direction)));

    sx_vec3 sphere_color = sx_vec3_mulf(sx_vec3f(1.0f, 0.0f, 1.0f), light_intensity);
    return sx_color4f(sphere_color.x, sphere_color.y, sphere_color.z, 1.0f);
}

#endif