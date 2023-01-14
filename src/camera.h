#ifndef LAVS_CAMERA_H_
#define LAVS_CAMERA_H_

#include "math.h"
#include "sokol_app.h"
#include "sx/math-vec.h"
#include "config.h"
#include "input.h"
#include "ray.h"

typedef struct 
{
    sx_mat4 projection;
    sx_mat4 view;
    sx_mat4 inv_projection;
    sx_mat4 inv_view;

    float vertical_fov;
    float near_clip;
    float far_clip;

    sx_vec3 *center;
    sx_vec3 position;
    sx_vec3 forward;
    sx_vec3 up;

    float distance;
    float latitude;
    float longitude;

    uint32_t viewport_width;
    uint32_t viewport_height;

    sx_vec3 ray_directions[IMAGE_WIDTH * IMAGE_HEIGHT];
} Camera;

void camera_calculate(Camera *camera);
void _calculate_position(Camera *camera);
void _calculate_view(Camera *camera);
void _calculate_projection(Camera *camera);
void _calculate_rays(Camera *camera);

void camera_init(Camera *camera, uint32_t width, uint32_t height)
{
    camera->vertical_fov = 45.0f;
    camera->near_clip = 0.1f;
    camera->far_clip = 100.0f;
    camera->center = &SX_VEC3_ZERO;
    camera->position = SX_VEC3_ZERO;
    camera->forward = sx_vec3f(0.0f, 0.0f, -1.0f);
    camera->up = sx_vec3f(0.0f, 1.0f, 0.0f);
    camera->distance = 5.0f;
    camera->latitude = 0.0f;
    camera->longitude = 0.0f;
    camera->viewport_width = width;
    camera->viewport_height = height;

    _calculate_projection(camera);
    camera_calculate(camera);
}

void camera_calculate(Camera *camera)
{
    _calculate_position(camera);
    _calculate_view(camera);
    _calculate_rays(camera);
}

sx_vec3 _camera_euclidean(float latitude, float longitude)
{
    float lat_rad = sx_torad(latitude);
    float lon_rad = sx_torad(longitude);
    return sx_vec3f(cosf(lat_rad) * sinf(lon_rad), sinf(lat_rad), cosf(lat_rad) * cosf(lon_rad));
}

void camera_orbit(Camera *camera, float delta_x, float delta_y)
{
    camera->longitude -= delta_x;
    if (camera->longitude < 0.0f)
    {
        camera->longitude += 360.0f;
    }
    if (camera->longitude > 360.0f)
    {
        camera->longitude -= 360.0f;
    }
    // TODO: set min and max latitude
    camera->latitude = sx_clamp(-85.0f, camera->latitude + delta_y, 85.0f);
}

void camera_zoom(Camera *camera, float delta)
{
    // TODO: set min and max distance
    camera->distance = sx_clamp(2.0f, camera->distance + delta, 10.0f);
}

void _calculate_position(Camera *camera)
{
    camera->position = sx_vec3_add(*camera->center, 
        sx_vec3_mulf(_camera_euclidean(camera->latitude, camera->longitude), camera->distance));
}

void _calculate_projection(Camera *camera)
{
    float aspect_ratio = camera->viewport_width / camera->viewport_height;
    camera->projection = sx_mat4_perspectiveFOV(sx_torad(camera->vertical_fov), 
        aspect_ratio, camera->near_clip, camera->far_clip, false);
    camera->inv_projection = sx_mat4_inv(&camera->projection);
}

void _calculate_view(Camera *camera)
{
    camera->view = sx_mat4_view_lookat(camera->position, *camera->center, camera->up);
    camera->inv_view = sx_mat4_inv(&camera->view);
}

void _calculate_rays(Camera *camera)
{
    for (uint32_t y = 0; y < camera->viewport_height; y++)
    {
        for (uint32_t x = 0; x < camera->viewport_width; x++)
        {
            float x_coord = (float) x / camera->viewport_width * 2.0f - 1.0f;
            float y_coord = (float) y / camera->viewport_height * 2.0f - 1.0f;

            sx_vec4 target = sx_mat4_mul_vec4(&camera->inv_projection, sx_vec4f(x_coord, y_coord, 1.0f, 1.0f));
            sx_vec3 a = sx_vec3_norm(sx_vec3_mulf(sx_vec3f(target.x, target.y, target.z), 1.0f / target.w));
            sx_vec4 ray_direction = sx_mat4_mul_vec4(&camera->inv_view, sx_vec4v3(a, 0.0f));
            camera->ray_directions[x + y * camera->viewport_width] = sx_vec3f(
                ray_direction.x, ray_direction.y, ray_direction.z);
        }
    }
}

bool camera_update(Camera *camera, float dt)
{
    UNUSED(dt);
    bool moved = true;
    const sapp_event *e = Input.event;
    if (e != NULL)
    {
        switch (e->type)
        {
            case SAPP_EVENTTYPE_MOUSE_SCROLL:
            {
                camera_zoom(camera, e->scroll_y * -1.0f);
                moved = true;
                break;
            }
            case SAPP_EVENTTYPE_MOUSE_MOVE:
            {
                if (sapp_mouse_locked())
                {
                    camera_orbit(camera, e->mouse_dx * 1.0f, e->mouse_dy * 1.0f);
                    moved = true;
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }

    if (moved)
    {
        camera_calculate(camera);
    }

    Input.event = NULL;

    return moved;
}

#endif