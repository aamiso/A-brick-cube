#include "engine/cubo.h"
#include <vitaGL.h>

// Las 8 esquinas de un cubo de tamaño 1 (centrado en el origen)
static const float esquinas[8][3] = {
    {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
    {-0.5f, -0.5f,  0.5f}, {0.5f, -0.5f,  0.5f}, {0.5f, 0.5f,  0.5f}, {-0.5f, 0.5f,  0.5f}};

// Las 6 caras: frente, atrás, izquierda, derecha, arriba, abajo
static const int caras[6][4] = {
    {4, 5, 6, 7}, {1, 0, 3, 2}, {0, 4, 7, 3},
    {5, 1, 2, 6}, {3, 7, 6, 2}, {0, 1, 5, 4}};

// Qué tan clara se ve cada cara (así se nota el volumen sin luces)
static const float brillo[6] = {0.80f, 0.60f, 0.70f, 0.70f, 1.00f, 0.40f};

#define CARA_ARRIBA 4

static float vertices[36 * 3];

// Convierte las 6 caras en 12 triángulos (36 puntos)
void cubo_construir(void) {
    const int orden[6] = {0, 1, 2, 0, 2, 3};
    int n = 0;
    for (int c = 0; c < 6; c++) {
        for (int k = 0; k < 6; k++) {
            const float *p = esquinas[caras[c][orden[k]]];
            for (int j = 0; j < 3; j++) vertices[n * 3 + j] = p[j];
            n++;
        }
    }
}

void cubo_activar(void) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
}

void cubo_desactivar(void) {
    glDisableClientState(GL_VERTEX_ARRAY);
}

void caja(float x, float y, float z, float sx, float sy, float sz,
          float r, float g, float b) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(sx, sy, sz);
    for (int c = 0; c < 5; c++) { // la cara de abajo nunca se ve, no se dibuja
        glColor3f(r * brillo[c], g * brillo[c], b * brillo[c]);
        glDrawArrays(GL_TRIANGLES, c * 6, 6);
    }
    glPopMatrix();
}

void baldosa(float x, float y, float z, float sx, float sz,
             float r, float g, float b) {
    glPushMatrix();
    // La tapa del cubo está 0.5 arriba de su centro; se baja 0.5 para que
    // la superficie del suelo quede justo a la altura "y" (los pies del personaje)
    glTranslatef(x, y - 0.5f, z);
    glScalef(sx, 1.0f, sz);
    glColor3f(r, g, b);
    glDrawArrays(GL_TRIANGLES, CARA_ARRIBA * 6, 6);
    glPopMatrix();
}
