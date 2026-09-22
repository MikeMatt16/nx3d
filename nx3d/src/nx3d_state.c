#include "nx3d_state.h"
#include "nx3d_types.h"
#include <math.h>
#include <stdlib.h>
#include <pbkit/pbkit.h>
#include <string.h>

nx3d_state state = { 0 };

static const NX3D_MATRIX identity =
{
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

static void multiply(float *out, const float *a, const float *b)
{
    NX3D_MATRIX result;
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r) {
            result[c * 4 + r] = 0;
            for (int k = 0; k < 4; ++k)
                result[c * 4 + r] += a[k * 4 + r] * b[c * 4 + k];
        }
    memcpy(out, result, sizeof(result));
}

/* NV097 matrix methods consume rows. Public matrices contain columns. */
static uint32_t *push_matrix(uint32_t *p, uint32_t method, const float *m)
{
    for (int r = 0; r < 4; ++r)
        p = pb_push4f(p, method + r * 16, m[r], m[4+r], m[8+r], m[12+r]);
    return p;
}

uint32_t *begin_commands(void)
{
    if (state.command_budget >= 8192)
    {
        while (pb_busy()) {}
        pb_reset();
        state.command_budget = 0;
    }
    state.command_budget += 128;
    return pb_begin();
}

static void apply_transform_mode(void)
{
    uint32_t execution_mode = state.transform_mode ?
        NX3D_MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_MODE,
            NV097_SET_TRANSFORM_EXECUTION_MODE_MODE_PROGRAM)
        : NX3D_MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_MODE,
            NV097_SET_TRANSFORM_EXECUTION_MODE_MODE_FIXED);

    while (pb_busy());
    uint32_t *p = begin_commands();
    p = pb_push1(p, NV097_SET_TRANSFORM_EXECUTION_MODE, execution_mode
        | NX3D_MASK(NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE,
            NV097_SET_TRANSFORM_EXECUTION_MODE_RANGE_MODE_PRIV));
    pb_end(p);
}

void setup_pipeline(void)
{
    apply_transform_mode();
    uint32_t *p = begin_commands();
    p = pb_push1(p, NV097_SET_TRANSFORM_PROGRAM_CXT_WRITE_EN, 0);
    p = pb_push1(p, NV097_SET_SKIN_MODE, NV097_SET_SKIN_MODE_OFF);
    p = pb_push1(p, NV097_SET_LIGHTING_ENABLE, 0);
    p = pb_push1(p, NV097_SET_SPECULAR_ENABLE, 0);
    p = pb_push1(p, NV097_SET_FOG_ENABLE, 0);
    p = pb_push1(p, NV097_SET_CULL_FACE_ENABLE, 0);
    p = pb_push1(p, NV097_SET_BLEND_ENABLE, 0);
    p = pb_push1(p, NV097_SET_ALPHA_TEST_ENABLE, 0);
    p = pb_push1(p, NV097_SET_STENCIL_TEST_ENABLE, 0);
    p = pb_push1(p, NV097_SET_SHADE_MODEL, NV097_SET_SHADE_MODEL_SMOOTH);
    p = pb_push1(p, NV097_SET_COLOR_MASK, 0x01010101);
    p = pb_push1(p, NV097_SET_DEPTH_FUNC, NV097_SET_DEPTH_FUNC_V_LESS);
    p = pb_push1(p, NV097_SET_ZMIN_MAX_CONTROL, NV097_SET_ZMIN_MAX_CONTROL_CULL_NEAR_FAR);
    p = pb_push1(p, NV097_SET_CLIP_MIN, 0);
    /* IEEE-754 encoding of 16777215.0f, the fixed-point Z24 maximum. */
    p = pb_push1(p, NV097_SET_CLIP_MAX, 0x4b7fffff);
    p = pb_push1(p, NV097_SET_SHADER_STAGE_PROGRAM, 0);
    for (int i = 0; i < NX3D_MAX_TEX; ++i)
        p = pb_push1(p, NV097_SET_TEXTURE_CONTROL0 + i * 64, 0);
    /* Final RGB = D = primary color (register 4); alpha = primary alpha.
     * The single general stage discards its outputs. */
    p = pb_push1(p, NV097_SET_COMBINER_CONTROL, 1);
    p = pb_push1(p, NV097_SET_COMBINER_COLOR_ICW, 0);
    p = pb_push1(p, NV097_SET_COMBINER_COLOR_OCW, 0);
    p = pb_push1(p, NV097_SET_COMBINER_ALPHA_ICW, 0);
    p = pb_push1(p, NV097_SET_COMBINER_ALPHA_OCW, 0);
    p = pb_push1(p, NV097_SET_COMBINER_SPECULAR_FOG_CW0, 4);
    p = pb_push1(p, NV097_SET_COMBINER_SPECULAR_FOG_CW1,
                 (4 << 8) | NV097_SET_COMBINER_SPECULAR_FOG_CW1_G_ALPHA);
    for (int i = 0; i < 16; ++i)
        p = pb_push1(p, NV097_SET_VERTEX_DATA_ARRAY_FORMAT + i * 4,
                     NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F);
    pb_end(p);
}

