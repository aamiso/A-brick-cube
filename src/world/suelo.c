#include "world/suelo.h"
#include "world/ciudad.h"
#include "engine/cubo.h"
#include <math.h>

#define CUADRO 8.0f      // tamaño de cada cuadro de pasto
#define VISTA  110.0f    // solo se dibujan los cuadros cercanos al jugador

void suelo_dibujar(float px, float pz) {
    float W = ciudad_ancho() * CIUDAD_TAM;
    float H = ciudad_alto()  * CIUDAD_TAM;

    int i = 0;
    for (float x = 0.0f; x < W; x += CUADRO, i++) {
        float ancho = (x + CUADRO > W) ? (W - x) : CUADRO;
        float cx = x + ancho * 0.5f - W * 0.5f;
        if (fabsf(cx - px) > VISTA) continue;
        int j = 0;
        for (float z = 0.0f; z < H; z += CUADRO, j++) {
            float largo = (z + CUADRO > H) ? (H - z) : CUADRO;
            float cz = z + largo * 0.5f - H * 0.5f;
            if (fabsf(cz - pz) > VISTA) continue;
            int par = ((i + j) & 1) == 0;
            baldosa(cx, 0.0f, cz, ancho, largo, par ? 0.42f : 0.38f, par ? 0.76f : 0.71f, par ? 0.30f : 0.27f);
        }
    }
}

void suelo_limitar(float *x, float *z) {
    float lim_x = ciudad_ancho() * CIUDAD_TAM * 0.5f - 0.5f;
    float lim_z = ciudad_alto()  * CIUDAD_TAM * 0.5f - 0.5f;
    if (*x >  lim_x) *x =  lim_x;
    if (*x < -lim_x) *x = -lim_x;
    if (*z >  lim_z) *z =  lim_z;
    if (*z < -lim_z) *z = -lim_z;
}
