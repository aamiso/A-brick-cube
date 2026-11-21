// La ciudad: calles, edificios y árboles hechos de cubos
#ifndef WORLD_CIUDAD_H
#define WORLD_CIUDAD_H

#define CIUDAD_LADO 24     // el mapa mide 24 x 24 baldosas
#define CIUDAD_TAM  3.0f   // cada baldosa mide 3 x 3

// Dibuja calles, edificios y árboles (el pasto lo dibuja suelo.c).
void ciudad_dibujar(void);

// Devuelve 1 si un círculo en (x, z) con ese radio toca un edificio o un árbol.
int ciudad_choca(float x, float z, float radio);

#endif