static void apply_face_settings(void)
{
    uint32_t front_face, *p;
    uint32_t cull_face = state.cull_face == NX3D_CULL_BACK
        ? NV097_SET_CULL_FACE_V_BACK : NV097_SET_CULL_FACE_V_FRONT;

    switch (state.winding_order)
    {
        case NX3D_WINDING_CW:
            front_face = NV097_SET_FRONT_FACE_V_CW;
            break;
        case NX3D_WINDING_CCW:
            front_face = NV097_SET_FRONT_FACE_V_CCW;
            break;
    }

    p = begin_commands();
    p = pb_push1(p, NV097_SET_FRONT_FACE, front_face);
    if (state.cull_face == NX3D_CULL_NONE)
    {
        p = pb_push1(p, NV097_SET_CULL_FACE_ENABLE, 0);
        pb_end(p);
        return;
    }

    p = pb_push1(p, NV097_SET_CULL_FACE, cull_face);
    p = pb_push1(p, NV097_SET_CULL_FACE_ENABLE, 1);
    pb_end(p);
}

static void apply_matrices(void)
{
    NX3D_MATRIX viewport, composite;
    memcpy(viewport, identity, sizeof(viewport));
    viewport[0] = state.vw * 0.5f;
    viewport[5] = state.vh * -0.5f;
    viewport[10] = 16777215.0f * 0.5f;
    viewport[12] = state.x + state.vw * 0.5f;
    viewport[13] = state.y + state.vh * 0.5f;
    viewport[14] = 16777215.0f * 0.5f;
    multiply(composite, state.projection, state.modelview);
    multiply(composite, viewport, composite);

    while (pb_busy());
    uint32_t *p = begin_commands();
    p = push_matrix(p, NV097_SET_MODEL_VIEW_MATRIX, state.modelview);
    p = push_matrix(p, NV097_SET_TEXTURE_MATRIX, state.tex_matrix);
    /* Fixed-function position transform includes the viewport. */
    p = push_matrix(p, NV097_SET_COMPOSITE_MATRIX, composite);
    pb_end(p);
}

static void apply_viewport(void)
{
    uint32_t *p = begin_commands();
    p = pb_push4f(p, NV097_SET_VIEWPORT_SCALE, 0, 0, 0, 0);
    p = pb_push4f(p, NV097_SET_VIEWPORT_OFFSET, 0.53125f, 0.53125f, 0, 0);
    p = pb_push1(p, NV097_SET_WINDOW_CLIP_TYPE, 0);
    for (int i = 0; i < 8; ++i) {
        p = pb_push1(p, NV097_SET_WINDOW_CLIP_HORIZONTAL + i * 4,
                     (uint32_t)state.x | ((uint32_t)(state.x + state.vw - 1) << 16));
        p = pb_push1(p, NV097_SET_WINDOW_CLIP_VERTICAL + i * 4,
                     (uint32_t)state.y | ((uint32_t)(state.y + state.vh - 1) << 16));
    }
    pb_end(p);
}

static void apply_depth(void)
{
    uint32_t *p = begin_commands();
    p = pb_push1(p, NV097_SET_DEPTH_TEST_ENABLE, state.depth_test);
    p = pb_push1(p, NV097_SET_DEPTH_MASK, state.depth_write);
    pb_end(p);
}

