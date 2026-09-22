#include "nx3d_types.h"
#include <string.h>

R11G11B10 vec3_to_r11g11b10(const float *f)
{
    float rf = f[0] < 0 ? 0 : f[0] > 1 ? 1 : f[0];
    float gf = f[1] < 0 ? 0 : f[1] > 1 ? 1 : f[1];
    float bf = f[2] < 0 ? 0 : f[2] > 1 ? 1 : f[2];
    uint16_t r = (uint16_t)(rf * (float)0x7FF);
    uint16_t g = (uint16_t)(gf * (float)0x7FF);
    uint16_t b = (uint16_t)(bf * (float)0x3FF);
    return (uint32_t)((r << 21) | (g << 10 ) | b);
}

void r11g11b10_to_vec3(const R11G11B10 c, float f[3])
{
    float r[3] = 
    {
        (float)((c >> 21) & 0x7FF) / (float)0x7FF,
        (float)((c >> 10) & 0x7FF) / (float)0x7FF,
        (float)((c) & 0x3FF) / (float)0x3FF,
    };
    memcpy(f, r, 12);
}
