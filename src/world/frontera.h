// La frontera del mapa: playa, valla, mar y montañas alrededor de la ciudad.
#ifndef WORLD_FRONTERA_H
#define WORLD_FRONTERA_H

// Playa y mar (pegados al suelo, se dibujan con la profundidad apagada)
void frontera_dibujar_suelo(float px, float pz);

// Valla y montañas (tienen volumen)
void frontera_dibujar(float px, float pz);

#endif
