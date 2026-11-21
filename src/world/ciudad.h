// La ciudad: calles, aceras, edificios y árboles hechos de cubos.
// El mapa se lee de un archivo de texto: cada letra es una celda de 1 x 1,
// igual de grande que el personaje.
#ifndef WORLD_CIUDAD_H
#define WORLD_CIUDAD_H

#define CIUDAD_TAM 1.0f   // lado de cada celda del mapa

// Lee el mapa desde un archivo de texto.
// Devuelve 1 si lo leyó bien. Si no puede leerlo, usa un mapa pequeño
// incluido en el programa y devuelve 0 (así el juego nunca queda vacío).
int ciudad_cargar(const char *ruta);

// Tamaño del mapa en celdas
int ciudad_ancho(void);
int ciudad_alto(void);

// Dónde empieza el jugador (la letra P del mapa, o un lugar libre cerca del centro)
void ciudad_inicio(float *x, float *z);

// Dibuja calles, aceras, edificios y árboles (el pasto lo dibuja suelo.c).
void ciudad_dibujar(void);

// Devuelve 1 si un círculo en (x, z) con ese radio toca un edificio o un árbol.
int ciudad_choca(float x, float z, float radio);

#endif
