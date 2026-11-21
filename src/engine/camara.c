#include "engine/camara.h"
#include "world/ciudad.h"
#include <vitaGL.h>
#include <math.h>

#define ALTO_MIN 5.0f
#define ALTO_MAX 70.0f
#define DIST_MIN 2.5f
#define PI_ 3.14159265f

void camara_iniciar(Camara *c) {
    c->giro = 0.0f;
    c->alto = 25.0f;
    c->distancia = 9.0f;
    c->real = 9.0f;
}

void camara_actualizar(Camara *c, float rx, float ry) {
    c->giro += rx * 2.5f;
    c->alto += ry * 1.5f;
    if (c->alto < ALTO_MIN) c->alto = ALTO_MIN;
    if (c->alto > ALTO_MAX) c->alto = ALTO_MAX;
}

// ¿Hay algo alto en ese punto (con un margen alrededor)?
static int bloqueado(float x, float y, float z) {
    const float m = 0.9f;
    return ciudad_altura_en(x, z) > y - 0.3f ||
           ciudad_altura_en(x + m, z) > y - 0.3f ||
           ciudad_altura_en(x - m, z) > y - 0.3f ||
           ciudad_altura_en(x, z + m) > y - 0.3f ||
           ciudad_altura_en(x, z - m) > y - 0.3f;
}

void camara_aplicar(Camara *c, float px, float pz) {
    float a = c->alto * PI_ / 180.0f, g = c->giro * PI_ / 180.0f;
    float dx = -cosf(a) * sinf(g), dy = sinf(a), dz = cosf(a) * cosf(g);

    // Camina desde el jugador hacia la cámara y se detiene antes de tocar un edificio
    float meta = c->distancia;
    for (int k = 1; k <= 24; k++) {
        float t = c->distancia * k / 24.0f;
        if (bloqueado(px + dx * t, 1.0f + dy * t, pz + dz * t)) {
            meta = t - c->distancia / 24.0f;
            break;
        }
    }
    if (meta < DIST_MIN) meta = DIST_MIN;
    // Se acerca rápido (para no atravesar paredes) y se aleja despacio (se siente suave)
    if (meta < c->real) c->real = c->real + (meta - c->real) * 0.5f;
    else                c->real = c->real + (meta - c->real) * 0.08f;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -c->real);
    glRotatef(c->alto, 1.0f, 0.0f, 0.0f);
    glRotatef(c->giro, 0.0f, 1.0f, 0.0f);
    glTranslatef(-px, -1.0f, -pz);
}
