// Misiones cortas: entregas, carreras y destrucción
#ifndef WORLD_MISIONES_H
#define WORLD_MISIONES_H
int  mision_activa(void);
int  mision_preparar(float px, float pz);          // inventa una misión para ofrecer (1 si pudo)
const char *mision_oferta(void);                   // texto de la oferta
void mision_aceptar(void);
void mision_actualizar(float x, float z, int en_carro);   // cada cuadro (x,z = jugador o su carro)
void mision_dibujar_3d(float x, float z);          // rayo amarillo en el destino
void mision_dibujar_hud(void);                     // dentro de ui_inicio / ui_fin
int  mision_marca(float *x, float *z);             // 1 si hay destino para el minimapa
int  mision_dinero(void);
#endif
