#include "world/reloj.h"
#include "ui/ui.h"
#include <math.h>

static float hora = 9.0f;

void reloj_actualizar(void) { hora += 24.0f / (480.0f * 60.0f); if (hora >= 24.0f) hora -= 24.0f; }
float reloj_hora(void) { return hora; }

static float sol(void) { return sinf((hora - 6.0f) * 3.14159265f / 12.0f); }   // >0 de día
static float suave(float a, float b, float x) { float t = (x - a) / (b - a); if (t < 0) t = 0; if (t > 1) t = 1; return t * t * (3 - 2 * t); }

void reloj_cielo(float *r, float *g, float *b) {
    float s = sol(), luz = suave(-0.15f, 0.25f, s);
    float cr = 0.03f + (0.50f - 0.03f) * luz, cg = 0.04f + (0.75f - 0.04f) * luz, cb = 0.12f + (1.00f - 0.12f) * luz;
    float ocaso = expf(-(s / 0.2f) * (s / 0.2f)) * 0.5f;                      // naranja al amanecer y atardecer
    *r = cr + (0.95f - cr) * ocaso; *g = cg + (0.50f - cg) * ocaso; *b = cb + (0.30f - cb) * ocaso;
}

void reloj_filtro(void) {
    float s = sol(), luz = suave(-0.15f, 0.25f, s);
    int a = (int)((1.0f - luz) * 150.0f);
    if (a > 0) ui_rect(0, 0, 960, 544, 0x0a1040u << 8 | (unsigned)a);
    int o = (int)(expf(-(s / 0.2f) * (s / 0.2f)) * 45.0f);
    if (o > 0) ui_rect(0, 0, 960, 544, 0xff7a30u << 8 | (unsigned)o);
}
