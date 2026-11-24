// Gente que camina por la ciudad y con la que se puede hablar
#ifndef WORLD_NPCS_H
#define WORLD_NPCS_H

void npcs_iniciar(float cx, float cz);                 // los reparte por las aceras cerca de (cx, cz)
void npcs_actualizar(float px, float pz);              // los mueve (solo los cercanos al jugador)
void npcs_dibujar(float px, float pz);
int  npcs_choca(float x, float z, float radio, int ignorar);   // 1 si un círculo toca a alguien
int  npc_cercano(float x, float z, float alcance);             // número del más cercano (-1 si no hay)
const char *npc_nombre(int n);
// Empieza a hablar: el NPC se queda quieto y mira al jugador. Abre la ventana de diálogo.
void npc_hablar(int n, float px, float pz);
void npc_soltar(int n);                                // termina la charla: sigue caminando

#endif
