#include "world/frontera.h"
#include "world/ciudad.h"
#include "engine/cubo.h"
#include <math.h>

#define PLAYA   6.0f    // franja de arena afuera del mapa
#define MAR     80.0f   // ancho del mar
#define LOSA    20.0f   // tamaño de cada cuadro de mar/arena
#define VISTA   120.0f  // hasta dónde se dibuja

static float cerca_borde(float v, float mitad) { return fabsf(v) - mitad; }

// Dibuja un rectángulo (x0..x1, z0..z1) cortado en cuadros chicos, solo los cercanos
static void franja(float x0, float x1, float z0, float z1, float px, float pz,
                   float r, float g, float b, float variacion) {
    int i = 0;
    for (float x = x0; x < x1 - 0.01f; x += LOSA, i++) {
        float w = fminf(LOSA, x1 - x);
        int j = 0;
        for (float z = z0; z < z1 - 0.01f; z += LOSA, j++) {
            float d = fminf(LOSA, z1 - z);
            float cx = x + w * 0.5f, cz = z + d * 0.5f;
            if (fabsf(cx - px) > VISTA || fabsf(cz - pz) > VISTA) continue;
            float k = (((i + j) & 1) == 0) ? 1.0f : 1.0f - variacion;
            baldosa(cx, 0.0f, cz, w, d, r * k, g * k, b * k);
        }
    }
}

void frontera_dibujar_suelo(float px, float pz) {
    float mx = ciudad_ancho() * CIUDAD_TAM * 0.5f;
    float mz = ciudad_alto()  * CIUDAD_TAM * 0.5f;
    float ax = mx + PLAYA, az = mz + PLAYA;       // borde de la arena
    float bx = ax + MAR,   bz = az + MAR;         // borde del mar

    // Mar: cuatro lados (arriba y abajo van completos; izquierda y derecha, entre medio)
    franja(-bx, bx, -bz, -az, px, pz, 0.15f, 0.45f, 0.85f, 0.10f);
    franja(-bx, bx,  az,  bz, px, pz, 0.15f, 0.45f, 0.85f, 0.10f);
    franja(-bx, -ax, -az, az, px, pz, 0.15f, 0.45f, 0.85f, 0.10f);
    franja( ax,  bx, -az, az, px, pz, 0.15f, 0.45f, 0.85f, 0.10f);
    // Arena
    franja(-ax, ax, -az, -mz, px, pz, 0.92f, 0.84f, 0.55f, 0.06f);
    franja(-ax, ax,  mz,  az, px, pz, 0.92f, 0.84f, 0.55f, 0.06f);
    franja(-ax, -mx, -mz, mz, px, pz, 0.92f, 0.84f, 0.55f, 0.06f);
    franja( mx,  ax, -mz, mz, px, pz, 0.92f, 0.84f, 0.55f, 0.06f);
    (void)cerca_borde;
}

// Un número "al azar" siempre igual para el mismo i (así las montañas no cambian)
static unsigned azar(unsigned i) {
    i = i * 2654435761u + 12345u;
    i ^= i >> 15;
    i *= 2246822519u;
    i ^= i >> 13;
    return i;
}

static void montana(float x, float z, unsigned s) {
    float w = 24.0f + (float)(azar(s) % 16);
    float h = 26.0f + (float)(azar(s + 7) % 40);
    int tipo = (int)(azar(s + 3) % 3);
    float r = (tipo == 0) ? 0.45f : (tipo == 1) ? 0.35f : 0.30f;
    float g = (tipo == 0) ? 0.45f : (tipo == 1) ? 0.50f : 0.55f;
    float b = (tipo == 0) ? 0.50f : (tipo == 1) ? 0.32f : 0.35f;
    caja(x, h * 0.5f, z, w, h, w, r, g, b);
    caja(x, h * 0.5f, z, w * 0.6f, h * 1.25f, w * 0.6f, r * 0.9f, g * 0.9f, b * 0.9f);
    if (h > 45.0f) caja(x, h * 1.25f, z, w * 0.45f, 3.0f, w * 0.45f, 0.95f, 0.95f, 1.0f); // nieve
}

void frontera_dibujar(float px, float pz) {
    float mx = ciudad_ancho() * CIUDAD_TAM * 0.5f;
    float mz = ciudad_alto()  * CIUDAD_TAM * 0.5f;

    // Valla roja y blanca justo afuera del mapa
    int n = 0;
    for (float x = -mx; x < mx; x += 4.0f, n++) {
        float len = fminf(4.0f, mx - x), cx = x + len * 0.5f;
        float r = (n & 1) ? 0.95f : 0.85f, g = (n & 1) ? 0.95f : 0.15f, b = (n & 1) ? 0.95f : 0.15f;
        if (fabsf(cx - px) < 70.0f) {
            if (fabsf(-mz - 0.6f - pz) < 70.0f) caja(cx, 0.6f, -mz - 0.6f, len, 1.2f, 0.5f, r, g, b);
            if (fabsf( mz + 0.6f - pz) < 70.0f) caja(cx, 0.6f,  mz + 0.6f, len, 1.2f, 0.5f, r, g, b);
        }
    }
    n = 0;
    for (float z = -mz; z < mz; z += 4.0f, n++) {
        float len = fminf(4.0f, mz - z), cz = z + len * 0.5f;
        float r = (n & 1) ? 0.95f : 0.85f, g = (n & 1) ? 0.95f : 0.15f, b = (n & 1) ? 0.95f : 0.15f;
        if (fabsf(cz - pz) < 70.0f) {
            if (fabsf(-mx - 0.6f - px) < 70.0f) caja(-mx - 0.6f, 0.6f, cz, 0.5f, 1.2f, len, r, g, b);
            if (fabsf( mx + 0.6f - px) < 70.0f) caja( mx + 0.6f, 0.6f, cz, 0.5f, 1.2f, len, r, g, b);
        }
    }

    // Montañas al otro lado del mar, rodeando todo
    float dx = mx + PLAYA + MAR + 18.0f, dz = mz + PLAYA + MAR + 18.0f;
    unsigned s = 0;
    for (float x = -dx; x <= dx; x += 26.0f, s++) {
        if (fabsf(x - px) < 230.0f) {
            if (fabsf(-dz - pz) < 230.0f) montana(x, -dz, s);
            if (fabsf( dz - pz) < 230.0f) montana(x,  dz, s + 1000);
        }
    }
    for (float z = -dz + 26.0f; z < dz; z += 26.0f, s++) {
        if (fabsf(z - pz) < 230.0f) {
            if (fabsf(-dx - px) < 230.0f) montana(-dx, z, s + 2000);
            if (fabsf( dx - px) < 230.0f) montana( dx, z, s + 3000);
        }
    }
}
