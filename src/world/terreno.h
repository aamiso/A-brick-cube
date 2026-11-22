// El suelo con detalle: pasto en cuadritos, calles con rayas, aceras con bordillo y cebras.
// Se arma en pedazos (chunks) solo cerca del jugador.
#ifndef WORLD_TERRENO_H
#define WORLD_TERRENO_H

// giro = hacia dónde mira la cámara (grados); no se dibujan los pedazos que quedan detrás
void terreno_dibujar(float px, float pz, float giro);

// Borra los pedazos armados (se llama si se carga otro mapa)
void terreno_reiniciar(void);

#endif
