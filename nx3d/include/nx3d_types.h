#ifndef NX3D_TYPES_H
#define NX3D_TYPES_H

#include <stdint.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef struct NX3D_VERTEX_ELEMENT_LAYOUT
    NX3D_VERTEX_ELEMENT_LAYOUT;
typedef struct NX3D_BUFFER
    NX3D_BUFFER;
typedef struct NX3D_COLOR_PALETTE
    NX3D_COLOR_PALETTE;
typedef struct NX3D_TEXTURE
    NX3D_TEXTURE;

typedef uint32_t R11G11B10;

#ifdef __cplusplus
extern "C" {
#endif

/* nyi */
R11G11B10 vec3_to_r11g11b10(const float *f);
/* nyi */
void r11g11b10_to_vec3(const R11G11B10 c, float f[3]);

#ifdef __cplusplus
}
#endif

#endif
