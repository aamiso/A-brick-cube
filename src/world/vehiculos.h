// Carros hechos de cubitos: se pueden manejar
#ifndef WORLD_VEHICULOS_H
#define WORLD_VEHICULOS_H

// Pone carros estacionados en las calles cerca de (cx, cz)
void vehiculos_iniciar(float cx, float cz);
void vehiculos_dibujar(float px, float pz);

// 1 si un círculo toca algún carro (menos el número "ignorar"; -1 = ninguno)
int vehiculos_choca(float x, float z, float radio, int ignorar);

// Número del carro más cercano a menos de "alcance" unidades (-1 si no hay)
int  vehiculo_cercano(float x, float z, float alcance);

// Maneja el carro n: volante -1..1 (izquierda negativo), gas 0..1, freno/reversa 0..1, mano = freno de mano
void vehiculo_manejar(int n, float volante, float gas, float freno, int mano);
void vehiculos_pasivos(int activo);
// Velocidad del carro en el mundo
void vehiculo_velocidad(int n, float *vx, float *vz);
void vehiculo_pos(int n, float *x, float *z, float *angulo, float *vel);

int vehiculo_danio(int n);   // 1 si ya tiene el frente roto
// Inclinación de la carrocería (grados): roll = balanceo, pitch = cabeceo
void vehiculo_inclinacion(int n, float *roll, float *pitch);
// 1 si un carro veloz toca el círculo; da la velocidad del carro (para atropellar)
int vehiculos_atropella(float x, float z, float radio, float *vx, float *vz);

#endif
