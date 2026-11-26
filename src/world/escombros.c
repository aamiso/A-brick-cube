#include "world/escombros.h"
#include "engine/cubo.h"
#include <math.h>

#define MAX_ESC 450
typedef struct { float x, y, z, vx, vy, vz, tam, r, g, b; int vida; } Esc;
static Esc esc[MAX_ESC];
static int sig = 0;
static unsigned semilla = 777u;
static float azar01(void) { semilla = semilla * 1664525u + 1013904223u; return (semilla >> 8 & 0xffff) / 65535.0f; }

void escombros_crear(float x, float y, float z, float vx, float vz, unsigned color, int n, float tam, float fuerza) {
    for (int i = 0; i < n; i++) {
        Esc *e = &esc[sig]; sig = (sig + 1) % MAX_ESC;
        float a = azar01() * 6.2832f, f = fuerza * (0.3f + azar01());
        e->x = x + (azar01() - 0.5f) * tam * 3.0f; e->y = y + azar01() * tam * 2.0f; e->z = z + (azar01() - 0.5f) * tam * 3.0f;
        e->vx = vx * 0.8f + sinf(a) * f; e->vz = vz * 0.8f + cosf(a) * f;
        e->vy = 0.06f + azar01() * 0.14f * (0.5f + fuerza * 2.0f);
        e->tam = tam * (0.7f + azar01() * 0.6f);
        float k = 0.8f + azar01() * 0.3f;
        e->r = ((color >> 16) & 255) / 255.0f * k; e->g = ((color >> 8) & 255) / 255.0f * k; e->b = (color & 255) / 255.0f * k;
        if (e->r > 1) e->r = 1; if (e->g > 1) e->g = 1; if (e->b > 1) e->b = 1;
        e->vida = 240 + (int)(azar01() * 120);
    }
}

void escombros_actualizar(void) {
    for (int i = 0; i < MAX_ESC; i++) {
        Esc *e = &esc[i];
        if (e->vida <= 0) continue;
        e->vida--;
        e->vy -= 0.012f;
        e->x += e->vx; e->y += e->vy; e->z += e->vz;
        float suelo = e->tam * 0.5f;
        if (e->y < suelo) {
            e->y = suelo;
            e->vy = -e->vy * 0.35f; if (fabsf(e->vy) < 0.012f) e->vy = 0.0f;
            e->vx *= 0.7f; e->vz *= 0.7f;
        }
    }
}

void escombros_dibujar(float px, float pz) {
    cubo_activar();
    for (int i = 0; i < MAX_ESC; i++) {
        const Esc *e = &esc[i];
        if (e->vida <= 0) continue;
        if (fabsf(e->x - px) > 60.0f || fabsf(e->z - pz) > 60.0f) continue;
        float t = e->tam * (e->vida < 40 ? e->vida / 40.0f : 1.0f);
        caja(e->x, e->y, e->z, t, t, t, e->r, e->g, e->b);
    }
}
