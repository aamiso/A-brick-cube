// La cámara semi cenital: mira desde arriba, un poco inclinada, siguiendo al jugador
#ifndef ENGINE_CAMARA_H
#define ENGINE_CAMARA_H

typedef struct {
    float giro;      // giro alrededor del jugador (grados)
    float alto;      // inclinación (grados): 0 = de lado, 90 = desde arriba
    float distancia; // qué tan lejos está del jugador
} Camara;

// Pone la cámara en su posición inicial (semi cenital).
void camara_iniciar(Camara *c);

// Mueve la cámara con el stick derecho (rx, ry van de -1 a 1).
void camara_actualizar(Camara *c, float rx, float ry);

// Pone la cámara mirando al jugador (px, pz). Se llama dentro del dibujo.
void camara_aplicar(const Camara *c, float px, float pz);

#endif
