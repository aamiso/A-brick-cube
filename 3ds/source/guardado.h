// Guardar y cargar partida (3DS): sdmc:/3ds/A-brick-cube/partida.dat
#ifndef BRICK3DS_GUARDADO_H
#define BRICK3DS_GUARDADO_H

typedef struct {
    float x, z, angulo, suelo;   // jugador
    float hora;                  // reloj del juego (0..24)
    int dinero;
} Partida;

int guardado_escribir(const Partida *p);   // 1 si se guardó
int guardado_leer(Partida *p);             // 1 si había partida válida

#endif
