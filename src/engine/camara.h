// La cámara semi cenital: mira desde arriba, un poco inclinada, siguiendo al jugador
#ifndef ENGINE_CAMARA_H
#define ENGINE_CAMARA_H

typedef struct {
    float giro;      // giro alrededor del jugador (grados)
    float alto;      // inclinación (grados): 0 = de lado, 90 = desde arriba
    float distancia; // qué tan lejos QUIERE estar del jugador
    float real;      // qué tan lejos está de verdad (se acerca si hay un edificio atrás)
    float alto_real; // inclinación real
    float py;         // altura del piso donde está el jugador (la cámara la sigue suave)
    int   sin_choque; // 1 = distancia fija (en el carro): no se acerca por árboles ni edificios
    int   hombro;    // 0 = cámara normal (GTA), 1 = cámara sobre el hombro (Batman)
    float desvio;    // cuánto se corre hacia un lado (suave)
    float dist_meta; // distancia que quiere según el modo
} Camara;

void camara_iniciar(Camara *c);
// alternar = 1 en el cuadro en que se aprieta el botón de cambiar de cámara
void camara_actualizar(Camara *c, float rx, float ry, int alternar);
// Gira sola para quedar detrás del carro (heading = hacia dónde mira el carro)
void camara_seguir(Camara *c, float heading, float cuanto);

// Pone la cámara mirando al jugador (px, pz). Si un edificio se interpone,
// la cámara se acerca (nunca sube) para no meterse dentro de la pared.
void camara_aplicar(Camara *c, float px, float pz);

#endif
