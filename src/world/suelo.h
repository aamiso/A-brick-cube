// El suelo: pasto en cuadros claros y oscuros, y el límite del jugador
#ifndef WORLD_SUELO_H
#define WORLD_SUELO_H

void suelo_dibujar(float px, float pz);

// Si la posición se sale del mapa, la devuelve al borde.
void suelo_limitar(float *x, float *z);

#endif
