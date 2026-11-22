// El jugador: su posición, su movimiento y su animación
#ifndef PLAYER_JUGADOR_H
#define PLAYER_JUGADOR_H

typedef struct {
    float x, z;     // posición en el suelo
    float angulo;   // hacia dónde mira (grados)
    float vel;      // 0 parado ... 1 a toda velocidad (suavizado)
    float fase;     // avanza al caminar (mueve piernas y brazos)
    float tiempo;   // segundos desde que empezó (respiración, mirar alrededor)
} Jugador;

void jugador_iniciar(Jugador *j);

// Lo mueve según el stick izquierdo (lx, ly van de -1 a 1).
// "giro_camara" son los grados de la cámara: se camina relativo a lo que ves.
void jugador_actualizar(Jugador *j, float lx, float ly, float giro_camara);

void jugador_dibujar(const Jugador *j);

#endif