static void apply_vertex_data_array(void)
{
    while (pb_busy());
    uint32_t *p = begin_commands();
    for (int i = 0; i < 16; i++)
        p = pb_push1(p, NV097_SET_VERTEX_DATA_ARRAY_FORMAT + i * 4,
                     NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE_F);

    if (!state.vertex_validated)
    {    
        pb_end(p);
        return;
    }

    for (uint32_t i = 0; i < state.vertex_layout->count; i++)
    {
        NX3D_ELEMENT e = state.vertex_layout->elements[i];
        uint32_t data = (uint32_t)state.vertex_buffer->buffer & 0x03ffffff;
        uint32_t attrib_offset = data + e.aligned_offset;
        p = pb_push1(p, NV097_SET_VERTEX_DATA_ARRAY_FORMAT + (e.v * 4),
            NX3D_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_TYPE, e.nv_type)
            | NX3D_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_SIZE, e.count)
            | NX3D_MASK(NV097_SET_VERTEX_DATA_ARRAY_FORMAT_STRIDE, state.vertex_layout->stride));
        p = pb_push1(p, NV097_SET_VERTEX_DATA_ARRAY_OFFSET + (e.v * 4), attrib_offset);
    }
    pb_end(p);
}

static void apply_textures(void)
{
    uint32_t *p;
    uint32_t stage_program = 0;
    for (uint32_t i = 0; i < NX3D_MAX_TEX; i++)
    {
        NX3D_TEXTURE *t = state.tex_state[i].t;
        NX3D_SAMPLER_DESC *s = &state.tex_state[i].s;
        while (pb_busy());
        p = begin_commands();

        if (!t)
        {
            p = pb_push1(p, NV20_TCL_PRIMITIVE_3D_TX_OFFSET(i), 0);
            p = pb_push1(p, NV20_TCL_PRIMITIVE_3D_TX_ENABLE(i), 0);
            pb_end(p);
            continue;
        }

        const float bias = fminf(fmaxf(s->lod_bias, -16.0f), 4095.0f / 256.0f);
        const int32_t bias_fixed = (int32_t)roundf(bias * 256.0f);
        const int32_t min_lod = (int32_t)(s->min_lod_clamp * 256.0f);
        const int32_t max_lod = (int32_t)(s->max_lod_clamp * 256.0f);
        const uint32_t wrap =
            ((s->wrap_w & 0xFF) << 16)
            | ((s->wrap_v & 0xFF) << 8)
            | (s->wrap_u & 0xFF);
        const uint32_t filter =
            NX3D_MASK(NV097_SET_TEXTURE_FILTER_MIPMAP_LOD_BIAS, bias_fixed)
            | NX3D_MASK(0x0000E000, s->kernel)
            | NX3D_MASK(NV097_SET_TEXTURE_FILTER_MIN, s->min_filter)
            | NX3D_MASK(NV097_SET_TEXTURE_FILTER_MAG, s->mag_filter);
        const uint32_t enable_flags =
            NX3D_MASK(NV097_SET_TEXTURE_CONTROL0_ENABLE, TRUE)
            | NX3D_MASK(NV097_SET_TEXTURE_CONTROL0_ANISOTROPY, (uint32_t)s->anisotropy_level)
            | NX3D_MASK(NV097_SET_TEXTURE_CONTROL0_MIN_LOD_CLAMP, min_lod)
            | NX3D_MASK(NV097_SET_TEXTURE_CONTROL0_MAX_LOD_CLAMP, max_lod);

        p = pb_push1(p, NV20_TCL_PRIMITIVE_3D_TX_OFFSET(i), (uint32_t)t->pixel_data & 0x03ffffff);
        p = pb_push1(p, NV20_TCL_PRIMITIVE_3D_TX_FORMAT(i), t->format);
        p = pb_push1(p, NV20_TCL_PRIMITIVE_3D_TX_WRAP(i), wrap);
        p = pb_push1(p, NV20_TCL_PRIMITIVE_3D_TX_NPOT_PITCH(i), t->pitch << 16);
        p = pb_push1(p, NV20_TCL_PRIMITIVE_3D_TX_FILTER(i), filter);
        p = pb_push1(p, NV20_TCL_PRIMITIVE_3D_TX_NPOT_SIZE(i), (t->width << 16) | t->height);
        p = pb_push1(p, NV20_TCL_PRIMITIVE_3D_TX_ENABLE(i), enable_flags);
        pb_end(p);
        stage_program |= 1u << (i * 5);
    }

    p = begin_commands();
    p = pb_push1(p, NV097_SET_SHADER_STAGE_PROGRAM, stage_program);

    pb_end(p);
}

