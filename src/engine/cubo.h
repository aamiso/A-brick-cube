// El cubo: la "pieza de LEGO" básica con la que se construye todo
#ifndef ENGINE_CUBO_H
#define ENGINE_CUBO_H

// Prepara los datos del cubo. Se llama una sola vez al empezar.
void cubo_construir(void);

// Se llaman antes y después de dibujar cajas o baldosas en cada cuadro.
void cubo_activar(void);
void cubo_desactivar(void);

// Dibuja una caja: posición (x,y,z), tamaño (ancho,alto,largo) y color (r,g,b).
void caja(float x, float y, float z, float sx, float sy, float sz,
          float r, float g, float b);

// Dibuja solo la tapa de arriba de una caja (para el suelo, es más rápido).
void baldosa(float x, float y, float z, float sx, float sz,
             float r, float g, float b);

#endif
