#include "player/jugador.h"
#include "engine/cubo.h"
#include "world/suelo.h"
#include "core/util.h"
#include <vitaGL.h>
#include <math.h>

void jugador_iniciar(Jugador *j) {
    j->x = 0.0f;
    j->z = 0.0f;
    j->angulo = 0.0f;
    j->paso = 0.0f;
}

void jugador_actualizar(Jugador *j, float lx, float ly, float giro_camara) {
    // Caminar relativo a hacia dónde mira la cámara
    float rad = giro_camara * PI / 180.0f;
    float adelante_x = sinf(rad), adelante_z = -cosf(rad);
    float derecha_x = cosf(rad),  derecha_z = sinf(rad);

    float dx = derecha_x * lx + adelante_x * (-ly);
    float dz = derecha_z * lx + adelante_z * (-ly);
    float fuerza = sqrtf(dx * dx + dz * dz);

    if (fuerza > 0.01f) {
        if (fuerza > 1.0f) { dx /= fuerza; dz /= fuerza; fuerza = 1.0f; }
        j->x += dx * 0.15f;
        j->z += dz * 0.15f;
        j->angulo = atan2f(dx, dz) * 180.0f / PI;
        j->paso += 0.25f * fuerza;
    } else {
        j->paso = 0.0f;
    }

    // No dejar que se salga del suelo
    suelo_limitar(&j->x, &j->z);
}

void jugador_dibujar(const Jugador *j) {
    // "paso" mueve las piernas y brazos cuando camina
    float swing = sinf(j->paso) * 25.0f;

    glPushMatrix();
    glTranslatef(j->x, 0.0f, j->z);
    glRotatef(j->angulo, 0.0f, 1.0f, 0.0f);

    // Piernas (azules)
    glPushMatrix();
    glTranslatef(-0.18f, 0.55f, 0.0f);
    glRotatef(swing, 1.0f, 0.0f, 0.0f);
    caja(0.0f, -0.275f, 0.0f, 0.3f, 0.55f, 0.3f, 0.10f, 0.25f, 0.75f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.18f, 0.55f, 0.0f);
    glRotatef(-swing, 1.0f, 0.0f, 0.0f);
    caja(0.0f, -0.275f, 0.0f, 0.3f, 0.55f, 0.3f, 0.10f, 0.25f, 0.75f);
    glPopMatrix();

    // Cuerpo (rojo)
    caja(0.0f, 0.90f, 0.0f, 0.75f, 0.70f, 0.40f, 0.85f, 0.10f, 0.10f);

    // Brazos (rojos)
    glPushMatrix();
    glTranslatef(-0.50f, 1.20f, 0.0f);
    glRotatef(-swing, 1.0f, 0.0f, 0.0f);
    caja(0.0f, -0.30f, 0.0f, 0.25f, 0.60f, 0.25f, 0.85f, 0.10f, 0.10f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.50f, 1.20f, 0.0f);
    glRotatef(swing, 1.0f, 0.0f, 0.0f);
    caja(0.0f, -0.30f, 0.0f, 0.25f, 0.60f, 0.25f, 0.85f, 0.10f, 0.10f);
    glPopMatrix();

    // Cabeza (amarilla) y el botoncito de arriba, como en las figuras LEGO
    caja(0.0f, 1.50f, 0.0f, 0.50f, 0.50f, 0.50f, 1.00f, 0.80f, 0.00f);
    caja(0.0f, 1.82f, 0.0f, 0.22f, 0.14f, 0.22f, 1.00f, 0.80f, 0.00f);

    // Ojitos (negros) en la cara de adelante
    caja(-0.12f, 1.55f, 0.26f, 0.08f, 0.08f, 0.02f, 0.05f, 0.05f, 0.05f);
    caja( 0.12f, 1.55f, 0.26f, 0.08f, 0.08f, 0.02f, 0.05f, 0.05f, 0.05f);

    glPopMatrix();
}
