#include "player/jugador.h"
#include <vitaGL.h>
#include "player/personaje.h"
#include "world/suelo.h"
#include "world/ciudad.h"
#include "world/vehiculos.h"
#include "core/util.h"
#include <math.h>

#define RADIO 0.4f // qué tan "ancho" es el personaje para chocar (mide 1 x 1 en el mapa)

static Personaje personaje;

void jugador_iniciar(Jugador *j) {
    j->x = 0.0f;
    j->z = 0.0f;
    j->angulo = 0.0f;
    j->vel = 0.0f;
    j->fase = 0.0f;
    j->tiempo = 0.0f;
    j->elev = 0.0f;
    j->vy = 0.0f;
    j->suelo = 0.0f;
    if (!personaje.listo) personaje_crear(&personaje, 0xd9382c, 0x2f62c9, 0xf2c230, 0x5a3a22);
}

void jugador_actualizar(Jugador *j, float lx, float ly, float giro_camara, int correr, int saltar) {
    j->tiempo += 1.0f / 60.0f;

    float rad = giro_camara * PI / 180.0f;
    float adelante_x = sinf(rad), adelante_z = -cosf(rad);
    float derecha_x = cosf(rad),  derecha_z = sinf(rad);

    float dx = derecha_x * lx + adelante_x * (-ly);
    float dz = derecha_z * lx + adelante_z * (-ly);
    float fuerza = sqrtf(dx * dx + dz * dz);
    float objetivo = 0.0f;

    if (fuerza > 0.01f) {
        if (fuerza > 1.0f) { dx /= fuerza; dz /= fuerza; fuerza = 1.0f; }
        float turbo = correr ? 1.7f : 1.0f;
        objetivo = fuerza * turbo;
        // Se mueve en X y en Z por separado: si choca de frente, todavía se desliza por la pared.
        float paso_x = dx * 0.15f * turbo, paso_z = dz * 0.15f * turbo;
        if (!ciudad_choca_y(j->x + paso_x, j->z, RADIO, j->elev) && !vehiculos_choca(j->x + paso_x, j->z, RADIO, -1)) j->x += paso_x;
        if (!ciudad_choca_y(j->x, j->z + paso_z, RADIO, j->elev) && !vehiculos_choca(j->x, j->z + paso_z, RADIO, -1)) j->z += paso_z;

        // Gira suave hacia donde camina (por el camino más corto)
        float meta = atan2f(dx, dz) * 180.0f / PI;
        float dif = meta - j->angulo;
        while (dif > 180.0f) dif -= 360.0f;
        while (dif < -180.0f) dif += 360.0f;
        j->angulo += dif * 0.30f;
        j->fase += 0.30f * fuerza * (correr ? 1.4f : 1.0f);
    }
    // Física vertical: se para sobre lo más alto que alcanza; si no hay piso, cae.
    float piso = ciudad_piso(j->x, j->z, RADIO, j->elev);
    if (saltar && j->elev <= piso + 0.02f && j->vy <= 0.0f) j->vy = 0.17f;
    if (j->elev > piso + 0.001f || j->vy > 0.0f) {
        j->elev += j->vy; j->vy -= 0.012f;
        float nuevo_piso = ciudad_piso(j->x, j->z, RADIO, j->elev);
        if (j->vy <= 0.0f && j->elev <= nuevo_piso) { j->elev = nuevo_piso; j->vy = 0.0f; }
        piso = nuevo_piso;
    } else {
        j->elev = piso; j->vy = 0.0f;
    }
    j->suelo = piso;
    // La velocidad sube y baja poco a poco (así no se ve brusco)
    j->vel += (objetivo - j->vel) * 0.15f;
    if (objetivo == 0.0f && j->vel < 0.02f) { j->vel = 0.0f; j->fase = 0.0f; }

    suelo_limitar(&j->x, &j->z);
}

void jugador_dibujar(const Jugador *j) {
    glPushMatrix();
    glTranslatef(0.0f, j->suelo, 0.0f);
    personaje_dibujar(&personaje, j->x, j->z, j->angulo, j->vel, j->fase, j->tiempo, j->elev - j->suelo);
    glPopMatrix();
}
