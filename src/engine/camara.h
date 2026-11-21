// La cámara semi cenital: mira desde arriba, un poco inclinada, siguiendo al jugador
#ifndef ENGINE_CAMARA_H
#define ENGINE_CAMARA_H

typedef struct {
    float giro;      // giro alrededor del jugador (grados)
    float alto;      // inclinación (grados): 0 = de lado, 90 = desde arriba
    float distancia; // qué tan lejos QUIERE estar del jugador
    float real;      // qué tan lejos está de verdad (se acerca si hay un edificio atrás)
} Camara;

void camara_iniciar(Camara *c);
void camara_actualizar(Camara *c, float rx, float ry);

// Pone la cámara mirando al jugador (px, pz). Si un edificio se interpone,
// la cámara se acerca para no meterse dentro de la pared.
void camara_aplicar(Camara *c, float px, float pz);

#endif
