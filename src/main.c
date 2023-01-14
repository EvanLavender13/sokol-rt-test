#include "omp.h"
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_gl.h"
#include "sokol_glue.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui/cimgui.h"
#include "sokol_imgui.h"
#include "sokol_time.h"
#include "sx/math-vec.h"
#include "camera.h"
#include "config.h"
#include "input.h"
#include "render.h"
#include "scene.h"
#include "sim.h"

static struct {
    sg_pass_action pass_action;
    sg_image image;

    uint32_t pixels[IMAGE_WIDTH][IMAGE_HEIGHT];
    uint64_t last_time;
    double last_render_time;

    Camera camera;
    Scene scene;
    // SimData sim_data;
} State;

void frame_gui();
void frame_update();

void init() 
{
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext()
    });
    sgl_setup(&(sgl_desc_t) { 0 });
    simgui_setup(&(simgui_desc_t) { 0 });
    stm_setup();

    State.image = sg_make_image(&(sg_image_desc)
    {
        .width = IMAGE_WIDTH,
        .height = IMAGE_HEIGHT,
        .usage = SG_USAGE_STREAM,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .label = "image-texture"
    });

    // default pass action
    State.pass_action = (sg_pass_action) 
    {
        .colors[0] = 
        { 
            .action = SG_ACTION_CLEAR, 
            .value = { 0.5f, 0.5f, 0.5f, 1.0f } 
        }
    };

    Material pink_sphere;
    pink_sphere.albedo = sx_vec3f(1.0f, 0.0f, 1.0f);
    pink_sphere.roughness = 0.0f;
    State.scene.materials[0] = pink_sphere;

    Material blue_sphere;
    blue_sphere.albedo = sx_vec3f(0.2f, 0.3f, 1.0f);
    blue_sphere.roughness = 0.01f;
    State.scene.materials[1] = blue_sphere;

    {
        Sphere sphere;
        sphere.position = sx_vec3f(0.0f, 10.0f, 0.0f);
        sphere.velocity = sx_vec3f(0.0f, 0.0f, 0.0f);
        sphere.radius = 1.0f;
        sphere.material_index = 0;
        State.scene.spheres[0] = sphere;
    }

    {
        Sphere sphere;
        sphere.position = sx_vec3f(0.0f, -101.0f, 0.0f);
        sphere.velocity = sx_vec3f(0.0f, 0.0f, 0.0f);
        sphere.radius = 100.0f;
        sphere.material_index = 1;
        State.scene.spheres[1] = sphere;
    }

    // State.sim_data.num_objects = 1;
    // State.sim_data.positions = malloc(1 * sizeof(sx_vec3)); // freed in sim_shutdown()
    // State.sim_data.positions[0] = &State.scene.spheres[0].position;

    // State.sim_data.velocities = malloc(1 * sizeof(sx_vec3));
    // State.sim_data.velocities[0] = &State.scene.spheres[0].velocity;
    render_reset();
    camera_init(&State.camera, IMAGE_WIDTH, IMAGE_HEIGHT);
    State.camera.center = &State.scene.spheres[0].position;
    sim_init(&State.scene);
}

void frame() 
{
    if (camera_update(&State.camera, sapp_frame_duration()))
    {
        render_reset();
    }
    frame_update();

    // draw texture quad
    sgl_viewport(0, 0, sapp_width(), sapp_height(), false);
    sgl_defaults();
    sgl_enable_texture();
    sgl_texture(State.image);
    sgl_begin_quads();
    sgl_v2f_t2f( -1.0f,  1.0f,  0.0, 1.0);
    sgl_v2f_t2f(  1.0f,  1.0f,  1.0, 1.0);
    sgl_v2f_t2f(  1.0f, -1.0f,  1.0, 0.0);
    sgl_v2f_t2f( -1.0f, -1.0f,  0.0, 0.0);    
    sgl_end();

    sg_begin_default_pass(&State.pass_action, sapp_width(), sapp_height());
    sgl_draw();
    frame_gui(); 
    sg_end_pass();
    sg_commit();
}

void frame_update()
{
    sim_update(sapp_frame_duration());
    // render_reset();
    render(State.pixels[0], &State.scene, &State.camera);
    sg_update_image(State.image, &(sg_image_data)
    {
        .subimage[0][0] = SG_RANGE(State.pixels)
    });

    uint64_t frame_time = stm_laptime(&State.last_time);
    State.last_render_time = stm_ms(frame_time);
}

void frame_gui()
{
    simgui_new_frame(&(simgui_frame_desc_t)
    {
        .width = sapp_width(),
        .height = sapp_height(),
        .delta_time = sapp_frame_duration(),
        .dpi_scale = sapp_dpi_scale()
    });
        
    igSetNextWindowSize((ImVec2) { 250, 300 }, ImGuiCond_FirstUseEver);
    igBegin("Settings", NULL, 0);
    igText("Last render: %.3fms", State.last_render_time);
    if (igButton("Reset", (ImVec2) { 0.0f, 0.0f }))
    {
        render_reset();
    }

    igText("Scene");
    for (int i = 0; i < NUM_MATERIALS; i++)
    {
        igPushID_Int(i);
        Material *material = &State.scene.materials[i];
        igColorEdit3("Albedo", (float*) &material->albedo, ImGuiColorEditFlags_None);
        igDragFloat("Roughness", &material->roughness, 0.01f, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_None);
        igSeparator();
        igPopID();
    }
    for (int i = 0; i < NUM_SPHERES; i++) 
    {
        igPushID_Int(i);
        Sphere *sphere = &State.scene.spheres[i];
        igDragFloat3("Position", (float*) &sphere->position, 0.01f, -10.0f, 10.0f, "%.3f", ImGuiSliderFlags_None);
        igDragFloat("Radius", &sphere->radius, 0.01f, 0.1f, 5.0f, "%.3f", ImGuiSliderFlags_None);
        if (igButton("Center", (ImVec2) { 0.0f, 0.0f }))
        {
            State.camera.center = &sphere->position;
            camera_calculate(&State.camera);
            render_reset();
        }
        igSeparator();
        igPopID();
    }
    igEnd();

    simgui_render();
}

void cleanup() 
{
    sim_shutdown();
    simgui_shutdown();
    sgl_shutdown();
    sg_shutdown();
}

void event(const sapp_event *e) 
{
    if (simgui_handle_event(e))
    {
        sapp_lock_mouse(false);
        return;
    }

    switch (e->type)
    {
        case SAPP_EVENTTYPE_MOUSE_DOWN:
        {
            if (e->mouse_button == SAPP_MOUSEBUTTON_LEFT)
            {
                sapp_lock_mouse(true);
            }
            break;
        }
        case SAPP_EVENTTYPE_MOUSE_UP:
        {
            if (e->mouse_button == SAPP_MOUSEBUTTON_LEFT)
            {
                sapp_lock_mouse(false);
            }
            break;
        }
        default:
        {
            break;
        }
    }

    Input.event = e;
}

sapp_desc sokol_main(int argc, char *argv[]) 
{
    UNUSED(argc);
    UNUSED(argv);
    return (sapp_desc) 
    {
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .width = 1200,
        .height = 800,
        .window_title = "sokol-rt-test app"
    };
}