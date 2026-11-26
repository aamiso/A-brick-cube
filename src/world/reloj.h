// Ciclo de día y noche (un día completo dura 8 minutos reales)
#ifndef WORLD_RELOJ_H
#define WORLD_RELOJ_H
void  reloj_actualizar(void);
float reloj_hora(void);                              // 0..24
void  reloj_cielo(float *r, float *g, float *b);     // color del cielo ahora
void  reloj_filtro(void);                            // oscurece/tiñe la pantalla (llamar dentro de ui_inicio..ui_fin)
#endif
