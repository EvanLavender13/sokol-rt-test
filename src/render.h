#ifndef LAVS_RENDER_H_
#define LAVS_RENDER_H_

#include "stdlib.h"

#include "sokol_gfx.h"

#include "HandmadeMath.h"

hmm_vec4 per_pixel(float x, float y);

static uint32_t convert_to_RGBA(hmm_vec4 *color)
{
    uint8_t r = (uint8_t) (color->R * 255.0f);
    uint8_t g = (uint8_t) (color->G * 255.0f);
    uint8_t b = (uint8_t) (color->B * 255.0f);
    uint8_t a = (uint8_t) (color->A * 255.0f);
    return (a << 24) | (b << 16) | (g << 8) | r;
}

void render(uint32_t *pixels, int width, int height)
{
    for (int y = 0; y < height; y++) 
    {
        for (int x = 0; x < width; x++) 
        {
            float x_coord = (float) x / (float) width * 2.0f - 1.0f;
            float y_coord = (float) y / (float) height * 2.0f - 1.0f;
            // uint32_t r = rand() & 0xFF;
            // r |= (rand() & 0xFF) << 8;
            // r |= (rand() & 0xFF) << 16;
            // r |= (rand() & 0xFF) << 24;
            // pixels[y * height + x] = per_pixel(x_coord, y_coord);
            hmm_vec4 color = per_pixel(x_coord, y_coord);
            color.R = HMM_Clamp(0.0f, color.R, 1.0f);
            color.G = HMM_Clamp(0.0f, color.G, 1.0f);
            color.B = HMM_Clamp(0.0f, color.B, 1.0f);
            pixels[x + y * width] = convert_to_RGBA(&color);
        }
    }
}

hmm_vec4 per_pixel(float x, float y)
{
    hmm_vec3 ray_origin = HMM_Vec3(0.0f, 0.0f, 1.0f);
    hmm_vec3 ray_direction = HMM_Vec3(x, y, -1.0f);
    float radius = 0.5f;

    float a = HMM_DotVec3(ray_direction, ray_direction);
    float b = 2.0f * HMM_DotVec3(ray_origin, ray_direction);
    float c = HMM_DotVec3(ray_origin, ray_origin) - radius * radius;

    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f)
    {
        return HMM_Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    float t0 = (-b - HMM_SQRTF(discriminant)) / (2.0f * a);
    float t1 = (-b + HMM_SQRTF(discriminant)) / (2.0f * a); // not used

    hmm_vec3 hit_point = HMM_MultiplyVec3f(HMM_AddVec3(ray_origin, ray_direction), t0);
    // hmm_vec3 normal = HMM_NormalizeVec3(hit_point);

    // hmm_vec3 light_direction = HMM_NormalizeVec3(HMM_Vec3(-1.0f, -1.0f, -1.0f));
    // float light_intensity = HMM_MAX(HMM_DotVec3(normal, HMM_MultiplyVec3f(light_direction, -1.0f)), 0.0f);

    // hmm_vec3 sphere_color = HMM_MultiplyVec3f(HMM_Vec3(1.0f, 0.0f, 1.0f), light_intensity);
    return HMM_Vec4v(hit_point, 1.0f);
}

#endif