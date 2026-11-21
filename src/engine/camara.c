#include "engine/camara.h"
#include <vitaGL.h>

// Límites de la inclinación: no dejar que baje al suelo ni quede totalmente arriba
#define ALTO_MIN 20.0f
#define ALTO_MAX 75.0f

void camara_iniciar(Camara *c) {
    c->giro = 0.0f;
    c->alto = 55.0f;    // semi cenital (antes era 25)
    c->distancia = 9.0f;
}

void camara_actualizar(Camara *c, float rx, float ry) {
    c->giro += rx * 2.5f;
    c->alto += ry * 1.5f;
    if (c->alto < ALTO_MIN) c->alto = ALTO_MIN;
    if (c->alto > ALTO_MAX) c->alto = ALTO_MAX;
}

void camara_aplicar(const Camara *c, float px, float pz) {
    // Se aleja, se inclina, gira y mira al personaje
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -c->distancia);
    glRotatef(c->alto, 1.0f, 0.0f, 0.0f);
    glRotatef(c->giro, 0.0f, 1.0f, 0.0f);
    glTranslatef(-px, -1.0f, -pz);
}
