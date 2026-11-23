// Modelos hechos de cubitos (voxels). Se leen de un archivo de texto:
//   modelo Nombre
//   color R #d9382c      (la letra R es ese color)
//   capa 1               (un piso de cubitos, de abajo hacia arriba)
//   ....RRRR....         (cada letra es un cubito, "." es vacío; arriba = fondo, abajo = frente)
#ifndef ENGINE_VOXEL_H
#define ENGINE_VOXEL_H

#include "engine/malla.h"

typedef struct {
    int W, D, H;                  // ancho (x), largo (z), alto (capas)
    unsigned char *v;             // una letra por cubito (0 = vacío)
    unsigned int pal[256];        // color de cada letra (0xRRGGBB)
    unsigned char hay[256];       // 1 si esa letra tiene color
} Voxeles;

// Lee un modelo. Devuelve 1 si salió bien.
int voxel_cargar(Voxeles *vx, const char *ruta);
// Lee un modelo desde un texto que ya está en memoria (sirve para pruebas).
int voxel_desde_texto(Voxeles *vx, const char *texto);
void voxel_liberar(Voxeles *vx);

// Pone un cubito (para armar modelos desde el código).
void voxel_crear(Voxeles *vx, int W, int D, int H);
void voxel_poner(Voxeles *vx, int x, int y, int z, char letra);
void voxel_caja(Voxeles *vx, int x0, int y0, int z0, int w, int h, int d, char letra);
void voxel_color(Voxeles *vx, char letra, unsigned int rgb);

// 1 si hay un cubito ahí
int voxel_hay(const Voxeles *vx, int x, int y, int z);

// Convierte el modelo en una malla (sin las caras que no se ven).
// Cada cubito mide "lado". El punto (ox,oy,oz) del modelo, en cubitos, queda en el origen.
void voxel_malla(const Voxeles *vx, float lado, float ox, float oy, float oz, Malla *salida);

#endif
