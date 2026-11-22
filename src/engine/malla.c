#include "engine/malla.h"
#include "engine/cubo.h"
#include <vitaGL.h>
#include <stdlib.h>

void malla_iniciar(Malla *m) { m->pos = NULL; m->col = NULL; m->n = m->cap = 0; }

void malla_liberar(Malla *m) {
    free(m->pos); free(m->col);
    malla_iniciar(m);
}

static int reservar(Malla *m, int extra) {
    if (m->n + extra <= m->cap) return 1;
    int nueva = m->cap ? m->cap * 2 : 256;
    while (nueva < m->n + extra) nueva *= 2;
    float *p = (float *)realloc(m->pos, (size_t)nueva * 3 * sizeof(float));
    if (!p) return 0;
    m->pos = p;
    unsigned char *c = (unsigned char *)realloc(m->col, (size_t)nueva * 4);
    if (!c) return 0;
    m->col = c;
    m->cap = nueva;
    return 1;
}

static unsigned char a_byte(float v) {
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    return (unsigned char)(v * 255.0f + 0.5f);
}

static void punto(Malla *m, const float *p, unsigned char r, unsigned char g, unsigned char b) {
    float *d = m->pos + m->n * 3;
    d[0] = p[0]; d[1] = p[1]; d[2] = p[2];
    unsigned char *c = m->col + m->n * 4;
    c[0] = r; c[1] = g; c[2] = b; c[3] = 255;
    m->n++;
}

void malla_quad(Malla *m, const float *a, const float *b, const float *c, const float *d,
                float r, float g, float bl) {
    if (!reservar(m, 6)) return;
    unsigned char R = a_byte(r), G = a_byte(g), B = a_byte(bl);
    punto(m, a, R, G, B); punto(m, b, R, G, B); punto(m, c, R, G, B);
    punto(m, a, R, G, B); punto(m, c, R, G, B); punto(m, d, R, G, B);
}

void malla_tapa(Malla *m, float x, float y, float z, float sx, float sz,
                float r, float g, float bl) {
    float x0 = x - sx * 0.5f, x1 = x + sx * 0.5f, z0 = z - sz * 0.5f, z1 = z + sz * 0.5f;
    float a[3] = {x0, y, z0}, b[3] = {x1, y, z0}, c[3] = {x1, y, z1}, d[3] = {x0, y, z1};
    malla_quad(m, a, b, c, d, r, g, bl);
}

void malla_caja(Malla *m, float x, float y, float z, float sx, float sy, float sz,
                float r, float g, float bl) {
    float x0 = x - sx * 0.5f, x1 = x + sx * 0.5f;
    float y0 = y - sy * 0.5f, y1 = y + sy * 0.5f;
    float z0 = z - sz * 0.5f, z1 = z + sz * 0.5f;
    float A[3], B[3], C[3], D[3];
    // arriba
    A[0]=x0;A[1]=y1;A[2]=z0; B[0]=x1;B[1]=y1;B[2]=z0; C[0]=x1;C[1]=y1;C[2]=z1; D[0]=x0;D[1]=y1;D[2]=z1;
    malla_quad(m, A, B, C, D, r * BRILLO_ARRIBA, g * BRILLO_ARRIBA, bl * BRILLO_ARRIBA);
    // frente (+z)
    A[0]=x0;A[1]=y0;A[2]=z1; B[0]=x1;B[1]=y0;B[2]=z1; C[0]=x1;C[1]=y1;C[2]=z1; D[0]=x0;D[1]=y1;D[2]=z1;
    malla_quad(m, A, B, C, D, r * BRILLO_FRENTE, g * BRILLO_FRENTE, bl * BRILLO_FRENTE);
    // atrás (-z)
    A[0]=x0;A[1]=y0;A[2]=z0; B[0]=x1;B[1]=y0;B[2]=z0; C[0]=x1;C[1]=y1;C[2]=z0; D[0]=x0;D[1]=y1;D[2]=z0;
    malla_quad(m, A, B, C, D, r * BRILLO_ATRAS, g * BRILLO_ATRAS, bl * BRILLO_ATRAS);
    // derecha (+x)
    A[0]=x1;A[1]=y0;A[2]=z0; B[0]=x1;B[1]=y0;B[2]=z1; C[0]=x1;C[1]=y1;C[2]=z1; D[0]=x1;D[1]=y1;D[2]=z0;
    malla_quad(m, A, B, C, D, r * BRILLO_DER, g * BRILLO_DER, bl * BRILLO_DER);
    // izquierda (-x)
    A[0]=x0;A[1]=y0;A[2]=z0; B[0]=x0;B[1]=y0;B[2]=z1; C[0]=x0;C[1]=y1;C[2]=z1; D[0]=x0;D[1]=y1;D[2]=z0;
    malla_quad(m, A, B, C, D, r * BRILLO_IZQ, g * BRILLO_IZQ, bl * BRILLO_IZQ);
}

void malla_dibujar(const Malla *m) {
    if (m->n <= 0) return;
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, m->pos);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, m->col);
    glDrawArrays(GL_TRIANGLES, 0, m->n);
    glDisableClientState(GL_COLOR_ARRAY);
    cubo_activar(); // vuelve a dejar listo el cubo para caja() y baldosa()
}
