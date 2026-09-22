#ifndef NX3D_STATE_H
#define NX3D_STATE_H

#include "nx3d.h"
#include <string.h>
#include <strings.h>
#include <synchapi.h>

#define NX3D_MAX_TEX    4

#define NX3D_MASK(mask, val) (((val) << (ffs(mask)-1)) & (mask))
#define MATRX_CPY(dest, src) (memcpy(dest, src, (4 * 4 * sizeof(float))))
#define OBJ_PTR(object)      ((NX3D_OBJ *)&object)
#define OBJ(object)          ((NX3D_OBJ)object)

typedef void *NX3D_OBJ;

typedef struct NX3D_OBJ_HEADER NX3D_OBJ_HEADER;

struct NX3D_OBJ_HEADER
{
    uint32_t obj_type;
    NX3D_OBJ *p_next;
    NX3D_OBJ *p_prev;
    RTL_CRITICAL_SECTION lock;
};

typedef struct NX3D_ELEMENT
{
    uint32_t v;
    uint32_t nv_type;
    uint32_t component_bytes;
    uint32_t count;
    uint32_t aligned_offset;
} NX3D_ELEMENT;

struct NX3D_VERTEX_ELEMENT_LAYOUT
{
    NX3D_OBJ_HEADER h;
    uint32_t count;
    uint32_t stride;
    int is_allocated;
    NX3D_ELEMENT *elements;
};

struct NX3D_BUFFER
{
    NX3D_OBJ_HEADER h;
    size_t length;
    int is_allocated;
    NX3D_BUFFER_TYPE type;
    union { void *__p; void *buffer; };
};

struct NX3D_TEXTURE
{
    NX3D_OBJ_HEADER h;
    NX3D_TEXTURE_TYPE type;
    uint16_t width;
    uint16_t height;
    uint16_t pitch;
    uint32_t format;
    uint32_t length;
    char *pixel_data;
};

typedef struct nx3d_tex_state
{
    NX3D_TEXTURE *t;
    NX3D_SAMPLER_DESC s;
    NX3D_COLOR_PALETTE *palette;
} nx3d_tex_state;

/* Setters accumulate these bits; init/frame boundaries invalidate all groups.
 * Viewport changes also dirty matrices because the composite includes it.
 * Texture bindings also dirty combiners because their inputs depend on bindings. */
enum nx3d_dirty_flags
{
    NX3D_DIRTY_FACE = 1u << 0,
    NX3D_DIRTY_MATRICES = 1u << 1,
    NX3D_DIRTY_VIEWPORT = 1u << 2,
    NX3D_DIRTY_DEPTH = 1u << 3,
    NX3D_DIRTY_VERTEX_ARRAY = 1u << 4,
    NX3D_DIRTY_TEXTURES = 1u << 5,
    NX3D_DIRTY_COMBINERS = 1u << 6,
    NX3D_DIRTY_TRANSFORM = 1u << 7,
    NX3D_DIRTY_ALL = (1u << 8) - 1
};

typedef struct nx3d_state
{
    NX3D_TRANSFORM_MODE transform_mode;
    int initialized, frame, error;
    int width, height, x, y, vw, vh;
    int depth_test, depth_write;
    uint32_t dirty;
    int vertex_color;
    unsigned int command_budget;
    NX3D_FACE_WINDING_ORDER winding_order;
    NX3D_CULL_FACE cull_face;
    NX3D_MATRIX modelview, projection, tex_matrix;
    NX3D_INDEX_DATA_TYPE index_type;
    NX3D_BUFFER *buffers;
    NX3D_VERTEX_ELEMENT_LAYOUT *layouts;
    NX3D_TEXTURE *textures;
    NX3D_BUFFER *vertex_buffer;
    NX3D_VERTEX_ELEMENT_LAYOUT *vertex_layout;
    NX3D_BUFFER *index_buffer;
    int vertex_validated;
    nx3d_tex_state tex_state[NX3D_MAX_TEX];
} nx3d_state;

#ifdef __cplusplus
extern "C" {
#endif

extern nx3d_state state;

/* pbkit does not wrap automatically. Reserve at most 128 DWORDs per batch
 * and recycle well before even its minimum 64 KiB pushbuffer is exhausted.
 * State survives a reset; finish each primitive before recycling. */
uint32_t *begin_commands(void);
void apply_state(void);
void setup_pipeline(void);

#ifdef __cplusplus
}
#endif

#endif
