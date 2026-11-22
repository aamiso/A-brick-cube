#include "engine/camara.h"
#include "world/ciudad.h"
#include <vitaGL.h>
#include <math.h>

#define ALTO_MIN 5.0f
#define ALTO_MAX 70.0f
#define DIST_MIN 1.6f
#define ALTO_AUTO_MAX 85.0f   // si hay una pared pegada, la cámara puede subir casi hasta arriba del jugador
#define PI_ 3.14159265f

void camara_iniciar(Camara *c) {
    c->giro = 0.0f;
    c->alto = 25.0f;
    c->distancia = 9.0f;
    c->real = 9.0f;
    c->alto_real = 25.0f;
}

void camara_actualizar(Camara *c, float rx, float ry) {
    c->giro += rx * 2.5f;
    c->alto += ry * 1.5f;
    if (c->alto < ALTO_MIN) c->alto = ALTO_MIN;
    if (c->alto > ALTO_MAX) c->alto = ALTO_MAX;
}

// ¿Hay algo que tape la cámara en ese punto? Los árboles solo cuentan justo en el punto;
// los edificios también cuentan un poco alrededor (para no pegarse a la pared).
static int bloqueado(float x, float y, float z, int con_margen) {
    const float m = 0.4f, y_min = y - 0.3f;
    if (ciudad_altura_en(x, z) > y_min) return 1;
    if (!con_margen) return 0;
    float h;
    h = ciudad_altura_en(x + m, z); if (h > 3.2f && h > y_min) return 1;
    h = ciudad_altura_en(x - m, z); if (h > 3.2f && h > y_min) return 1;
    h = ciudad_altura_en(x, z + m); if (h > 3.2f && h > y_min) return 1;
    h = ciudad_altura_en(x, z - m); if (h > 3.2f && h > y_min) return 1;
    return 0;
}

// Hasta dónde llega la cámara (de 0 a "dist") antes de topar con algo, con esa inclinación
static float alcance(float px, float pz, float alto, float giro, float dist) {
    float a = alto * PI_ / 180.0f, g = giro * PI_ / 180.0f;
    float dx = -cosf(a) * sinf(g), dy = sinf(a), dz = cosf(a) * cosf(g);
    for (int k = 1; k <= 24; k++) {
        float t = dist * k / 24.0f;
        if (bloqueado(px + dx * t, 1.0f + dy * t, pz + dz * t, t >= 2.5f)) return t - dist / 24.0f;
    }
    return dist;
}

void camara_aplicar(Camara *c, float px, float pz) {
    // 1) Busca la inclinación más baja (desde la que elige el jugador) con el camino libre
    float objetivo = c->alto, mejor_alcance = -1.0f, mejor_alto = c->alto;
    int libre = 0;
    for (float ang = c->alto; ang <= ALTO_AUTO_MAX + 0.01f; ang += 4.0f) {
        float t = alcance(px, pz, ang, c->giro, c->distancia);
        if (t >= c->distancia - 0.01f) { objetivo = ang; libre = 1; break; }
        if (t >= mejor_alcance) { mejor_alcance = t; mejor_alto = ang; }
    }
    if (!libre) objetivo = mejor_alto;

    // 2) Sube de golpe si hace falta; baja despacio (se siente suave)
    float usar = objetivo;
    if (objetivo < c->alto_real) {
        usar = c->alto_real + (objetivo - c->alto_real) * 0.10f;
        if (alcance(px, pz, usar, c->giro, c->distancia) < alcance(px, pz, objetivo, c->giro, c->distancia) - 0.01f)
            usar = objetivo;
    }
    c->alto_real = usar;

    // 3) Si aun así no cabe, se acerca
    float meta = alcance(px, pz, usar, c->giro, c->distancia);
    if (meta < DIST_MIN) meta = DIST_MIN;
    if (meta < c->real) c->real = meta;
    else                c->real = c->real + (meta - c->real) * 0.08f;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -c->real);
    glRotatef(c->alto_real, 1.0f, 0.0f, 0.0f);
    glRotatef(c->giro, 0.0f, 1.0f, 0.0f);
    glTranslatef(-px, -1.0f, -pz);
}
