// La ciudad: calles, aceras, edificios con ventanas, árboles y lugares con modelo voxel.
// El mapa se lee de un archivo de texto: cada letra es una celda de 1 x 1,
// igual de grande que el personaje.
#ifndef WORLD_CIUDAD_H
#define WORLD_CIUDAD_H

#define CIUDAD_TAM 1.0f   // lado de cada celda del mapa

// Lee el mapa. Devuelve 1 si lo leyó bien; si no, usa un mapa pequeño de emergencia y devuelve 0.
int ciudad_cargar(const char *ruta);

int ciudad_ancho(void);
int ciudad_alto(void);

// Dónde empieza el jugador (la letra P del mapa)
void ciudad_inicio(float *x, float *z);

// Dibuja lo que va PEGADO al suelo (agua, calles, aceras). Se llama con la profundidad apagada.
void ciudad_dibujar_suelo(float px, float pz);

// Dibuja lo que tiene volumen (casas, edificios, árboles). Solo lo que está cerca del jugador.
void ciudad_dibujar(float px, float pz);

// 1 si un círculo en (x, z) con ese radio toca un edificio, árbol o agua.
int ciudad_choca(float x, float z, float radio);

// Qué letra hay en la celda (col, fila). Fuera del mapa devuelve '.'.
char ciudad_tipo_en(int col, int fila);
// Centro de una celda en el mundo
float ciudad_centro_x(int col);
float ciudad_centro_z(int fila);

// Altura de lo que hay en ese punto (0 si es suelo libre). La usa la cámara.
float ciudad_altura_en(float x, float z);

#endif
