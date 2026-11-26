// Cubitos que vuelan cuando algo se rompe
#ifndef WORLD_ESCOMBROS_H
#define WORLD_ESCOMBROS_H
// n cubitos de color (0xRRGGBB) y tamaño "tam" salen de (x,y,z), empujados con (vx,vz) más una explosión de "fuerza"
void escombros_crear(float x, float y, float z, float vx, float vz, unsigned color, int n, float tam, float fuerza);
void escombros_actualizar(void);
void escombros_dibujar(float px, float pz);
#endif
