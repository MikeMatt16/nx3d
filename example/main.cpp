#include "math3d.hpp"
#include "colors.h"
#include <fstream>
#include <hal/debug.h>
#include <hal/video.h>
#include <math.h>
#include <nx3d.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

/* From `nxdk/samples/mesh` */

#include "mesh_texture.h"

typedef struct Vertex   
{
    float position[3];
    float normal[3];
    float tex_coord[2];
} MeshVertex;

#include "mesh_vertices.h"

typedef struct mesh
{
    NX3D_BUFFER *vb, *i;
    NX3D_VERTEX_ELEMENT_LAYOUT *layout;
} mesh;

static const NX3D_VERTEX_DATA_DESC mesh_elements[] =
{
    { NX3D_POSITION, NX3D_FLOAT32, 3, 0 },
    { NX3D_NORMAL, NX3D_FLOAT32, 3, 12 },
    { NX3D_TEXCOORD0, NX3D_FLOAT32, 2, 24 },
};

static const NX3D_VERTEX_DATA_DESC vertex_elements[] =
{
    { NX3D_POSITION, NX3D_FLOAT32, 3, 0 },
    { NX3D_DIFFUSE, NX3D_UNORM8_BGRA, 4, 12 },
    { NX3D_NORMAL, NX3D_FLOAT32, 3, 16 },
    { NX3D_TEXCOORD0, NX3D_FLOAT32, 2, 28 },
};

static const NX3D_VERTEX foreground[] =
{
    {{-0.9f, -0.7f, 0}, 0xFFFF0000, {0, 0, 1}, {0, 0}},
    {{ 0.9f, -0.7f, 0}, 0xFF00FF00, {0, 0, 1}, {0, 0}},
    {{ 0.0f,  0.9f, 0}, 0xFF0000FF, {0, 0, 1}, {0, 0}}
};

static const uint16_t front_triangles[] =
{
    0, 1, 2,
    2, 1, 0
};

static int width, height;
static bool run_loop = false;
static mesh back_mesh, front_mesh;
static struct
{
    NX3D_SAMPLER_DESC sampler;
    NX3D_TEXTURE *tex;
} texture;

/* Initialize the video mode */
static int init_video(void)
{
    int w, h;
    const DWORD encoder_settings = XVideoGetEncoderSettings();
    if (encoder_settings & VIDEO_MODE_1080I)
    {
        w = (encoder_settings & VIDEO_WIDESCREEN) == 0 ? 1440 : 1920;
        h = 1080;
    }
    else if (encoder_settings & VIDEO_MODE_720P)
    {
        w = (encoder_settings & VIDEO_WIDESCREEN) == 0 ? 960 : 1280;
        h = 720;
    }
    else
    {
        // pbkit doesn't like 720x480 so we just use 640x480
        w = 640;
        h = 480;
    }

    (void)XVideoSetMode(w, h, 32, 0);
    auto vm = XVideoGetMode();
    width = vm.width;
    height = vm.height;
    return 0;
}

/* Initialize nx3d and setup defaults */
static int init_nx3d(void)
{
    if (nx3d_init())
    {
        debugPrint("nx3d_init failed: %x\n", nx3d_get_error());
        Sleep(5000);
        return 1;
    }

    const float fov = DEG2RAD(60.0f);
    const float aspect = (float)width / (float)height;
    const float near_z = 0.1f, far_z = 20.0f;
    mat4 projection = mat4::perspective(fov, aspect, near_z, far_z);
    (void)nx3d_load_matrix(NX3D_PROJECTION, projection.m);
    (void)nx3d_depth_test(TRUE);

    return 0;
}

