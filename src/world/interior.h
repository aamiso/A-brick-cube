// Interiores de lugares (hospital, café, farmacia, banco): cámara fija desde arriba, como Pokémon.
#ifndef WORLD_INTERIOR_H
#define WORLD_INTERIOR_H
#include "player/jugador.h"

int  interior_hay(char letra);                    // 1 si a ese lugar se puede entrar
const char *interior_nombre(char letra);          // "Hospital", "Café"...
void interior_entrar(char letra, Jugador *j);     // arma el cuarto y pone al jugador en la puerta
void interior_salir(void);
// Mueve al jugador (stick izquierdo; arriba = norte). Devuelve 1 si pisó la salida.
int  interior_actualizar(Jugador *j, float lx, float ly, int correr);
void interior_dibujar(float tiempo);              // cuarto y gente (el jugador lo dibuja main)
int  interior_npc_cercano(const Jugador *j, float alcance);
const char *interior_npc_nombre(int n);
void interior_hablar(int n, const Jugador *j);
#endif
