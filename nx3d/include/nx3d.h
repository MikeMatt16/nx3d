#ifndef NX3D_H
#define NX3D_H

#include "nx3d_error.h"
#include "nx3d_types.h"
#include <stdint.h>

typedef enum NX3D_INDEX_DATA_TYPE
{
    NX3D_UINT16,
    NX3D_UINT32
} NX3D_INDEX_DATA_TYPE;

typedef enum NX3D_VERTEX_DATA_TYPE
{
    NX3D_FLOAT32,
    NX3D_UNORM8_BGRA,
    NX3D_SNORM16,
    NX3D_UNORM8_RGBA,
    NX3D_SINT16,
    NX3D_SNORM_11_11_10,
} NX3D_VERTEX_DATA_TYPE;

typedef enum NX3D_VERTEX_SEMANTICS
{
    NX3D_POSITION,
    NX3D_BLENDWEIGHT,
    NX3D_NORMAL,
    NX3D_DIFFUSE,
    NX3D_SPECULAR,
    NX3D_FOGCOORD,
    NX3D_PSIZE,
    NX3D_BACK_DIFFUSE,
    NX3D_BACK_SPECULAR,
    NX3D_TEXCOORD0,
    NX3D_TEXCOORD1,
    NX3D_TEXCOORD2,
    NX3D_TEXCOORD3,
} NX3D_VERTEX_SEMANTICS;

typedef struct NX3D_VERTEX_DATA_DESC
{
    NX3D_VERTEX_SEMANTICS semantic_index;
    NX3D_VERTEX_DATA_TYPE type;
    int count;
    int aligned_offset;
} NX3D_VERTEX_DATA_DESC;

typedef enum NX3D_BUFFER_TYPE
{
    NX3D_VERTEX_BUFFER,
    NX3D_INDEX_BUFFER,
} NX3D_BUFFER_TYPE;

typedef enum NX3D_PRIMITIVE_TYPE
{
    NX3D_POINTS,
    NX3D_LINE,
    NX3D_LINE_LOOP,
    NX3D_LINE_STRIP,
    NX3D_TRIANGLES,
    NX3D_TRIANGLE_STRIP,
    NX3D_TRIANGLE_FAN,
    NX3D_QUADS,
    NX3D_QUADS_STRIP,
    NX3D_POLYGON
} NX3D_PRIMITIVE_TYPE;

typedef enum NX3D_SAMPLER_CONVOLUTION_KERNEL
{
    NX3D_SAMPLER_KERNEL_QUINCUNX = 1,
    NX3D_SAMPLER_KERNEL_GAUSSIAN = 2
} NX3D_SAMPLER_CONVOLUTION_KERNEL;

typedef enum NX3D_SAMPLER_WRAP_MODE
{
    NX3D_SAMPLER_WRAP_REPEAT = 1,
    NX3D_SAMPLER_WRAP_MIRRORED,
    NX3D_SAMPLER_WRAP_CLAMP,
    NX3D_SAMPLER_WRAP_CLAMP_BORDER,
    NX3D_SAMPLER_WRAP_CLAMP_EDGE,
} NX3D_SAMPLER_WRAP_MODE;

typedef enum NX3D_SAMPLER_MIN_FILTER
{
    NX3D_SAMPLER_MIN_BOX_LOD0 = 1,
    NX3D_SAMPLER_MIN_TENT_LOD0,
    NX3D_SAMPLER_MIN_BOX_NEAREST_LOD,
    NX3D_SAMPLER_MIN_TENT_NEAREST_LOD,
    NX3D_SAMPLER_MIN_BOX_TENT_LOD,
    NX3D_SAMPLER_MIN_TENT_TENT_LOD,
    NX3D_SAMPLER_MIN_CONVOLUTION_2D_LOD0
} NX3D_SAMPLER_MIN_FILTER;

typedef enum NX3D_SAMPLER_MAG_FILTER
{
    NX3D_SAMPLER_MAG_BOX_LOD0 = 1,
    NX3D_SAMPLER_MAG_TENT_LOD0,
    NX3D_SAMPLER_MAG_CONVOLUTION_2D_LOD0 = 4
} NX3D_SAMPLER_MAG_FILTER;

typedef enum NX3D_SAMPLER_ANISOTROPY
{
    NX3D_SAMPLER_ANISOTROPY_1X,
    NX3D_SAMPLER_ANISOTROPY_2X,
    NX3D_SAMPLER_ANISOTROPY_4X,
    NX3D_SAMPLER_ANISOTROPY_8X
} NX3D_SAMPLER_ANISOTROPY;

typedef struct NX3D_SAMPLER_DESC
{
    NX3D_SAMPLER_CONVOLUTION_KERNEL kernel;
    NX3D_SAMPLER_MIN_FILTER min_filter;
    NX3D_SAMPLER_MAG_FILTER mag_filter;
    NX3D_SAMPLER_WRAP_MODE wrap_u, wrap_v, wrap_w;
    NX3D_SAMPLER_ANISOTROPY anisotropy_level;
    uint32_t border_color;
    float lod_bias, min_lod_clamp, max_lod_clamp;
    uint32_t alpha_kill_enable;
} NX3D_SAMPLER_DESC;

