#include "world/ciudad.h"
#include "engine/cubo.h"
#include <math.h>
#include <string.h>

// ============================================================
//  EL MAPA
//  Cada letra es una baldosa de 3 x 3. Se lee de arriba hacia abajo.
//  Para cambiar la ciudad, solo cambia las letras:
//
//    .  pasto
//    =  calle
//    A  edificio azul     (bajo)
//    B  edificio rojo     (mediano)
//    C  edificio amarillo (chiquito)
//    D  edificio blanco   (alto)
//    T  árbol
//
//  Cada fila debe tener 24 letras y debe haber 24 filas.
//  (Si una fila queda corta, lo que falta se toma como pasto.)
// ============================================================
static const char *const MAPA[CIUDAD_LADO] = {
    "........................",
    ".T..T=AA.CC==CC.AA=T..T.",
    "..BB.=AA.CC==CC.AA=.DD..",
    "..BB.=.....==.....=.DD..",
    ".T..T=CC.AA==AA.CC=T..T.",
    ".======================.",
    ".CC.D=.....==T...T=BB.A.",
    ".CC.D=.T.T.==.AAA.=BB.A.",
    ".....=..T..==.AAA.=.....",
    ".DD.C=.T.T.==.AAA.=AA.B.",
    ".DD.C=.....==T...T=AA.B.",
    ".======================.",
    ".======================.",
    ".T..T=T...T==.....=DD.C.",
    "..CC.=.BBB.==.T.T.=DD.C.",
    "..CC.=.BBB.==..T..=.....",
    "..CC.=.BBB.==.T.T.=CC.D.",
    ".T..T=T...T==.....=CC.D.",
    ".======================.",
    ".AA.B=T...T==CC.BB=.....",
    ".AA.B=.DDD.==CC.BB=.TT..",
    ".....=.DDD.==.....=.TT..",
    ".BB.A=T...T==BB.CC=.....",
    "........................",
};

// Qué hay en la baldosa (columna, fila). Fuera del mapa o fila corta = pasto.
static char tipo_en(int col, int fila) {
    if (col < 0 || col >= CIUDAD_LADO || fila < 0 || fila >= CIUDAD_LADO) return '.';
    const char *f = MAPA[fila];
    if (f == NULL || (int)strlen(f) <= col) return '.';
    return f[col];
}

static int es_edificio(char t) {
    return t == 'A' || t == 'B' || t == 'C' || t == 'D';
}

static int es_solido(char t) {
    return es_edificio(t) || t == 'T';
}

// Centro de una baldosa en el mundo (el mapa está centrado en 0,0)
static float centro(int indice) {
    return (indice - CIUDAD_LADO / 2 + 0.5f) * CIUDAD_TAM;
}

// ---------- Dibujo ----------

static void dibujar_edificio(char t, float x, float z) {
    float h, r, g, b;
    switch (t) {
        case 'A': h = 3.0f; r = 0.20f; g = 0.40f; b = 0.85f; break;
        case 'B': h = 4.0f; r = 0.80f; g = 0.15f; b = 0.15f; break;
        case 'C': h = 2.5f; r = 0.95f; g = 0.80f; b = 0.15f; break;
        default:  h = 3.5f; r = 0.85f; g = 0.85f; b = 0.88f; break; // 'D'
    }
    float ancho = CIUDAD_TAM * 0.96f; // un poquito menos, así se ven las juntas de cada bloque
    caja(x, h / 2.0f, z, ancho, h, ancho, r, g, b);
    // El "botoncito" de arriba, como en las piezas LEGO
    caja(x, h + 0.10f, z, 1.0f, 0.20f, 1.0f, r * 0.9f, g * 0.9f, b * 0.9f);
}

static void dibujar_arbol(float x, float z) {
    caja(x, 0.60f, z, 0.50f, 1.20f, 0.50f, 0.45f, 0.28f, 0.12f); // tronco
    caja(x, 1.70f, z, 1.50f, 1.40f, 1.50f, 0.10f, 0.55f, 0.15f); // hojas
    caja(x, 2.55f, z, 0.80f, 0.60f, 0.80f, 0.15f, 0.65f, 0.20f); // copa
}

void ciudad_dibujar(void) {
    for (int fila = 0; fila < CIUDAD_LADO; fila++) {
        for (int col = 0; col < CIUDAD_LADO; col++) {
            char t = tipo_en(col, fila);
            float x = centro(col), z = centro(fila);
            if (t == '=') {
                // La calle va un poquito arriba del pasto para que no se mezclen
                float gris = (((col + fila) & 1) == 0) ? 0.24f : 0.28f;
                baldosa(x, 0.03f, z, CIUDAD_TAM, CIUDAD_TAM, gris, gris, gris + 0.02f);
            } else if (es_edificio(t)) {
                dibujar_edificio(t, x, z);
            } else if (t == 'T') {
                dibujar_arbol(x, z);
            }
        }
    }
}

// ---------- Colisiones ----------

int ciudad_choca(float x, float z, float radio) {
    // Baldosas que el círculo podría estar tocando
    int c0 = (int)floorf((x - radio) / CIUDAD_TAM) + CIUDAD_LADO / 2;
    int c1 = (int)floorf((x + radio) / CIUDAD_TAM) + CIUDAD_LADO / 2;
    int f0 = (int)floorf((z - radio) / CIUDAD_TAM) + CIUDAD_LADO / 2;
    int f1 = (int)floorf((z + radio) / CIUDAD_TAM) + CIUDAD_LADO / 2;

    for (int fila = f0; fila <= f1; fila++) {
        for (int col = c0; col <= c1; col++) {
            char t = tipo_en(col, fila);
            if (!es_solido(t)) continue;

            // Edificio: toda la baldosa. Árbol: solo el tronco (más chiquito).
            float mitad = (t == 'T') ? 0.45f : CIUDAD_TAM / 2.0f;
            float cx = centro(col), cz = centro(fila);

            // Punto de la caja más cercano al círculo
            float px = fminf(fmaxf(x, cx - mitad), cx + mitad);
            float pz = fminf(fmaxf(z, cz - mitad), cz + mitad);
            float dx = x - px, dz = z - pz;
            if (dx * dx + dz * dz < radio * radio) return 1;
        }
    }
    return 0;
}
