// Los lugares con modelo voxel: casas, banco y café.
// En el mapa se marcan con una letra que rellena un rectángulo del tamaño del modelo:
//   H casa roja (10 x 9)      J casa azul (11 x 11)     K casa moderna (13 x 11)
//   X banco (15 x 13)         Q café (16 x 14)       c comisaría (28 x 18)       Y hospital (44 x 24)
#ifndef WORLD_LUGARES_H
#define WORLD_LUGARES_H

// Ruta donde están los modelos dentro del juego
#ifndef RUTA_MODELOS
#define RUTA_MODELOS "app0:data/modelos/"
// Si el jugador está pegado a un lugar al que se puede entrar, devuelve su letra (si no, 0)
char lugar_entrada_cerca(float x, float z, float alcance);

#endif

// 1 si esa letra del mapa es un lugar con modelo
int lugar_es(char letra);

// Borra todos los lugares (se llama antes de leer un mapa nuevo)
void lugares_limpiar(void);

// Lee los archivos de los modelos (si falta alguno, ese lugar se dibuja como una caja)
void lugares_cargar_modelos(void);

// Pone un lugar en el mapa: letra y centro (x, z). w y d son el tamaño del rectángulo en el mapa.
void lugares_agregar(char letra, float x, float z, float w, float d);

// Dibuja los lugares cercanos al jugador
void lugares_dibujar(float px, float pz);

// 1 si un círculo toca una pared, columna, arbusto, etc. de un lugar
int lugares_choca_y(float x, float z, float radio, float y);
float lugares_piso(float x, float z, float radio, float y);
int lugares_choca(float x, float z, float radio);

// Altura de lo que hay en ese punto (0 si nada). La usa la cámara.
float lugares_altura(float x, float z);

// Cuántos lugares hay y datos de cada uno (para pruebas)
int lugares_cantidad(void);
int lugares_info(int i, char *letra, float *x, float *z);

// Si el jugador está pegado a un lugar al que se puede entrar, devuelve su letra (si no, 0)
char lugar_entrada_cerca(float x, float z, float alcance);

#endif
