#include "core/entrada.h"
#include <math.h>

float leer_stick(unsigned char valor) {
    float v = ((float)valor - 128.0f) / 128.0f;
    if (fabsf(v) < 0.15f) return 0.0f; // los sticks nunca quedan en 0 exacto
    return v;
}
