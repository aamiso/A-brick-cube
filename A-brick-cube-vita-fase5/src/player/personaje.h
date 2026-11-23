// Un personaje hecho de cubitos (voxels) con partes que se mueven:
// cabeza, torso, dos brazos y dos piernas. Sirve para el jugador y para los NPC.
#ifndef PLAYER_PERSONAJE_H
#define PLAYER_PERSONAJE_H

#include "engine/malla.h"

typedef struct {
    Malla cabeza, torso, brazo, pierna;
    int listo;
} Personaje;

// Colores en formato 0xRRGGBB
void personaje_crear(Personaje *p, unsigned camisa, unsigned pantalon, unsigned piel, unsigned pelo);
void personaje_liberar(Personaje *p);

// Dibuja al personaje en (x, z) mirando a "angulo" grados.
//   vel   = 0 parado ... 1 corriendo (mezcla entre reposo y caminar)
//   fase  = avanza al caminar (mueve piernas y brazos)
//   tiempo = segundos para la respiración y mirar alrededor en reposo
void personaje_dibujar(const Personaje *p, float x, float z, float angulo,
                       float vel, float fase, float tiempo);

#endif
