#include "ui/ui.h"
#include "engine/cubo.h"
#include <vitaGL.h>
#include <string.h>
#include "ui/fuente_datos.h"

#define MAXV 12000
static float pos[MAXV * 3];
static unsigned char col[MAXV * 4];
static int nv = 0;

static void vaciar(void) {
    if (nv == 0) return;
    glVertexPointer(3, GL_FLOAT, 0, pos);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, col);
    glDrawArrays(GL_TRIANGLES, 0, nv);
    nv = 0;
}

void ui_inicio(void) {
    nv = 0;
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    /* 960x544 centrado en 400x240, conservando la proporción de los elementos. */
    glOrtho(0.0f, 960.0f, 560.0f, -16.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
}

void ui_fin(void) {
    vaciar();
    glDisableClientState(GL_COLOR_ARRAY);
    glEnable(GL_DEPTH_TEST);
    cubo_activar();
}

void ui_rect(float x, float y, float w, float h, unsigned rgba) {
    if (nv + 6 > MAXV) vaciar();
    float q[6][2] = { {x, y}, {x + w, y}, {x + w, y + h}, {x, y}, {x + w, y + h}, {x, y + h} };
    for (int i = 0; i < 6; i++) {
        pos[nv * 3] = q[i][0]; pos[nv * 3 + 1] = q[i][1]; pos[nv * 3 + 2] = 0.0f;
        col[nv * 4] = (rgba >> 24) & 255; col[nv * 4 + 1] = (rgba >> 16) & 255;
        col[nv * 4 + 2] = (rgba >> 8) & 255; col[nv * 4 + 3] = rgba & 255;
        nv++;
    }
}

// Lee un carácter UTF-8 y avanza el puntero
static unsigned siguiente(const char **p) {
    const unsigned char *s = (const unsigned char *)*p;
    unsigned c = s[0];
    if (c < 0x80) { *p += 1; return c; }
    if ((c & 0xE0) == 0xC0 && s[1]) { *p += 2; return ((c & 0x1F) << 6) | (s[1] & 0x3F); }
    if ((c & 0xF0) == 0xE0 && s[1] && s[2]) { *p += 3; return ((c & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F); }
    *p += 1; return '?';
}

static int buscar(unsigned c) {
    for (int i = 0; i < FUENTE_N; i++) if (FUENTE_COD[i] == c) return i;
    return buscar('?');
}

float ui_ancho(const char *txt, int esc) {
    float n = 0;
    while (*txt) { siguiente(&txt); n += 1; }
    return n * FUENTE_W * esc;
}

float ui_texto(float x, float y, int esc, const char *txt, unsigned rgba) {
    float x0 = x;
    while (*txt) {
        unsigned c = siguiente(&txt);
        int g = buscar(c);
        for (int f = 0; f < FUENTE_H; f++) {
            unsigned char b = FUENTE_BITS[g][f];
            int k = 0;
            while (k < FUENTE_W) {
                if (!(b & (0x80 >> k))) { k++; continue; }
                int k2 = k;
                while (k2 < FUENTE_W && (b & (0x80 >> k2))) k2++;
                ui_rect(x + k * esc, y + f * esc, (k2 - k) * esc, esc, rgba);
                k = k2;
            }
        }
        x += FUENTE_W * esc;
    }
    return x - x0;
}

int ui_partir(const char *txt, int esc, float ancho, char lin[][96], int max_lin) {
    int max_car = (int)(ancho / (FUENTE_W * esc));
    if (max_car > 94) max_car = 94;
    int n = 0;
    char palabra[96], actual[96] = "";
    int ca = 0;      // caracteres (no bytes) en la línea actual
    const char *p = txt;
    while (*p && n < max_lin) {
        int pb = 0, pc = 0;
        while (*p == ' ') p++;
        while (*p && *p != ' ' && pb < 90) { const char *q = p; siguiente(&p); memcpy(palabra + pb, q, p - q); pb += (int)(p - q); pc++; }
        palabra[pb] = 0;
        if (pb == 0) break;
        if (ca > 0 && ca + 1 + pc > max_car) { strcpy(lin[n++], actual); actual[0] = 0; ca = 0; if (n >= max_lin) break; }
        if (ca > 0) { strcat(actual, " "); ca++; }
        strcat(actual, palabra); ca += pc;
    }
    if (ca > 0 && n < max_lin) strcpy(lin[n++], actual);
    return n;
}