static void apply_combiners(void)
{
    uint32_t *p = begin_commands();
    uint32_t combiner_count = 0;
    for (uint32_t i = 0; i < NX3D_MAX_TEX; i++)
    {
        if (!state.tex_state[i].t)
            continue;

        uint32_t texture = 8u + i;
        uint32_t previous = combiner_count
            ? 0x0C                          // previous result: spare0.rgb
            : (state.vertex_color ? 0x04    // primary vertex color.rgb
                : 0x20);                    // inverted zero: vec3(1)
        uint32_t offset = combiner_count * 4;
        p = pb_push1(p, NV097_SET_COMBINER_COLOR_ICW + offset,
                     (texture << 24) | (previous << 16));
        p = pb_push1(p, NV097_SET_COMBINER_COLOR_OCW + offset,
                     NX3D_MASK(NV097_SET_COMBINER_COLOR_OCW_AB_DST, 12u));

        uint32_t alpha_previous = combiner_count
            ? 0x1C                          // spare0.alpha
            : (state.vertex_color ? 0x14    // primary vertex color.alpha
                : 0x30);                    // inverted zero: 1

        p = pb_push1(p, NV097_SET_COMBINER_ALPHA_ICW + offset,
                     ((texture | 0x10u) << 24) | (alpha_previous << 16));
        p = pb_push1(p, NV097_SET_COMBINER_ALPHA_OCW + offset,
                     NX3D_MASK(NV097_SET_COMBINER_ALPHA_OCW_AB_DST, 12u));
        ++combiner_count;
    }

    /* Keep one discard stage when untextured, as in setup_pipeline(). */
    if (!combiner_count)
    {
        p = pb_push1(p, NV097_SET_COMBINER_COLOR_ICW, 0);
        p = pb_push1(p, NV097_SET_COMBINER_COLOR_OCW, 0);
        p = pb_push1(p, NV097_SET_COMBINER_ALPHA_ICW, 0);
        p = pb_push1(p, NV097_SET_COMBINER_ALPHA_OCW, 0);
    }
    p = pb_push1(p, NV097_SET_COMBINER_CONTROL,
                 combiner_count ? combiner_count : 1);
    /* Final RGB = D; final alpha = G.alpha. */
    uint32_t result = combiner_count
                    ? 0x0C
                    : (state.vertex_color ? 0x04
                    : 0x20);
    p = pb_push1(p, NV097_SET_COMBINER_SPECULAR_FOG_CW0, result);
    p = pb_push1(p, NV097_SET_COMBINER_SPECULAR_FOG_CW1,
                 (result << 8) | NV097_SET_COMBINER_SPECULAR_FOG_CW1_G_ALPHA);
    pb_end(p);
}

/* Each group owns its GPU registers, so updating one cannot invalidate another.
 * Keep the dispatch here alongside the uploads when adding new state groups. */
void apply_state(void)
{
    const uint32_t dirty = state.dirty;
    if (!dirty) return;
    if (dirty & NX3D_DIRTY_TRANSFORM) apply_transform_mode();
    if (dirty & NX3D_DIRTY_FACE) apply_face_settings();
    if (dirty & NX3D_DIRTY_MATRICES) apply_matrices();
    if (dirty & NX3D_DIRTY_VIEWPORT) apply_viewport();
    if (dirty & NX3D_DIRTY_DEPTH) apply_depth();
    if (dirty & NX3D_DIRTY_VERTEX_ARRAY) apply_vertex_data_array();
    if (dirty & NX3D_DIRTY_TEXTURES) apply_textures();
    if (dirty & NX3D_DIRTY_COMBINERS) apply_combiners();
    state.dirty &= ~dirty;
}