typedef enum NX3D_TEXTURE_FORMAT
{
    NX3D_TEXTURE_FORMAT_Y8,
    NX3D_TEXTURE_FORMAT_AY8,
    NX3D_TEXTURE_FORMAT_A1R5G5B5,
    NX3D_TEXTURE_FORMAT_X1R5G5B5,
    NX3D_TEXTURE_FORMAT_A4R4G4B4,
    NX3D_TEXTURE_FORMAT_R5G6B5,
    NX3D_TEXTURE_FORMAT_A8R8G8B8,
    NX3D_TEXTURE_FORMAT_X8R8G8B8,
    NX3D_TEXTURE_FORMAT_I8_A8R8G8B8,
    NX3D_TEXTURE_FORMAT_DXT1,
    NX3D_TEXTURE_FORMAT_DXT23,
    NX3D_TEXTURE_FORMAT_DXT45,
    NX3D_TEXTURE_FORMAT_A8,
    NX3D_TEXTURE_FORMAT_A8Y8,
    NX3D_TEXTURE_FORMAT_R6G5B5,
    NX3D_TEXTURE_FORMAT_G8B8,
    NX3D_TEXTURE_FORMAT_R8B8,
    NX3D_TEXTURE_FORMAT_A8B8G8R8,
    NX3D_TEXTURE_FORMAT_B8G8R8A8,
    NX3D_TEXTURE_FORMAT_R8G8B8A8
} NX3D_TEXTURE_FORMAT;

typedef enum NX3D_TEXTURE_TYPE
{
    NX3D_TEXTURE_2D,
    NX3D_TEXTURE_3D,
    NX3D_TEXTURE_CUBE,
} NX3D_TEXTURE_TYPE;

typedef struct NX3D_TEXTURE_DESCRIPTOR
{
    NX3D_TEXTURE_TYPE type;
    NX3D_TEXTURE_FORMAT format;
    uint16_t width;
    uint16_t height;
    uint16_t depth;
    uint16_t mipmap_count;
} NX3D_TEXTURE_DESCRIPTOR;

typedef enum NX3D_TEXTURE_SLOT
{
    NX3D_TEXTURE_SLOT_0,
    NX3D_TEXTURE_SLOT_1,
    NX3D_TEXTURE_SLOT_2,
    NX3D_TEXTURE_SLOT_3,
} NX3D_TEXTURE_SLOT;

typedef enum NX3D_ENABLE_OPTIONS
{
    NX3D_ENABLE_VERTEX_COLOR
} NX3D_ENABLE_OPTIONS;

typedef enum NX3D_VIDEO_FORMAT
{
    NX3D_VIDEO_FORMAT_A1R5G5B5,
    NX3D_VIDEO_FORMAT_R5G6B5,
    NX3D_VIDEO_FORMAT_A8R8G8B8,
    NX3D_VIDEO_FORMAT_Y8,
    NX3D_VIDEO_FORMAT_R8B8,
    NX3D_VIDEO_FORMAT_G8B8,
    NX3D_VIDEO_FORMAT_AY8,
    NX3D_VIDEO_FORMAT_X1R5G5B5,
    NX3D_VIDEO_FORMAT_A4R4G4B4,
    NX3D_VIDEO_FORMAT_X8R8G8B8,
    NX3D_VIDEO_FORMAT_A8,
    NX3D_VIDEO_FORMAT_A8Y8
} NX3D_VIDEO_FORMAT;

typedef enum NX3D_SURFACE_BUFFER_FORMAT
{
    NX3D_SURFACE_BUFFER_FORMAT_Z24S8,
    NX3D_SURFACE_BUFFER_FORMAT_Z16,
} NX3D_BUFFER_FORMAT;

#define NX3D_VIDEO_PARAMS_DEFAULT = \
{ \
    640, 480, 60, \
    NX3D_VIDEO_FORMAT_A8R8G8B8, \
    NX3D_SURFACE_BUFFER_FORMAT_Z24S8, \
    2 \
}

typedef struct NX3D_VIDEO_PARAMETERS
{
    uint32_t width;
    uint32_t height;
    uint32_t refresh;
    NX3D_VIDEO_FORMAT color_format;
    NX3D_BUFFER_FORMAT buffer_format;
    uint32_t buffer_count;
} NX3D_VIDEO_PARAMETERS;

/* Column-major matrices acting on column vectors, as in OpenGL. */
typedef float NX3D_MATRIX[16];

typedef struct NX3D_VERTEX
{
    float position[3];
    uint32_t color;
    float normal[3];
    float texcoord[2];
} NX3D_VERTEX;

typedef enum NX3D_MATRIX_MODE
{
    NX3D_MODELVIEW, NX3D_PROJECTION
} NX3D_MATRIX_MODE;

typedef enum NX3D_CLEAR_BUFFER_BITS
{
    NX3D_CLEAR_COLOR =   0x1,
    NX3D_CLEAR_DEPTH =   0x2,
    NX3D_CLEAR_STENCIL = 0x4
} NX3D_CLEAR_BUFFER_BITS;

