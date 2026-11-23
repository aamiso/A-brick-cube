#include "engine/camara.h"
#include "world/ciudad.h"
#include <vitaGL.h>
#include <math.h>

#define ALTO_MIN 14.0f
#define ALTO_MAX 70.0f
#define DIST_MIN 1.6f
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

// Hasta dónde llega la cámara sin tocar nada (afina el punto exacto)
static float libre_hasta(float px, float pz, float alto, float giro, float dist) {
    float a = alto * PI_ / 180.0f, g = giro * PI_ / 180.0f;
    float dx = -cosf(a) * sinf(g), dy = sinf(a), dz = cosf(a) * cosf(g);
    float ok = 0.0f, mal = -1.0f;
    for (int k = 1; k <= 40; k++) {
        float t = dist * k / 40.0f;
        if (bloqueado(px + dx * t, 1.0f + dy * t, pz + dz * t, t >= 2.5f)) { mal = t; break; }
        ok = t;
    }
    if (mal < 0.0f) return dist;
    for (int i = 0; i < 4; i++) {
        float t = (ok + mal) * 0.5f;
        if (bloqueado(px + dx * t, 1.0f + dy * t, pz + dz * t, t >= 2.5f)) mal = t; else ok = t;
    }
    return ok - 0.15f;
}

void camara_aplicar(Camara *c, float px, float pz) {
    // La inclinación NO cambia: si algo estorba, la cámara solo se acerca al jugador.
    c->alto_real = c->alto;
    float meta = libre_hasta(px, pz, c->alto, c->giro, c->distancia);
    if (meta < DIST_MIN) meta = DIST_MIN;
    // Suave en los dos sentidos (así no da tirones): se acerca rápido y se aleja despacio
    float k = (meta < c->real) ? 0.60f : 0.05f;
    c->real += (meta - c->real) * k;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -c->real);
    glRotatef(c->alto_real, 1.0f, 0.0f, 0.0f);
    glRotatef(c->giro, 0.0f, 1.0f, 0.0f);
    glTranslatef(-px, -1.0f, -pz);
}
