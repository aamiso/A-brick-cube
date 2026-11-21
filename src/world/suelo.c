#include "world/suelo.h"
#include "world/ciudad.h"
#include "engine/cubo.h"

// El pasto cubre justo el mismo tamaño que el mapa de la ciudad
#define MITAD (CIUDAD_LADO / 2)
#define TAM   CIUDAD_TAM

void suelo_dibujar(void) {
    for (int i = -MITAD; i < MITAD; i++) {
        for (int j = -MITAD; j < MITAD; j++) {
            int par = ((i + j) & 1) == 0;
            float g = par ? 0.65f : 0.50f;
            baldosa((i + 0.5f) * TAM, 0.0f, (j + 0.5f) * TAM, TAM, TAM,
                    0.20f, g, 0.20f);
        }
    }
}

void suelo_limitar(float *x, float *z) {
    float limite = MITAD * TAM - 0.5f;
    if (*x >  limite) *x =  limite;
    if (*x < -limite) *x = -limite;
    if (*z >  limite) *z =  limite;
    if (*z < -limite) *z = -limite;
}
