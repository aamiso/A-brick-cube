#include "world/suelo.h"
#include "world/ciudad.h"
#include "engine/cubo.h"

// Cuadros de pasto: se dibujan de 4 x 4, o de 8 x 8 si el mapa es muy grande,
// para no dibujar miles de baldosas.
static float tam_cuadro(void) {
    return (ciudad_ancho() * ciudad_alto() <= 96 * 96) ? 4.0f : 8.0f;
}

void suelo_dibujar(void) {
    float W = ciudad_ancho() * CIUDAD_TAM;
    float H = ciudad_alto()  * CIUDAD_TAM;
    float S = tam_cuadro();

    int i = 0;
    for (float x = 0.0f; x < W; x += S, i++) {
        float ancho = (x + S > W) ? (W - x) : S;   // el último cuadro puede ser más angosto
        int j = 0;
        for (float z = 0.0f; z < H; z += S, j++) {
            float largo = (z + S > H) ? (H - z) : S;
            float g = (((i + j) & 1) == 0) ? 0.65f : 0.50f;
            baldosa(x + ancho * 0.5f - W * 0.5f, 0.0f, z + largo * 0.5f - H * 0.5f,
                    ancho, largo, 0.20f, g, 0.20f);
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
