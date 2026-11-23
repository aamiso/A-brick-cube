// Una "malla": muchos cuadritos (caras) con su color, guardados juntos para
// dibujarlos TODOS de una sola vez. Así un modelo con miles de cubitos cuesta
// casi lo mismo que una caja.
#ifndef ENGINE_MALLA_H
#define ENGINE_MALLA_H

typedef struct {
    float *pos;            // x,y,z de cada punto
    unsigned char *col;    // r,g,b,a de cada punto
    int n, cap;            // puntos usados y espacio reservado
    int gpu;               // 1 si ya está guardada en la memoria de video (más rápido de dibujar)
    unsigned int vbo[2];   // buffers de la memoria de video (posiciones y colores)
} Malla;

void malla_iniciar(Malla *m);
void malla_liberar(Malla *m);

// Un cuadrado con 4 esquinas (a, b, c, d en orden) y un color r,g,b (0 a 1).
void malla_quad(Malla *m, const float *a, const float *b, const float *c, const float *d,
                float r, float g, float bl);

// Caja centrada en (x,y,z). Se le pone el brillo de cada cara. No tiene cara de abajo.
void malla_caja(Malla *m, float x, float y, float z, float sx, float sy, float sz,
                float r, float g, float bl);

// Solo un rectángulo plano horizontal a altura y.
void malla_tapa(Malla *m, float x, float y, float z, float sx, float sz,
                float r, float g, float bl);

// Pasa la malla a la memoria de video y borra la copia de la memoria normal.
// Después de esto la malla se dibuja MUCHO más rápido, pero ya no se le pueden agregar caras.
void malla_subir(Malla *m);

// Copia "origen" dentro de "destino" (movida a dx,dy,dz, escalada y girada alrededor de Y).
void malla_agregar(Malla *destino, const Malla *origen, float dx, float dy, float dz, float escala, float giro_grados);

void malla_dibujar(const Malla *m);

// Brillo de cada cara (así se nota el volumen sin luces)
#define BRILLO_ARRIBA 1.00f
#define BRILLO_FRENTE 0.90f   // cara que mira a +z
#define BRILLO_ATRAS  0.68f   // -z
#define BRILLO_DER    0.78f   // +x
#define BRILLO_IZQ    0.84f   // -x

#endif
