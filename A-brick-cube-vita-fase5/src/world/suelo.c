#include "world/suelo.h"
#include "world/ciudad.h"
#include "engine/cubo.h"
#include "engine/malla.h"
#include <math.h>

#define CUADRO 8.0f      // tamaño de cada cuadro de pasto

static Malla pasto;
static int pasto_listo = 0, pasto_ancho = 0, pasto_alto = 0;

// El pasto de todo el mapa es UNA malla en la memoria de video: se arma una vez.
void suelo_dibujar(float px, float pz) {
    (void)px; (void)pz;
    if (pasto_listo && (pasto_ancho != ciudad_ancho() || pasto_alto != ciudad_alto())) {
        malla_liberar(&pasto); pasto_listo = 0;
    }
    if (!pasto_listo) {
        float W = ciudad_ancho() * CIUDAD_TAM;
        float H = ciudad_alto()  * CIUDAD_TAM;
        malla_iniciar(&pasto);
        int i = 0;
        for (float x = 0.0f; x < W; x += CUADRO, i++) {
            float ancho = (x + CUADRO > W) ? (W - x) : CUADRO;
            int j = 0;
            for (float z = 0.0f; z < H; z += CUADRO, j++) {
                float largo = (z + CUADRO > H) ? (H - z) : CUADRO;
                int par = ((i + j) & 1) == 0;
                malla_tapa(&pasto, x + ancho * 0.5f - W * 0.5f, 0.0f, z + largo * 0.5f - H * 0.5f,
                           ancho, largo, par ? 0.42f : 0.38f, par ? 0.76f : 0.71f, par ? 0.30f : 0.27f);
            }
        }
        malla_subir(&pasto);
        pasto_listo = 1; pasto_ancho = ciudad_ancho(); pasto_alto = ciudad_alto();
    }
    malla_dibujar(&pasto);
}

void suelo_limitar(float *x, float *z) {
    float lim_x = ciudad_ancho() * CIUDAD_TAM * 0.5f - 0.5f;
    float lim_z = ciudad_alto()  * CIUDAD_TAM * 0.5f - 0.5f;
    if (*x >  lim_x) *x =  lim_x;
    if (*x < -lim_x) *x = -lim_x;
    if (*z >  lim_z) *z =  lim_z;
    if (*z < -lim_z) *z = -lim_z;
}