typedef enum NX3D_TRANSFORM_MODE
{
    NX3D_TRANSFORM_FIXED,
    NX3D_TRANSFORM_PROGRAM
} NX3D_TRANSFORM_MODE;

typedef enum NX3D_FACE_WINDING_ORDER
{
    NX3D_WINDING_CW,
    NX3D_WINDING_CCW,
} NX3D_FACE_WINDING_ORDER;

typedef enum NX3D_CULL_FACE
{
    NX3D_CULL_NONE,
    NX3D_CULL_BACK,
    NX3D_CULL_FRONT
} NX3D_CULL_FACE;

#ifdef __cplusplus
extern "C" {
#endif

/* Returns and clears the first error since the previous call. */
int nx3d_get_error(void);
/* Set the video mode first. nx3d owns pbkit until shutdown; do not mix raw
 * pbkit rendering with this API. One context, used from one thread only.
 * Fallible operations return 0 on success, -1 on failure. */
int nx3d_init();
int nx3d_shutdown(void);
int nx3d_begin_frame(void);
int nx3d_end_frame(void);
int nx3d_print(const char *fmt, ...);
/* Full framebuffer clear, independent of viewport/depth write state.
 * Color is 0xAARRGGBB. Depth is in [0, 1]. Requires an active frame. */
int nx3d_clear(uint32_t flags, uint32_t color, float depth, uint8_t stencil);
/* Top-left origin; positive dimensions contained in the framebuffer.
 * Clip-space Z follows OpenGL [-w, w], mapped to a fixed Z24S8 buffer. */
int nx3d_viewport(int x, int y, int width, int height);
int nx3d_load_matrix(NX3D_MATRIX_MODE mode, const float *matrix);
int nx3d_load_identity(NX3D_MATRIX_MODE mode);
int nx3d_depth_test(int enabled); /* LESS comparison; disabled by default. */
/* Unlit, smooth, unculled triangle list; count must be a multiple of three.
 * Data is copied into the pushbuffer before returning: stack arrays work.
 * Requires an active frame; count == 0 permits NULL and does nothing. */
int nx3d_draw_triangles(const NX3D_VERTEX *vertices, uint32_t count);
int nx3d_draw_primitives(NX3D_PRIMITIVE_TYPE type, uint32_t start, uint32_t count);
int nx3d_draw_indexed_primitives(NX3D_PRIMITIVE_TYPE type, uint32_t start_index, uint32_t index_count);
/* Returns `0` on success. Release created buffer with `nx3d_release_buffer(NX3D_BUFFER *)` */
int nx3d_create_buffer(NX3D_BUFFER **p_buffer, NX3D_BUFFER_TYPE type, uint32_t length);
/* Returns `0` on success. Release created layout with `nx3d_release_element_layout(NX3D_VERTEX_ELEMENT_LAYOUT *)` */
int nx3d_create_element_layout(NX3D_VERTEX_ELEMENT_LAYOUT **p_layout, uint32_t vertex_stride, const NX3D_VERTEX_DATA_DESC *descriptors, uint32_t count);
/* Returns `0` on success. Release created textures with `nx3d_release_textures(NX3D_TEXTURE *, uint32_t)` */
int nx3d_create_textures(NX3D_TEXTURE **p_textures, NX3D_TEXTURE_DESCRIPTOR *descriptors, uint32_t texture_count);
int nx3d_release_buffer(NX3D_BUFFER *buffer);
int nx3d_release_element_layout(NX3D_VERTEX_ELEMENT_LAYOUT *layout);
int nx3d_release_textures(NX3D_TEXTURE *textures, uint32_t texture_count);
int nx3d_bind_vertex_buffer(NX3D_BUFFER *buffer, NX3D_VERTEX_ELEMENT_LAYOUT *layout);
int nx3d_bind_index_buffer(NX3D_BUFFER *buffer, NX3D_INDEX_DATA_TYPE index_type);
int nx3d_bind_texture(NX3D_TEXTURE *texture, NX3D_SAMPLER_DESC *sampler, NX3D_TEXTURE_SLOT texture_slot);
int nx3d_buffer_data(NX3D_BUFFER *buffer, const void *data, uint32_t length);
int nx3d_set_mode(NX3D_TRANSFORM_MODE mode);
int nx3d_enable(NX3D_ENABLE_OPTIONS option, uint32_t value);
int nx3d_set_winding(NX3D_FACE_WINDING_ORDER winding_order);
int nx3d_set_cull_face(NX3D_CULL_FACE cull_face);
int nx3d_set_texture_pixel_data(NX3D_TEXTURE *texture, const void *pixel_data, uint32_t length);
void nx3d_sampler_default(NX3D_SAMPLER_DESC *desc);
void nx3d_generate_normals(NX3D_VERTEX *vertices, const void *triangles, NX3D_INDEX_DATA_TYPE triangle_type, uint32_t triangle_count);
#ifdef __cplusplus
}
#endif
#endif
