// El jugador: su posición, su movimiento y su animación
#ifndef PLAYER_JUGADOR_H
#define PLAYER_JUGADOR_H

typedef struct {
    float x, z;     // posición en el suelo
    float angulo;   // hacia dónde mira (grados)
    float vel;      // 0 parado ... 1 a toda velocidad (suavizado)
    float fase;     // avanza al caminar (mueve piernas y brazos)
    float tiempo;   // segundos desde que empezó (respiración, mirar alrededor)
    float elev, vy; // altura sobre el suelo (absoluta) y velocidad vertical
    float suelo;    // altura del piso donde está parado
} Jugador;

void jugador_iniciar(Jugador *j);

// Lo mueve según el stick izquierdo (lx, ly van de -1 a 1).
// "giro_camara" son los grados de la cámara: se camina relativo a lo que ves.
// correr = 1 mientras corre; saltar = 1 en el cuadro en que se aprieta saltar.
void jugador_actualizar(Jugador *j, float lx, float ly, float giro_camara, int correr, int saltar);

void jugador_dibujar(const Jugador *j);
void jugador_dibujar_en_carro(const Jugador *j, float cx, float cz, float ang, float roll, float pitch, float alt);

#endif