static int initialize(void)
{
    /* Set front face winding direction and which faces are to be culled */
    if (nx3d_set_winding(NX3D_WINDING_CW) ||
        nx3d_set_cull_face(NX3D_CULL_BACK))
        return 1;

    /* Create element layouts for both the forground and background vertices */
    if (nx3d_create_element_layout(&front_mesh.layout, sizeof(NX3D_VERTEX),
            vertex_elements, sizeof(vertex_elements) / sizeof(vertex_elements[0])) ||
        nx3d_create_element_layout(&back_mesh.layout, sizeof(MeshVertex),
            mesh_elements, sizeof(mesh_elements) / sizeof(mesh_elements[0])))
        return 1;

    /* Create and populate buffers for the front indices, 
            back indices, front vertices, and back vertices */
    if (nx3d_create_buffer(&front_mesh.i, NX3D_INDEX_BUFFER, sizeof(front_triangles)) ||
        nx3d_buffer_data(front_mesh.i, front_triangles, sizeof(front_triangles)))
        return 1;

    if (nx3d_create_buffer(&back_mesh.i, NX3D_INDEX_BUFFER, sizeof(indices)) ||
        nx3d_buffer_data(back_mesh.i, indices, sizeof(indices)))
        return 1;

    if (nx3d_create_buffer(&front_mesh.vb, NX3D_VERTEX_BUFFER, sizeof(foreground)) ||
        nx3d_buffer_data(front_mesh.vb, foreground, sizeof(foreground)))
        return 1;

    if (nx3d_create_buffer(&back_mesh.vb, NX3D_VERTEX_BUFFER, sizeof(vertices)) ||
        nx3d_buffer_data(back_mesh.vb, vertices, sizeof(vertices)))
        return 1;

    /* Texture descriptor used to define a texture to be created */
    NX3D_TEXTURE_DESCRIPTOR tex_desc[1];
    memset(tex_desc, 0, sizeof(tex_desc));
    tex_desc->format = NX3D_TEXTURE_FORMAT_A8R8G8B8;
    tex_desc->type = NX3D_TEXTURE_2D;
    tex_desc->width = texture_width;
    tex_desc->height = texture_height;
    
    /* Create texture for the back mesh and set pixel data */
    if (nx3d_create_textures(&texture.tex, tex_desc, 1) ||
        nx3d_set_texture_pixel_data(texture.tex, texture_rgba, texture_pitch * texture_height))
        return nx3d_get_error();

    /* Setup the texture sampler */
    NX3D_SAMPLER_DESC sampler;
    nx3d_sampler_default(&sampler);
    texture.sampler = sampler;

    return 0;
}

static int update(const DWORD start_ticks)
{
    /* Calculate front and back model matrices */
    const float angle = (GetTickCount() - start_ticks) * 0.001f;
    const mat4 front_view = mat4::rotation_y(angle) * 
        mat4::translation(vec3(0, 0, -3));
    const mat4 mesh_scale = mat4::scale(vec3(0.02f, 0.02f, 0.02f));
    const mat4 back_view = mesh_scale * mat4::rotation_x(angle * 0.5f) *
        mat4::translation(vec3(0, 0, -4.5f));

    /* Clear color, depth, and stencil */
    uint32_t clear_bits = NX3D_CLEAR_COLOR | NX3D_CLEAR_DEPTH | NX3D_CLEAR_STENCIL;
    if (nx3d_begin_frame() ||
        nx3d_clear(clear_bits, Color_CornflowerBlue, 1.0f, 0))
        return 1;

    /* Load the front view matrix, bind front vertices and
            indices and draw the front mesh */
    if (nx3d_load_matrix(NX3D_MODELVIEW, front_view.m) ||
        nx3d_bind_vertex_buffer(front_mesh.vb, front_mesh.layout) ||
        nx3d_bind_index_buffer(front_mesh.i, NX3D_UINT16) ||
        nx3d_bind_texture(NULL, NULL, NX3D_TEXTURE_SLOT_0) ||
        nx3d_enable(NX3D_ENABLE_VERTEX_COLOR, TRUE) ||
        nx3d_draw_indexed_primitives(NX3D_TRIANGLES, 0, sizeof(front_triangles) / sizeof(front_triangles[0])))
        return 1;

    /* Load the back view matrix, bind back vertices and
            indices and draw the back mesh */
    if (nx3d_load_matrix(NX3D_MODELVIEW, back_view.m) ||
        nx3d_bind_vertex_buffer(back_mesh.vb, back_mesh.layout) ||
        nx3d_bind_index_buffer(back_mesh.i, NX3D_UINT16) ||
        nx3d_bind_texture(texture.tex, &texture.sampler, NX3D_TEXTURE_SLOT_0) ||
        nx3d_enable(NX3D_ENABLE_VERTEX_COLOR, FALSE) ||
        nx3d_draw_indexed_primitives(NX3D_TRIANGLES, 0, sizeof(indices) / (sizeof(indices[0]) / 2)))
        return 1;

    (void)nx3d_end_frame();

    return 0;
}

static void cleanup()
{
    /* Call all corresponding release functions */
    (void)nx3d_release_textures(texture.tex, 1);
    (void)nx3d_release_buffer(front_mesh.vb);
    (void)nx3d_release_buffer(back_mesh.vb);
    (void)nx3d_release_buffer(front_mesh.i);
    (void)nx3d_release_buffer(back_mesh.i);
    (void)nx3d_release_element_layout(front_mesh.layout);
    (void)nx3d_release_element_layout(back_mesh.layout);
    (void)nx3d_shutdown();
}

int main(void)
{
    int status;
    if ((status = init_video())) return status;
    if ((status = init_nx3d())) return status;
    if ((status = initialize())) return status;

    const DWORD start = GetTickCount();
    run_loop = true;
    while (run_loop)
        if ((status = update(start)))
            break;

    const int error = nx3d_get_error();
    cleanup();

    debugPrint("nx3d render failed: %x\n", error);
    Sleep(5000);
    return 1;
}
