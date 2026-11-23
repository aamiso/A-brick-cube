#include "world/terreno.h"
#include "world/ciudad.h"
#include "world/lugares.h"
#include "engine/malla.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define CH         16        // cada pedazo mide 16 x 16 celdas
#define MAX_CH     26        // hasta 26 x 26 pedazos (416 celdas)
#define RADIO_ARMA 46.0f     // se arman los pedazos que están más cerca que esto
#define RADIO_BORRA 120.0f   // y se borran los que están más lejos que esto

static Malla pedazos[MAX_CH][MAX_CH];
static char  armado[MAX_CH][MAX_CH];

static unsigned azar(int a, int b) {
    unsigned h = (unsigned)a * 2246822519u + (unsigned)b * 3266489917u + 374761393u;
    h = (h ^ (h >> 15)) * 2654435761u;
    return h ^ (h >> 13);
}

static int es_calle(int c, int f) { return ciudad_tipo_en(c, f) == '='; }

// Cuántas celdas seguidas de calle hay desde (c,f) en una dirección (se corta en 10)
static int corrida(int c, int f, int dc, int df) {
    int n = 0;
    while (n < 10 && es_calle(c + dc * (n + 1), f + df * (n + 1))) n++;
    return n;
}

static float g_y = 0.0f;   // altura de la capa que se está armando
static void rect(Malla *m, float x0, float z0, float x1, float z1, float r, float g, float b) {
    malla_tapa(m, (x0 + x1) * 0.5f, g_y, (z0 + z1) * 0.5f, x1 - x0, z1 - z0, r, g, b);
}

static void celda_base(Malla *m, int c, int f) {
    char t = ciudad_tipo_en(c, f);
    g_y = (t == '=' || t == 'S' || t == 'W') ? 0.06f : -0.015f;
    float x0 = ciudad_centro_x(c) - 0.5f, z0 = ciudad_centro_z(f) - 0.5f;
    int par = (c + f) & 1;
    unsigned q = azar(c, f);
    float r, g, b;
    if (t == '=') {
        float v = par ? 0.30f : 0.285f;
        r = v; g = v + 0.01f; b = v + 0.04f;
    } else if (t == 'S') {
        float v = par ? 0.73f : 0.69f;
        r = v; g = v - 0.01f; b = v - 0.04f;
    } else if (t == 'W') {
        r = par ? 0.18f : 0.16f; g = par ? 0.48f : 0.45f; b = par ? 0.86f : 0.83f;
        if (q % 9 == 0) { r = 0.45f; g = 0.74f; b = 0.95f; }
    } else {   // pasto (también debajo de los árboles y de los lugares)
        r = par ? 0.42f : 0.38f; g = par ? 0.76f : 0.71f; b = par ? 0.30f : 0.27f;
        if (q % 13 == 0) { r += 0.05f; g += 0.05f; b += 0.04f; }
    }
    rect(m, x0, z0, x0 + 1.0f, z0 + 1.0f, r, g, b);
}

