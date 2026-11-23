// Carros hechos de cubitos: se pueden manejar
#ifndef WORLD_VEHICULOS_H
#define WORLD_VEHICULOS_H

// Pone carros estacionados en las calles cerca de (cx, cz)
void vehiculos_iniciar(float cx, float cz);
void vehiculos_dibujar(float px, float pz);

// Número del carro más cercano a menos de "alcance" unidades (-1 si no hay)
int  vehiculo_cercano(float x, float z, float alcance);

// Maneja el carro n. lx = volante (-1..1), ly = stick (arriba = -1 = acelerar), freno = 1 si frena
void vehiculo_manejar(int n, float lx, float ly, int freno);
void vehiculo_pos(int n, float *x, float *z, float *angulo, float *vel);

#endif
