// El suelo: baldosas verdes en cuadrícula
#ifndef WORLD_SUELO_H
#define WORLD_SUELO_H

void suelo_dibujar(void);

// Si la posición se sale del suelo, la devuelve al borde.
void suelo_limitar(float *x, float *z);

#endif