// Rayas, bordillos y cebras (van encima de la base)
static void celda_detalle(Malla *m, int c, int f) {
    g_y = 0.09f;
    char t = ciudad_tipo_en(c, f);
    float x0 = ciudad_centro_x(c) - 0.5f, z0 = ciudad_centro_z(f) - 0.5f;

    if (t == 'S') {
        // bordillo: una tira más clara pegada a la calle
        const float gr = 0.84f;
        if (es_calle(c, f - 1)) rect(m, x0, z0, x0 + 1.0f, z0 + 0.14f, gr, gr, gr * 0.96f);
        if (es_calle(c, f + 1)) rect(m, x0, z0 + 0.86f, x0 + 1.0f, z0 + 1.0f, gr, gr, gr * 0.96f);
        if (es_calle(c - 1, f)) rect(m, x0, z0, x0 + 0.14f, z0 + 1.0f, gr, gr, gr * 0.96f);
        if (es_calle(c + 1, f)) rect(m, x0 + 0.86f, z0, x0 + 1.0f, z0 + 1.0f, gr, gr, gr * 0.96f);
        return;
    }
    if (t != '=') return;

    int arriba = corrida(c, f, 0, -1), abajo = corrida(c, f, 0, 1);
    int izq = corrida(c, f, -1, 0), der = corrida(c, f, 1, 0);
    int vz = arriba + abajo + 1;       // largo de calle en vertical (hasta 21)
    int hx = izq + der + 1;
    int horizontal = (vz <= 8 && hx > vz);
    int vertical   = (hx <= 8 && vz > hx);
    const float amar_r = 0.96f, amar_g = 0.78f, amar_b = 0.16f;

    if (horizontal) {
        int pos = arriba, w = vz;
        // raya amarilla del centro, cortada
        if (pos == w / 2 && (c & 1) == 0) {
            if (w % 2 == 0) rect(m, x0, z0 - 0.07f, x0 + 1.0f, z0 + 0.07f, amar_r, amar_g, amar_b);
            else            rect(m, x0, z0 + 0.43f, x0 + 1.0f, z0 + 0.57f, amar_r, amar_g, amar_b);
        }
        // cebra: la última celda antes de un cruce
        int antes_der = es_calle(c + 1, f) && corrida(c + 1, f, 0, -1) + corrida(c + 1, f, 0, 1) + 1 > 8;
        int antes_izq = es_calle(c - 1, f) && corrida(c - 1, f, 0, -1) + corrida(c - 1, f, 0, 1) + 1 > 8;
        if (antes_der || antes_izq) {
            rect(m, x0 + 0.1f, z0 + 0.08f, x0 + 0.9f, z0 + 0.40f, 0.93f, 0.93f, 0.9f);
            rect(m, x0 + 0.1f, z0 + 0.60f, x0 + 0.9f, z0 + 0.92f, 0.93f, 0.93f, 0.9f);
        }
    } else if (vertical) {
        int pos = izq, w = hx;
        if (pos == w / 2 && (f & 1) == 0) {
            if (w % 2 == 0) rect(m, x0 - 0.07f, z0, x0 + 0.07f, z0 + 1.0f, amar_r, amar_g, amar_b);
            else            rect(m, x0 + 0.43f, z0, x0 + 0.57f, z0 + 1.0f, amar_r, amar_g, amar_b);
        }
        int antes_ab = es_calle(c, f + 1) && corrida(c, f + 1, -1, 0) + corrida(c, f + 1, 1, 0) + 1 > 8;
        int antes_ar = es_calle(c, f - 1) && corrida(c, f - 1, -1, 0) + corrida(c, f - 1, 1, 0) + 1 > 8;
        if (antes_ab || antes_ar) {
            rect(m, x0 + 0.08f, z0 + 0.1f, x0 + 0.40f, z0 + 0.9f, 0.93f, 0.93f, 0.9f);
            rect(m, x0 + 0.60f, z0 + 0.1f, x0 + 0.92f, z0 + 0.9f, 0.93f, 0.93f, 0.9f);
        }
    }
}

static void armar_pedazo(int pc, int pf) {
    Malla *m = &pedazos[pf][pc];
    malla_iniciar(m);
    int c0 = pc * CH, f0 = pf * CH;
    int c1 = c0 + CH, f1 = f0 + CH;
    if (c1 > ciudad_ancho()) c1 = ciudad_ancho();
    if (f1 > ciudad_alto())  f1 = ciudad_alto();
    for (int f = f0; f < f1; f++) for (int c = c0; c < c1; c++) celda_base(m, c, f);
    for (int f = f0; f < f1; f++) for (int c = c0; c < c1; c++) celda_detalle(m, c, f);
    malla_subir(m);
    armado[pf][pc] = 1;
}

void terreno_reiniciar(void) {
    for (int f = 0; f < MAX_CH; f++)
        for (int c = 0; c < MAX_CH; c++)
            if (armado[f][c]) { malla_liberar(&pedazos[f][c]); armado[f][c] = 0; }
}

void terreno_dibujar(float px, float pz, float giro) {
    float rad = giro * 3.14159265f / 180.0f, fx = sinf(rad), fz = -cosf(rad);
    int nc = (ciudad_ancho() + CH - 1) / CH, nf = (ciudad_alto() + CH - 1) / CH;
    if (nc > MAX_CH) nc = MAX_CH;
    if (nf > MAX_CH) nf = MAX_CH;
    int arma_este_cuadro = 2;
    for (int pf = 0; pf < nf; pf++) {
        for (int pc = 0; pc < nc; pc++) {
            float cx = ciudad_centro_x(pc * CH) - 0.5f + CH * 0.5f;
            float cz = ciudad_centro_z(pf * CH) - 0.5f + CH * 0.5f;
            float d = fmaxf(fabsf(cx - px), fabsf(cz - pz)) - CH * 0.5f;
            if (d > RADIO_BORRA) {
                if (armado[pf][pc]) { malla_liberar(&pedazos[pf][pc]); armado[pf][pc] = 0; }
                continue;
            }
            if (d > RADIO_ARMA) continue;
            // detrás de la cámara no hay nada que ver
            if ((cx - px) * fx + (cz - pz) * fz < -CH * 1.5f - 12.0f) continue;
            if (!armado[pf][pc]) {
                if (arma_este_cuadro <= 0) continue;
                arma_este_cuadro--;
                armar_pedazo(pc, pf);
            }
            malla_dibujar(&pedazos[pf][pc]);
        }
    }
}
