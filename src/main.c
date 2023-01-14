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

static struct {
    sg_pass_action pass_action;
    sg_image image;

    uint32_t pixels[IMAGE_WIDTH][IMAGE_HEIGHT];
    uint64_t last_time;
    double last_render_time;

    Camera camera;
    Scene scene;
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

    camera_init(&State.camera, IMAGE_WIDTH, IMAGE_HEIGHT);

    {
        Sphere sphere;
        sphere.position = sx_vec3f(0.0f, 0.0f, 0.0f);
        sphere.radius = 0.5f;
        sphere.albedo = sx_vec3f(1.0f, 0.0f, 1.0f);
        State.scene.spheres[0] = sphere;
    }

    {
        Sphere sphere;
        sphere.position = sx_vec3f(1.0f, 0.0f, -5.0f);
        sphere.radius = 1.5f;
        sphere.albedo = sx_vec3f(0.2f, 0.3f, 1.0f);
        State.scene.spheres[1] = sphere;
    }    
}

void frame() 
{
    camera_update(&State.camera, sapp_frame_duration());
    frame_update();

    // draw texture quad
    sgl_viewport(0, 0, sapp_width(), sapp_height(), false);
    sgl_defaults();
    sgl_enable_texture();
    sgl_texture(State.image);
    sgl_begin_quads();
    sgl_v2f_t2f( -1.0f,  1.0f,  0.0, 0.0);
    sgl_v2f_t2f(  1.0f,  1.0f,  1.0, 0.0);
    sgl_v2f_t2f(  1.0f, -1.0f,  1.0, 1.0);
    sgl_v2f_t2f( -1.0f, -1.0f,  0.0, 1.0);     
    sgl_end();

    sg_begin_default_pass(&State.pass_action, sapp_width(), sapp_height());
    sgl_draw();

    frame_gui(); 

    sg_end_pass();
    sg_commit();
}

void frame_update()
{
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
        
    igSetNextWindowSize((ImVec2) { 250, 250 }, ImGuiCond_FirstUseEver);
    igBegin("Settings", NULL, 0);
    igText("Last render: %.3fms", State.last_render_time);
    if (igButton("Render", (ImVec2) { 0.0f, 0.0f }))
    {
        // frame_update();
    }

    igText("Scene");
    for (int i = 0; i < NUM_SPHERES; i++) {
        igPushID_Int(i);
        Sphere *sphere = &State.scene.spheres[i];
        igSliderFloat3("Position", (float*) &sphere->position, -10.0f, 10.0f, "%.3f", ImGuiSliderFlags_None);
        igSliderFloat("Radius", &sphere->radius, 0.1f, 5.0f, "%.3f", ImGuiSliderFlags_None);
        igColorEdit3("Albedo", (float*) &sphere->albedo, ImGuiColorEditFlags_None);
        igSeparator();
        igPopID();
    }
    igEnd();

    simgui_render();
}

void cleanup() 
{
    simgui_shutdown();
    sgl_shutdown();
    sg_shutdown();
}

void event(const sapp_event *e) 
{
    if (simgui_handle_event(e))
    {
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
        .width = IMAGE_WIDTH,
        .height = IMAGE_HEIGHT,
        .window_title = "sokol-rt-test app"
    };
}