#include "nx3d.h"
#include "nx3d_state.h"
#include <math.h>
#include <pbkit/pbkit.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <xboxkrnl/xboxkrnl.h>

static const char log_filename[] = "D:\\output.txt";

static const NX3D_MATRIX identity =
{
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

#define OPEN_LOG(f) \
    FILE *f; \
    if ((f = open_log()) != NULL)

static FILE *open_log(void)
{
    FILE *f = NULL;
    f = fopen(log_filename, "a");
    if (!f) return NULL;
    return f;
}

static void format_float(char *s, size_t len, float f, uint32_t num_decimals)
{
    if (!s || len == 0) return;
    memset(s, 0, len);
    snprintf(s, len, "%d", (int)f);
    if (num_decimals == 0)
        return;

    size_t whole_len = strlen(s);
    size_t string_len = whole_len + 1 + num_decimals;
    if (string_len + 1 > len) return;
    uint32_t offset = whole_len;
    *(s + offset) = '.';
    offset++;

    f -= (int)f;
    for (uint32_t i = 0; i < num_decimals; i++)
    {
        snprintf((s + offset), len, "%d", (int)(f *= 10));
        f -= (int)f;
        offset++;
    }
    *(s + offset) = '\0';
}

static void format_matrix(char *s, size_t len, float *f, uint32_t num_decimals)
{
    if (!s || len == 0) return;
    memset(s, 0, len);

    // todo: implement matrix formatting
}

static void log_state(void)
{
    OPEN_LOG(f)
    {
        fprintf(f, "\n--- nx3d state ---\n");
        fprintf(f, "initialized=%d frame=%d error=%d dirty=%u\n",
                state.initialized, state.frame, state.error, state.dirty);
        fprintf(f, "width=%d height=%d x=%d y=%d vw=%d vh=%d\n",
                state.width, state.height, state.x, state.y, state.vw, state.vh);
        fprintf(f, "depth_test=%d depth_write=%d command_budget=%u\n",
                state.depth_test, state.depth_write, state.command_budget);
        fprintf(f, "index_type=%d vertex_validated=%d\n",
                (int)state.index_type, state.vertex_validated);
        fprintf(f, "buffers=%p layouts=%p textures=%p\n",
                (void *)state.buffers, (void *)state.layouts, (void *)state.textures);
        fprintf(f, "vertex_buffer=%p vertex_layout=%p index_buffer=%p\n",
                (void *)state.vertex_buffer, (void *)state.vertex_layout,
                (void *)state.index_buffer);

        /* Omit matrices: nxdk does not enable floating-point formatting by default. */
        for (unsigned int i = 0; i < NX3D_MAX_TEX; ++i)
            fprintf(f, "tex_state[%u]: texture=%p sampler=%p palette=%p\n", i,
                    (void *)state.tex_state[i].t, (void *)&state.tex_state[i].s,
                    (void *)state.tex_state[i].palette);
        fprintf(f, "--- end nx3d state ---\n");
        fclose(f);
    }
}

static int fail(int error)
{
    OPEN_LOG(f)
    {
        fprintf(f, "-- nx3d fail: error=%d", error);
        fclose(f);
    }

    if (!state.error) state.error = error;
    return -1;
}

static int create_vertex_buffer(NX3D_BUFFER *buffer, uint32_t length)
{
    void *p;
    if (!(p = MmAllocateContiguousMemoryEx(length, 0, 0x03ffb000, 0, PAGE_READWRITE | PAGE_WRITECOMBINE)))
        return fail(NX3D_ERROR_OUT_OF_MEMORY);

    memset(p, 0, length);
    buffer->__p = p;
    buffer->length = length;
    buffer->is_allocated = TRUE;
    buffer->type = NX3D_VERTEX_BUFFER;
    return 0;
}

static int create_index_buffer(NX3D_BUFFER *buffer, uint32_t length)
{
    void *p;
    uint32_t remainder = length % 4;
    if (remainder > 0)
        length += (4 - remainder);
    if (!(p = malloc(length)))
        return fail(NX3D_ERROR_OUT_OF_MEMORY);

    memset(p, 0, length);
    buffer->__p = p;
    buffer->length = length;
    buffer->is_allocated = TRUE;
    buffer->type = NX3D_INDEX_BUFFER;
    return 0;
}

static int validate_index_buffer(void)
{
    uint32_t element_size = state.index_type == NX3D_UINT32 ? 4 : 2;
    uint32_t max_vertex_count = state.vertex_buffer->length / state.vertex_layout->stride;
    uint32_t max_index_count = state.index_buffer->length / element_size;
    const char *index_data = state.index_buffer->__p;

    uint32_t index_offset;
    for (uint32_t i = 0; i < max_index_count; i++)
    {
        index_offset = i * element_size;
        uint32_t index = state.index_type == NX3D_UINT32 ? *(const uint32_t *)(index_data + index_offset) :
            *(const uint16_t *)(index_data + index_offset);

        if (index >= max_vertex_count)
            return TRUE;
    }

    return FALSE;
}

static void register_object(NX3D_OBJ obj, NX3D_OBJ *obj_tail)
{
    NX3D_OBJ_HEADER *h;
    h = (NX3D_OBJ_HEADER *)obj;
    if (*obj_tail)
    {
        h->p_prev = *obj_tail;
        (*(NX3D_OBJ_HEADER **)(obj_tail))->p_next = obj;
    }
    *obj_tail = obj;
};

static int get_bits_per_pixel(NX3D_TEXTURE_FORMAT format)  // returns zero for compressed formats `NX3D_TEXTURE_FORMAT_DXT*`
{
    switch (format)
    {
        case NX3D_TEXTURE_FORMAT_Y8:
        case NX3D_TEXTURE_FORMAT_AY8:
        case NX3D_TEXTURE_FORMAT_I8_A8R8G8B8:
        case NX3D_TEXTURE_FORMAT_A8:
            return 8;

        case NX3D_TEXTURE_FORMAT_A1R5G5B5:
        case NX3D_TEXTURE_FORMAT_X1R5G5B5:
        case NX3D_TEXTURE_FORMAT_A4R4G4B4:
        case NX3D_TEXTURE_FORMAT_R5G6B5:
        case NX3D_TEXTURE_FORMAT_A8Y8:
        case NX3D_TEXTURE_FORMAT_R6G5B5:
        case NX3D_TEXTURE_FORMAT_G8B8:
        case NX3D_TEXTURE_FORMAT_R8B8:
        return 16;

        case NX3D_TEXTURE_FORMAT_A8R8G8B8:
        case NX3D_TEXTURE_FORMAT_X8R8G8B8:
        case NX3D_TEXTURE_FORMAT_A8B8G8R8:
        case NX3D_TEXTURE_FORMAT_B8G8R8A8:
        case NX3D_TEXTURE_FORMAT_R8G8B8A8:
            return 32;

        // return zero for compressed formats
        case NX3D_TEXTURE_FORMAT_DXT1:
        case NX3D_TEXTURE_FORMAT_DXT23:
        case NX3D_TEXTURE_FORMAT_DXT45:
        default:
            return 0;
    }
}

static int is_pow_two(uint16_t width, uint16_t height)
{
    return width != 0 && (width & (width - 1)) == 0
        && height != 0 && (height & (height - 1)) == 0;
}

int nx3d_get_error(void)
{
    int error = state.error;
    state.error = NX3D_ERROR_NONE;
    return error;
}

int nx3d_init()
{
    if (state.initialized) return fail(NX3D_ERROR_ALREADY_INITIALIZED);
    if (pb_init()) return fail(NX3D_ERROR_PBKIT_INIT_FAILED);

    state.initialized = 1;
    state.command_budget = 0;
    state.width = pb_back_buffer_width();
    state.height = pb_back_buffer_height();
    state.x = state.y = 0;
    state.vw = state.width;
    state.vh = state.height;
    state.depth_test = 0;
    state.depth_write = 1;
    state.frame = 0;
    state.dirty = NX3D_DIRTY_ALL;
    memcpy(state.modelview, identity, sizeof(identity));
    memcpy(state.projection, identity, sizeof(identity));
    memcpy(state.tex_matrix, identity, sizeof(identity));
    state.buffers = NULL;
    state.layouts = NULL;
    state.vertex_buffer = NULL;
    state.vertex_layout = NULL;
    state.index_buffer = NULL;
    state.index_type = NX3D_UINT16;
    state.vertex_validated = FALSE;
    for (uint32_t i = 0; i < NX3D_MAX_TEX; i++)
    {
        state.tex_state[i].t = NULL;
        memset(&state.tex_state[i].s, 0, sizeof(NX3D_SAMPLER_DESC));
        state.tex_state[i].palette = NULL;
    }
    setup_pipeline();
    pb_show_front_screen();
    return 0;
}

int nx3d_shutdown(void)
{
    if (!state.initialized) return fail(NX3D_ERROR_NOT_INITIALIZED);
    while (pb_busy()) {}
    pb_kill();
    state.initialized = state.frame = 0;
    return 0;
}

int nx3d_begin_frame(void)
{
    if (!state.initialized) return fail(NX3D_ERROR_NOT_INITIALIZED);
    if (state.frame) return fail(NX3D_ERROR_INVALID_OPERATION);
    pb_erase_text_screen();
    pb_wait_for_vbl();
    pb_reset();
    pb_target_back_buffer();
    state.command_budget = 128; /* Includes target selection and swap headroom. */
    /* pb_target_back_buffer() enables W-buffering in CONTROL0 each frame.
     * Our projection/viewport produces fixed-point Z24 depth instead.
     * Preserve perspective-correct textures and stencil writes. */
    uint32_t *p = begin_commands();
    p = pb_push1(p, NV097_SET_CONTROL0,
                 NV097_SET_CONTROL0_Z_FORMAT_FIXED |
                 NV097_SET_CONTROL0_TEXTURE_PERSPECTIVE_ENABLE |
                 NV097_SET_CONTROL0_STENCIL_WRITE_ENABLE);
    pb_end(p);
    state.frame = 1;
    return 0;
}

int nx3d_end_frame(void)
{
    if (!state.initialized) return fail(NX3D_ERROR_NOT_INITIALIZED);
    if (!state.frame) return fail(NX3D_ERROR_INVALID_OPERATION);
    pb_draw_text_screen();
    while (pb_busy()) {}
    while (pb_finished()) {}
    state.frame = 0;
    return 0;
}

int nx3d_print(const char *fmt, ...)
{
    if (!state.initialized) return fail(NX3D_ERROR_NOT_INITIALIZED);
    if (!state.frame) return fail(NX3D_ERROR_INVALID_OPERATION);
    char buffer[512];
    uint32_t i;

    va_list argList;
    va_start(argList, fmt);
    vsprintf(buffer, fmt, argList);
    va_end(argList);

    for (i = 0; i < strlen(buffer); i++) {
        pb_print_char(buffer[i]);
    }
    return 0;
}

int nx3d_clear(uint32_t flags, uint32_t color, float depth, uint8_t stencil)
{
    if (!state.initialized) return fail(NX3D_ERROR_NOT_INITIALIZED);
    if (!state.frame) return fail(NX3D_ERROR_INVALID_OPERATION);
    if ((flags & ~0x7) || !isfinite(depth) || depth < 0 || depth > 1)
        return fail(NX3D_ERROR_INVALID_ARGUMENT);
    if (!flags) return 0;
    uint32_t mask = 0;
    if (flags & NX3D_CLEAR_COLOR) mask |= NV097_CLEAR_SURFACE_COLOR;
    if (flags & NX3D_CLEAR_DEPTH) mask |= NV097_CLEAR_SURFACE_Z;
    if (flags & NX3D_CLEAR_STENCIL) mask |= NV097_CLEAR_SURFACE_STENCIL;
    uint32_t *p = begin_commands();
    p = pb_push1(p, NV097_SET_CLEAR_RECT_HORIZONTAL, (state.width - 1) << 16);
    p = pb_push1(p, NV097_SET_CLEAR_RECT_VERTICAL, (state.height - 1) << 16);
    p = pb_push1(p, NV097_SET_COLOR_CLEAR_VALUE, color);
    p = pb_push1(p, NV097_SET_ZSTENCIL_CLEAR_VALUE,
                 ((uint32_t)(depth * 1.6777215e7f) << 8) | stencil);
    p = pb_push1(p, NV097_CLEAR_SURFACE, mask);
    pb_end(p);
    return 0;
}

int nx3d_viewport(int x, int y, int width, int height)
{
    if (!state.initialized) return fail(NX3D_ERROR_NOT_INITIALIZED);
    if (x < 0 || y < 0 || width <= 0 || height <= 0 ||
        width > state.width || height > state.height ||
        x > state.width - width || y > state.height - height)
        return fail(NX3D_ERROR_INVALID_ARGUMENT);
    state.x = x; state.y = y; state.vw = width; state.vh = height;
    state.dirty |= NX3D_DIRTY_VIEWPORT | NX3D_DIRTY_MATRICES;
    return 0;
}

int nx3d_load_matrix(NX3D_MATRIX_MODE mode, const float *matrix)
{
    if (!state.initialized)
        return fail(NX3D_ERROR_NOT_INITIALIZED);

    float *dst_matrix = NULL;
    switch (mode)
    {
        case NX3D_MODELVIEW:
            dst_matrix = state.modelview;
            break;
        case NX3D_PROJECTION:
            dst_matrix = state.projection;
            break;
        default:
            return fail(NX3D_ERROR_INVALID_ARGUMENT);
    }

    if (!matrix)
    {
        memcpy(dst_matrix, &identity, sizeof(NX3D_MATRIX));
        state.dirty |= NX3D_DIRTY_MATRICES;
        return 0;
    }

    for (int i = 0; i < 16; ++i)
        if (!isfinite(matrix[i]))
            return fail(NX3D_ERROR_INVALID_ARGUMENT);

    memcpy(dst_matrix, matrix, sizeof(NX3D_MATRIX));
    state.dirty |= NX3D_DIRTY_MATRICES;
    return 0;
}

int nx3d_load_identity(NX3D_MATRIX_MODE mode)
{
    return nx3d_load_matrix(mode, identity);
}

int nx3d_depth_test(int enabled)
{
    if (!state.initialized) return fail(NX3D_ERROR_NOT_INITIALIZED);
    state.depth_test = !!enabled;
    state.dirty |= NX3D_DIRTY_DEPTH;
    return 0;
}

int nx3d_draw_triangles(const NX3D_VERTEX *vertices, uint32_t count)
{
    if (!state.initialized) return fail(NX3D_ERROR_NOT_INITIALIZED);
    if (!state.frame) return fail(NX3D_ERROR_INVALID_OPERATION);
    if (count % 3 || (count && !vertices) || count > UINT32_MAX / sizeof(*vertices))
        return fail(NX3D_ERROR_INVALID_ARGUMENT);

    for (uint32_t i = 0; i < count; ++i)
        for (int j = 0; j < 3; ++j)
            if (!isfinite(vertices[i].position[j]))
                return fail(NX3D_ERROR_INVALID_ARGUMENT);

    if (!count) return 0;
    apply_state();
    /* Each complete triangle uses 34 DWORDs, below pbkit's 128-DWORD
     * reservation. No DMA-accessible application allocation is needed. */
    for (uint32_t i = 0; i < count; i += 3) {
        uint32_t *p = begin_commands();
        p = pb_push1(p, NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_TRIANGLES);
        for (int j = 0; j < 3; j++)
        {
            const NX3D_VERTEX *v = &vertices[i+j];
            float a = (float)((v->color >> 24) & 0xFF) / 255.0f;
            float b = (float)((v->color >> 16) & 0xFF) / 255.0f;
            float g = (float)((v->color >>  8) & 0xFF) / 255.0f;
            float r = (float)((v->color >>  0) & 0xFF) / 255.0f;
            p = pb_push2f(p, NV097_SET_TEXCOORD0_2F, v->texcoord[0], v->texcoord[1]);
            p = pb_push3f(p, NV097_SET_NORMAL3F, v->normal[0], v->normal[1], v->normal[2]);
            p = pb_push4f(p, NV097_SET_DIFFUSE_COLOR4F, r, g, b, a);
            p = pb_push4f(p, NV097_SET_VERTEX4F, v->position[0], v->position[1], v->position[2], 1);
        }
        p = pb_push1(p, NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_END);
        pb_end(p);
    }
    return 0;
}

int nx3d_draw_primitives(NX3D_PRIMITIVE_TYPE type, uint32_t start, uint32_t count)
{
    if (!state.initialized) return fail(NX3D_ERROR_NOT_INITIALIZED);
    if (count == 0) return fail(NX3D_ERROR_INVALID_ARGUMENT);
    if (!state.frame || !state.vertex_buffer || !state.vertex_layout)
        return fail(NX3D_ERROR_INVALID_OPERATION);

    EnterCriticalSection(&state.vertex_buffer->h.lock);
    const char *vertex_data = (const char *)state.vertex_buffer->__p;
    const NX3D_ELEMENT *elements = state.vertex_layout->elements;
    for (uint32_t i = 0; i < count; i++)
    {
        uint32_t vertex_offset = (start + i) * state.vertex_layout->stride;
        for (uint32_t j = 0; j < state.vertex_layout->count; j++)
        {
            NX3D_ELEMENT e = elements[j];
            uint32_t vertex_element_offset = vertex_offset + e.aligned_offset;
            for (uint32_t k = 0; k < e.count; k++)
            {
                const char *element_data = &vertex_data[vertex_element_offset + (k * e.component_bytes)];
                // todo: validate data pointed to at element_data before issuing draw command
                switch (e.nv_type)
                {
                    case NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_UB_D3D:
                        break;

                    case NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S1:
                        break;

                    case NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F:
                        const float f = *(const float *)element_data;
                        if (!isfinite(f))
                            return fail(NX3D_ERROR_INVALID_OPERATION);
                        break;

                    case NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_UB_OGL:
                        break;

                    case NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S32K:
                        break;

                    case NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_CMP:
                        break;
                }
            }
        }
    }

    uint32_t mode;
    switch (type)
    {
        case NX3D_POINTS:
            mode = NV097_SET_BEGIN_END_OP_POINTS;
            break;
        case NX3D_LINE:
            mode = NV097_SET_BEGIN_END_OP_LINES;
            break;
        case NX3D_LINE_LOOP:
            mode = NV097_SET_BEGIN_END_OP_LINE_LOOP;
            break;
        case NX3D_LINE_STRIP:
            mode = NV097_SET_BEGIN_END_OP_LINE_STRIP;
            break;
        case NX3D_TRIANGLES:
            mode = NV097_SET_BEGIN_END_OP_TRIANGLES;
            break;
        case NX3D_TRIANGLE_STRIP:
            mode = NV097_SET_BEGIN_END_OP_TRIANGLE_STRIP;
            break;
        case NX3D_TRIANGLE_FAN:
            mode = NV097_SET_BEGIN_END_OP_TRIANGLE_FAN;
            break;
        case NX3D_QUADS:
            mode = NV097_SET_BEGIN_END_OP_QUADS;
            break;
        case NX3D_QUADS_STRIP:
            mode = NV097_SET_BEGIN_END_OP_QUAD_STRIP;
            break;
        default:
            return fail(NX3D_ERROR_INVALID_ARGUMENT);
    }

    apply_state();
    uint32_t draw = NX3D_MASK(NV097_DRAW_ARRAYS_COUNT, (count - 1))
        | NX3D_MASK(NV097_DRAW_ARRAYS_START_INDEX, start);

    uint32_t *p = begin_commands();
    p = pb_push1(p, NV097_SET_BEGIN_END, mode);
    p = pb_push1(p, 0x40000000 | NV097_DRAW_ARRAYS, draw);
    p = pb_push1(p, NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_END);
    pb_end(p);
    LeaveCriticalSection(&state.vertex_buffer->h.lock);
    return 0;
}

int nx3d_draw_indexed_primitives(NX3D_PRIMITIVE_TYPE type, uint32_t start_index, uint32_t index_count)
{
    if (!state.initialized) return fail(NX3D_ERROR_NOT_INITIALIZED);
    if (index_count == 0) return fail(NX3D_ERROR_INVALID_ARGUMENT);
    if (!state.frame || !state.index_buffer || !state.vertex_buffer || !state.vertex_layout)
        return fail(NX3D_ERROR_INVALID_OPERATION);
    if (state.index_type != NX3D_UINT16 && state.index_type != NX3D_UINT32)
        return fail(NX3D_ERROR_INVALID_OPERATION);
    if (!state.vertex_validated)
        return fail(NX3D_ERROR_INVALID_OPERATION);

    const char *indices = (const char *)state.index_buffer->buffer;
    uint32_t mode, max_index_count, element, element_size, batched;

    switch (type)
    {
        case NX3D_POINTS:
            mode = NV097_SET_BEGIN_END_OP_POINTS;
            break;
        case NX3D_LINE:
            mode = NV097_SET_BEGIN_END_OP_LINES;
            break;
        case NX3D_LINE_LOOP:
            mode = NV097_SET_BEGIN_END_OP_LINE_LOOP;
            break;
        case NX3D_LINE_STRIP:
            mode = NV097_SET_BEGIN_END_OP_LINE_STRIP;
            break;
        case NX3D_TRIANGLES:
            mode = NV097_SET_BEGIN_END_OP_TRIANGLES;
            break;
        case NX3D_TRIANGLE_STRIP:
            mode = NV097_SET_BEGIN_END_OP_TRIANGLE_STRIP;
            break;
        case NX3D_TRIANGLE_FAN:
            mode = NV097_SET_BEGIN_END_OP_TRIANGLE_FAN;
            break;
        case NX3D_QUADS:
            mode = NV097_SET_BEGIN_END_OP_QUADS;
            break;
        case NX3D_QUADS_STRIP:
            mode = NV097_SET_BEGIN_END_OP_QUAD_STRIP;
            break;
        default:
            return fail(NX3D_ERROR_INVALID_ARGUMENT);
    }

    switch (state.index_type)
    {
        case NX3D_UINT16:
            element = NV097_ARRAY_ELEMENT16;
            element_size = 2;
            break;

        case NX3D_UINT32:
            element = NV097_ARRAY_ELEMENT32;
            element_size = 4;
            break;
    }

    max_index_count = state.index_buffer->length / element_size;
    if (start_index + index_count > max_index_count)
        return fail(NX3D_ERROR_INVALID_ARGUMENT);

    if (validate_index_buffer())
        return fail(NX3D_ERROR_INVALID_OPERATION);

    apply_state();

    uint32_t *p;
    indices += (start_index * element_size);
    const uint32_t *elements = (const uint32_t *)indices;
    const uint32_t dword_count = (index_count * element_size) / sizeof(uint32_t);
    for (uint32_t i = 0; i < dword_count;)
    {
        batched = (dword_count - i) > 120 ? 120 : (dword_count - i);
        while (pb_busy());
        p = begin_commands();
        p = pb_push1(p, NV097_SET_BEGIN_END, mode);
        pb_push(p++, 0x40000000 | element, batched);
        memcpy(p, &elements[i], batched * 4);
        p += batched;
        p = pb_push1(p, NV097_SET_BEGIN_END, NV097_SET_BEGIN_END_OP_END);
        pb_end(p);
        i += batched;
    }

    return 0;
}

int nx3d_create_buffer(NX3D_BUFFER **p_buffer, NX3D_BUFFER_TYPE type, uint32_t length)
{
    if (!p_buffer) return fail(NX3D_ERROR_INVALID_ARGUMENT);
    if (length == 0) return fail(NX3D_ERROR_INVALID_ARGUMENT);
    switch (type)
    {
        case NX3D_VERTEX_BUFFER:
        case NX3D_INDEX_BUFFER:
            break;
        default: return fail(NX3D_ERROR_INVALID_ARGUMENT);
    }

    NX3D_BUFFER *buffer = (NX3D_BUFFER *)calloc(1, sizeof(NX3D_BUFFER));
    if (!buffer) return fail(NX3D_ERROR_OUT_OF_MEMORY);
    int status;
    switch (type)
    {
        case NX3D_VERTEX_BUFFER:
            if ((status = create_vertex_buffer(buffer, length)))
                return status;
            break;
        case NX3D_INDEX_BUFFER:
            if ((status = create_index_buffer(buffer, length)))
                return status;
            break;
        default:
            return fail(NX3D_ERROR_INVALID_ARGUMENT);
    }

    InitializeCriticalSection(&buffer->h.lock);
    register_object(OBJ(buffer), OBJ_PTR(state.buffers));
    *p_buffer = buffer;

    return 0;
}

int nx3d_create_element_layout(NX3D_VERTEX_ELEMENT_LAYOUT **p_layout, uint32_t vertex_stride, const NX3D_VERTEX_DATA_DESC *descriptors, uint32_t count)
{
    if (!p_layout || !descriptors || count == 0 || vertex_stride == 0)
        return fail(NX3D_ERROR_INVALID_ARGUMENT);

    NX3D_VERTEX_ELEMENT_LAYOUT *layout = (NX3D_VERTEX_ELEMENT_LAYOUT *)calloc(1, sizeof(NX3D_VERTEX_ELEMENT_LAYOUT));
    if (!layout) return fail(NX3D_ERROR_OUT_OF_MEMORY);
    memset(layout, 0, sizeof(NX3D_VERTEX_ELEMENT_LAYOUT));

    NX3D_ELEMENT *elements = (NX3D_ELEMENT *)malloc(sizeof(NX3D_ELEMENT) * count);
    for (uint32_t i = 0; i < count; i++)
    {
        NX3D_VERTEX_DATA_DESC desc = descriptors[i];
        NX3D_ELEMENT *e = &elements[i];
        e->count = desc.count;
        e->aligned_offset = desc.aligned_offset;
        if (desc.semantic_index > NX3D_TEXCOORD3)
            return fail(NX3D_ERROR_INVALID_OPERATION);
        e->v = desc.semantic_index;

        switch (desc.type)
        {
            case NX3D_FLOAT32:
                e->nv_type = NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F;
                e->component_bytes = 4;
                break;
            case NX3D_UNORM8_BGRA:
                e->nv_type = NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_UB_D3D;
                e->component_bytes = 1;
                break;
            case NX3D_SNORM16:
                e->nv_type = NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S1;
                e->component_bytes = 2;
                break;
            case NX3D_UNORM8_RGBA:
                e->nv_type = NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_UB_OGL;
                e->component_bytes = 1;
                break;
            case NX3D_SINT16:
                e->nv_type = NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S32K;
                e->component_bytes = 2;
                break;
            case NX3D_SNORM_11_11_10:
                e->nv_type = NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_CMP;
                e->component_bytes = 4;
                break;
        }
    }

    layout->count = count;
    layout->stride = vertex_stride;
    layout->elements = elements;
    layout->is_allocated = TRUE;
    register_object(OBJ(layout), OBJ_PTR(state.layouts));
    memcpy(p_layout, &layout, sizeof(layout));

    return 0;
}

int nx3d_create_textures(NX3D_TEXTURE **p_textures, NX3D_TEXTURE_DESCRIPTOR *descriptors, uint32_t texture_count)
{
    int result;
    if (p_textures == NULL || descriptors == NULL || texture_count == 0)
        return fail(NX3D_ERROR_INVALID_ARGUMENT);

    NX3D_TEXTURE *tex, *textures = (NX3D_TEXTURE *)calloc(texture_count, sizeof(NX3D_TEXTURE));
    NX3D_TEXTURE_DESCRIPTOR t;
    for (uint32_t i = 0; i < texture_count; i++)
    {
        tex = &textures[i];
        t = descriptors[i];
        char pow_of_two = is_pow_two(t.width, t.height);
        int bpp = get_bits_per_pixel(t.format);
        uint32_t pitch = bpp * t.width / 8;

        if (pitch == 0)
        {
            if (!pow_of_two)
                return fail(NX3D_ERROR_INVALID_OPERATION);

            uint32_t tex_count = t.width * t.height / 16;
            uint32_t size;
            switch (t.format)
            {
                case NX3D_TEXTURE_FORMAT_DXT1:
                    size = tex_count * 8;
                    break;
                case NX3D_TEXTURE_FORMAT_DXT23:
                case NX3D_TEXTURE_FORMAT_DXT45:
                    size = tex_count * 16;
                    break;
                default:
                    return fail(NX3D_ERROR_INVALID_OPERATION);
            }
            pitch = size / t.height;
        }

        uint32_t color_format = NV097_SET_TEXTURE_FORMAT_COLOR_LU_IMAGE_A8R8G8B8;
        /* CONTEXT_DMA is a field mask, not a selector: 1 selects DMA A.
         * Linear 2D textures also require dimensionality 2 for NPOT sizes. */
        tex->format = NX3D_MASK(NV097_SET_TEXTURE_FORMAT_DIMENSIONALITY, 2);
        tex->format |= NX3D_MASK(NV097_SET_TEXTURE_FORMAT_CONTEXT_DMA, 1)
            | NX3D_MASK(NV097_SET_TEXTURE_FORMAT_BORDER_SOURCE, NV097_SET_TEXTURE_FORMAT_BORDER_SOURCE_COLOR)
            | NX3D_MASK(NV097_SET_TEXTURE_FORMAT_COLOR, color_format)
            | NX3D_MASK(NV097_SET_TEXTURE_FORMAT_MIPMAP_LEVELS, 1);

        tex->type = t.type;
        tex->width = t.width;
        tex->height = t.height;
        tex->pitch = pitch;
        tex->length = pitch * t.height;
        tex->pixel_data = (char *)MmAllocateContiguousMemoryEx(tex->length, 0, MAXRAM, 0, PAGE_READWRITE | PAGE_WRITECOMBINE);
        if (!tex->pixel_data)
        {
            result = fail(NX3D_ERROR_INVALID_OPERATION);
            goto cleanup;
        }
        memset(tex->pixel_data, 0, tex->length);
    }

    *p_textures = textures;
    for (uint32_t i = 0; i < texture_count; i++)
        register_object(OBJ(&textures[i]), OBJ_PTR(state.textures));
    return 0;

cleanup:
    for (uint32_t i = 0; i < texture_count; i++)
        if (textures[i].pixel_data)
            MmFreeContiguousMemory(textures[i].pixel_data);

    free(textures);
    return result;
}

int nx3d_release_buffer(NX3D_BUFFER *buffer)
{
    if (!state.initialized) return fail(NX3D_ERROR_NOT_INITIALIZED);
    if (!buffer) return fail(NX3D_ERROR_INVALID_ARGUMENT);
    if (!buffer->is_allocated) return fail(NX3D_ERROR_INVALID_OPERATION);

    while (pb_busy());
    EnterCriticalSection(&buffer->h.lock);
    switch (buffer->type)
    {
        case NX3D_VERTEX_BUFFER:
            MmFreeContiguousMemory(buffer->__p);
            break;
        
        case NX3D_INDEX_BUFFER:
            free(buffer->__p);
            break;
        
        default:
            return fail(NX3D_ERROR_NOT_IMPLEMENTED);
    }
    
    memset(buffer, 0, sizeof(NX3D_BUFFER));
    LeaveCriticalSection(&buffer->h.lock);
    free(buffer);

    return 0;
}

int nx3d_release_element_layout(NX3D_VERTEX_ELEMENT_LAYOUT *layout)
{
    if (!layout) return fail(NX3D_ERROR_INVALID_ARGUMENT);
    if (!layout->is_allocated) return fail(NX3D_ERROR_INVALID_OPERATION);

    free(layout->elements);
    memset(layout, 0, sizeof(NX3D_VERTEX_ELEMENT_LAYOUT));
    free(layout);

    return 0;
}

int nx3d_release_textures(NX3D_TEXTURE *textures, uint32_t texture_count)
{
    if (textures == NULL || texture_count == 0)
        return fail(NX3D_ERROR_INVALID_ARGUMENT);

    for (uint32_t i = 0; i < texture_count; i++)
    {
        NX3D_TEXTURE t = textures[i];
        if (t.pixel_data) MmFreeContiguousMemory(t.pixel_data);
    }
    free(textures);
    return 0;
}

int nx3d_buffer_data(NX3D_BUFFER *buffer, const void *data, uint32_t length)
{
    if (!state.initialized) return fail(NX3D_ERROR_NOT_INITIALIZED);
    if (!buffer || !data || length == 0) return fail(NX3D_ERROR_INVALID_ARGUMENT);
    if (buffer->length < length) return fail(NX3D_ERROR_INVALID_ARGUMENT);
    if (!buffer->is_allocated) return fail(NX3D_ERROR_INVALID_OPERATION);

    while (pb_busy());
    EnterCriticalSection(&buffer->h.lock);
    memcpy(buffer->__p, data, length);
    LeaveCriticalSection(&buffer->h.lock);
    return 0;
}

int nx3d_bind_vertex_buffer(NX3D_BUFFER *buffer, NX3D_VERTEX_ELEMENT_LAYOUT *layout)
{
    if (!state.initialized)
        return fail(NX3D_ERROR_NOT_INITIALIZED);
    if (!state.frame)
        return fail(NX3D_ERROR_INVALID_OPERATION);

    if (!buffer && state.vertex_buffer)
    {
        state.vertex_buffer = NULL;
        state.vertex_layout = NULL;
        state.vertex_validated = FALSE;
        state.dirty |= NX3D_DIRTY_VERTEX_ARRAY;
        return 0;
    }

    // Validate buffer and layout
    if (!layout)
        return fail(NX3D_ERROR_INVALID_ARGUMENT);
    if (!buffer->is_allocated || !layout->is_allocated)
        return fail(NX3D_ERROR_INVALID_OPERATION);

    // Validate vertex data
    const NX3D_ELEMENT *elements = layout->elements;
    const char *vertex_data = (const char *)buffer->__p;
    uint32_t max_vertex_count = buffer->length / layout->stride;
    for (uint32_t i = 0; i < max_vertex_count; i++)
    {
        uint32_t vertex_offset = layout->stride * i;
        for (uint32_t j = 0; j < layout->count; j++)
        {
            NX3D_ELEMENT e = elements[j];
            for (uint32_t k = 0; k < e.count; k++)
            {
                uint32_t element_offset = k * e.component_bytes;
                const void *element_data = vertex_data + vertex_offset + element_offset;
                switch (e.nv_type)
                {
                    case NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_UB_D3D:
                        break;
                    case NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S1:
                        break;
                    case NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F:
                        const float f = *(const float *)element_data;
                        if (!isfinite(f))
                            return fail(NX3D_ERROR_INVALID_OPERATION);
                        break;
                    case NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_UB_OGL:
                        break;
                    case NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_S32K:
                        break;
                    case NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_CMP:
                        break;
                }
            }
        }
    }

    state.vertex_validated = TRUE;
    if (state.vertex_buffer != buffer || state.vertex_layout != layout)
        state.dirty |= NX3D_DIRTY_VERTEX_ARRAY;
    state.vertex_buffer = buffer;
    state.vertex_layout = layout;
    return 0;
}

int nx3d_bind_index_buffer(NX3D_BUFFER *buffer, NX3D_INDEX_DATA_TYPE index_type)
{
    if (!state.initialized)
        return fail(NX3D_ERROR_NOT_INITIALIZED);
    if (!state.frame)
        return fail(NX3D_ERROR_INVALID_OPERATION);

    if (!buffer)
    {
        state.index_buffer = NULL;
        state.index_type = NX3D_UINT16;
        return 0;
    }

    if (!buffer->is_allocated)
        return fail(NX3D_ERROR_INVALID_OPERATION);
    state.index_buffer = buffer;
    state.index_type = index_type;
    return 0;
}

int nx3d_bind_texture(NX3D_TEXTURE *texture, NX3D_SAMPLER_DESC *sampler, NX3D_TEXTURE_SLOT texture_slot)
{
    if (texture_slot < NX3D_TEXTURE_SLOT_0 || texture_slot > NX3D_TEXTURE_SLOT_3)
        return fail(NX3D_ERROR_INVALID_ARGUMENT);
    if (!state.frame)
        return fail(NX3D_ERROR_INVALID_OPERATION);

    int slot = (int)texture_slot;
    if (!texture || !sampler)
    {
        state.tex_state[slot].t = NULL;
        memset(&state.tex_state[slot].s, 0, sizeof(NX3D_SAMPLER_DESC));
        state.tex_state[slot].palette = NULL;
        state.dirty |= NX3D_DIRTY_TEXTURES | NX3D_DIRTY_COMBINERS;
        return 0;
    }
    
    state.tex_state[slot].t = texture;
    memcpy(&state.tex_state[slot].s, sampler, sizeof(NX3D_SAMPLER_DESC));
    state.dirty |= NX3D_DIRTY_TEXTURES | NX3D_DIRTY_COMBINERS;
    return 0;
}

int nx3d_set_mode(NX3D_TRANSFORM_MODE mode)
{
    if (!state.initialized) return fail(NX3D_ERROR_NOT_INITIALIZED);
    state.transform_mode = mode;
    state.dirty |= NX3D_DIRTY_TRANSFORM;
    return 0;
}

int nx3d_enable(NX3D_ENABLE_OPTIONS option, uint32_t value)
{
    void *opt_val_ptr;
    switch (option)
    {
        case NX3D_ENABLE_VERTEX_COLOR:
            opt_val_ptr = &state.vertex_color;
            break;
        default:
            return fail(NX3D_ERROR_INVALID_ARGUMENT);
    }
    *(uint32_t *)opt_val_ptr = value;
    state.dirty |= NX3D_DIRTY_COMBINERS;
    return 0;
}

int nx3d_set_winding(NX3D_FACE_WINDING_ORDER winding_order)
{
    if (winding_order < NX3D_WINDING_CW || winding_order > NX3D_WINDING_CCW)
        return fail(NX3D_ERROR_INVALID_ARGUMENT);
    state.winding_order = winding_order;
    state.dirty |= NX3D_DIRTY_FACE;
    return 0;
}

int nx3d_set_cull_face(NX3D_CULL_FACE cull_face)
{
    if (cull_face < NX3D_CULL_NONE || cull_face > NX3D_CULL_FRONT)
        return fail(NX3D_ERROR_INVALID_ARGUMENT);
    state.cull_face = cull_face;
    state.dirty |= NX3D_DIRTY_FACE;
    return 0;
}

int nx3d_set_texture_pixel_data(NX3D_TEXTURE *texture, const void *pixel_data, uint32_t length)
{
    if (!texture) return fail(NX3D_ERROR_INVALID_ARGUMENT);
    if (texture->pixel_data)
        memset(texture->pixel_data, 0, texture->length);

    if (!pixel_data || length == 0)
        return 0;

    memcpy(texture->pixel_data, pixel_data, length);
    return 0;
}

void nx3d_sampler_default(NX3D_SAMPLER_DESC *desc)
{
    if (!desc) return;
    memset(desc, 0, sizeof(NX3D_SAMPLER_DESC));
    desc->wrap_u = NX3D_SAMPLER_WRAP_CLAMP;
    desc->wrap_v = NX3D_SAMPLER_WRAP_CLAMP;
    desc->wrap_w = NX3D_SAMPLER_WRAP_CLAMP;
    desc->kernel = NX3D_SAMPLER_KERNEL_GAUSSIAN;
    desc->min_filter = NX3D_SAMPLER_MIN_TENT_LOD0;
    desc->mag_filter = NX3D_SAMPLER_MAG_TENT_LOD0;
    desc->anisotropy_level = NX3D_SAMPLER_ANISOTROPY_1X;
}

void nx3d_generate_normals(NX3D_VERTEX *vertices, const void *triangles, NX3D_INDEX_DATA_TYPE triangle_type, uint32_t triangle_count)
{
    if (vertices == NULL ||
        triangles == NULL ||
        (triangle_type != NX3D_UINT16 &&
         triangle_type != NX3D_UINT32) ||
        (triangle_count > UINT16_MAX
            && triangle_type == NX3D_UINT16))
        return;

    uint32_t i[3];
    NX3D_VERTEX v[3];
    const uint16_t *tri_16 = (const uint16_t *)triangles;
    const uint32_t *tri_32 = (const uint32_t *)triangles;
    for (uint32_t t = 0; t < triangle_count; t++)
    {
        uint32_t a = t * 3;
        for (uint32_t j = 0; j < 3; j++)
        {
            switch (triangle_type)
            {
                case NX3D_UINT16: i[j] = tri_16[a + j]; break;
                case NX3D_UINT32: i[j] = tri_32[a + j]; break;
            }
            v[j] = vertices[i[j]];
        }

        float ux = v[1].position[0] - v[0].position[0];
        float uy = v[1].position[1] - v[0].position[1];
        float uz = v[1].position[2] - v[0].position[2];
        float vx = v[2].position[0] - v[0].position[0];
        float vy = v[2].position[1] - v[0].position[1];
        float vz = v[2].position[2] - v[0].position[2];
        float nx = uy * vz - uz * vy;
        float ny = uz * vx - ux * vz;
        float nz = ux * vy - uy * vx;
        float length_squared = nx * nx + ny * ny + nz * nz;
        if (length_squared > 0.0f)
        {
            float inverse_length = 1.0f / sqrtf(length_squared);
            nx *= inverse_length;
            ny *= inverse_length;
            nz *= inverse_length;
        }
        for (uint32_t j = 0; j < 3; j++)
        {
            v[j].normal[0] = nx;
            v[j].normal[1] = ny;
            v[j].normal[2] = nz;
        }

        for (uint32_t j = 0; j < 3; j++)
            vertices[i[j]] = v[j];
    }
}
