#include "world/ciudad.h"
#include "engine/cubo.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// ============================================================
//  EL MAPA (archivo data/mapas/ciudad.txt)
//
//  Cada letra es una celda de 1 x 1. Se lee de arriba hacia abajo.
//
//    .  pasto
//    =  calle
//    S  acera
//    A  edificio azul      (altura 4)
//    B  edificio rojo      (altura 5)
//    C  edificio amarillo  (altura 3)
//    D  edificio blanco    (altura 6)
//    T  árbol
//    P  donde empieza el jugador
//
//  Las celdas iguales que se tocan se dibujan como UN solo bloque grande.
//  Un edificio de 5 x 5 son 25 letras iguales juntas.
//  Si quieres dos edificios del mismo color pegados, ponles una celda
//  de otra cosa en medio.
// ============================================================

#define MAX_ANCHO   200
#define MAX_ALTO    200
#define MAX_BLOQUES 6000   // bloques grandes (calles, aceras, edificios)
#define MAX_ARBOLES 6000

static char mapa[MAX_ALTO][MAX_ANCHO];
static int ancho = 0, alto = 0;
static float inicio_x = 0.0f, inicio_z = 0.0f;

typedef struct {
    float x, z;      // centro
    float w, d;      // ancho y largo
    char tipo;
} Bloque;

static Bloque bloques[MAX_BLOQUES];
static int n_bloques = 0;
static float arboles[MAX_ARBOLES][2];
static int n_arboles = 0;

// Mapa pequeño de emergencia, por si no se encuentra el archivo
static const char *const MAPA_EMERGENCIA[] = {
    "SSSSSSSSSSSSSSSSSSSSSSSS",
    "S======================S",
    "SS....................SS",
    "S=AAAAA...BBBBB...T...=S",
    "S=AAAAA...BBBBB.......=S",
    "S=AAAAA...BBBBB...CCC.=S",
    "S=AAAAA...BBBBB...CCC.=S",
    "S=AAAAA...........CCC.=S",
    "S=.........P..........=S",
    "S=.T...DDDDDD......T..=S",
    "S=.....DDDDDD.........=S",
    "SS....................SS",
    "S======================S",
    "SSSSSSSSSSSSSSSSSSSSSSSS",
};

// ---------- Utilidades ----------

static int es_edificio(char t) { return t == 'A' || t == 'B' || t == 'C' || t == 'D'; }
static int es_solido(char t)   { return es_edificio(t) || t == 'T'; }

// Qué hay en la celda (col, fila). Fuera del mapa = pasto.
static char tipo_en(int col, int fila) {
    if (col < 0 || col >= ancho || fila < 0 || fila >= alto) return '.';
    return mapa[fila][col];
}

// Centro de una celda en el mundo (el mapa queda centrado en 0,0)
static float centro_x(int col)  { return (col  - ancho * 0.5f + 0.5f) * CIUDAD_TAM; }
static float centro_z(int fila) { return (fila - alto  * 0.5f + 0.5f) * CIUDAD_TAM; }

static void limpiar_mapa(void) {
    memset(mapa, '.', sizeof(mapa));
    ancho = alto = 0;
    n_bloques = n_arboles = 0;
}

// ---------- Preparar el mapa para dibujarlo ----------

// Junta celdas iguales que se tocan en rectángulos grandes
// (menos bloques = el juego dibuja más rápido).
static void armar_bloques(void) {
    static unsigned char usada[MAX_ALTO][MAX_ANCHO];
    memset(usada, 0, sizeof(usada));
    n_bloques = 0;
    n_arboles = 0;
    inicio_x = inicio_z = 0.0f;
    int hay_inicio = 0;

    // Primero: la P solo marca dónde empieza el jugador. La celda de la P
    // toma lo que hay al lado (calle, acera o pasto) para no dejar un hueco.
    for (int f = 0; f < alto; f++) {
        for (int c = 0; c < ancho; c++) {
            if (mapa[f][c] != 'P') continue;
            if (!hay_inicio) {
                inicio_x = centro_x(c);
                inicio_z = centro_z(f);
                hay_inicio = 1;
            }
            static const int vecinos[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
            char reemplazo = '.';
            for (int v = 0; v < 4; v++) {
                char n = tipo_en(c + vecinos[v][0], f + vecinos[v][1]);
                if (n == '.' || n == '=' || n == 'S') { reemplazo = n; break; }
            }
            mapa[f][c] = reemplazo;
        }
    }

    for (int f = 0; f < alto; f++) {
        for (int c = 0; c < ancho; c++) {
            char t = mapa[f][c];

            if (t == 'T') {
                if (n_arboles < MAX_ARBOLES) {
                    arboles[n_arboles][0] = centro_x(c);
                    arboles[n_arboles][1] = centro_z(f);
                    n_arboles++;
                }
                continue;
            }

            if (!(t == '=' || t == 'S' || es_edificio(t))) continue;
            if (usada[f][c]) continue;

            // Estirar hacia la derecha
            int w = 1;
            while (c + w < ancho && mapa[f][c + w] == t && !usada[f][c + w]) w++;
            // Estirar hacia abajo mientras toda la fila coincida
            int d = 1;
            for (;;) {
                if (f + d >= alto) break;
                int igual = 1;
                for (int k = 0; k < w; k++) {
                    if (mapa[f + d][c + k] != t || usada[f + d][c + k]) { igual = 0; break; }
                }
                if (!igual) break;
                d++;
            }
            for (int j = 0; j < d; j++)
                for (int k = 0; k < w; k++) usada[f + j][c + k] = 1;

            if (n_bloques < MAX_BLOQUES) {
                Bloque *b = &bloques[n_bloques++];
                b->x = (centro_x(c) + centro_x(c + w - 1)) * 0.5f;
                b->z = (centro_z(f) + centro_z(f + d - 1)) * 0.5f;
                b->w = w * CIUDAD_TAM;
                b->d = d * CIUDAD_TAM;
                b->tipo = t;
            }
        }
    }

    if (!hay_inicio) {
        // Sin letra P: la celda libre más cercana al centro del mapa
        float mejor = 1e30f;
        for (int f = 0; f < alto; f++) {
            for (int c = 0; c < ancho; c++) {
                if (es_solido(mapa[f][c])) continue;
                float dx = centro_x(c), dz = centro_z(f);
                float dist = dx * dx + dz * dz;
                if (dist < mejor) { mejor = dist; inicio_x = dx; inicio_z = dz; }
            }
        }
    }
}

// ---------- Cargar ----------

static void cargar_emergencia(void) {
    limpiar_mapa();
    int filas = (int)(sizeof(MAPA_EMERGENCIA) / sizeof(MAPA_EMERGENCIA[0]));
    for (int f = 0; f < filas && f < MAX_ALTO; f++) {
        int largo = (int)strlen(MAPA_EMERGENCIA[f]);
        for (int c = 0; c < largo && c < MAX_ANCHO; c++) {
            mapa[f][c] = MAPA_EMERGENCIA[f][c];
            if (c + 1 > ancho) ancho = c + 1;
        }
        alto = f + 1;
    }
    armar_bloques();
}

int ciudad_cargar(const char *ruta) {
    FILE *archivo = fopen(ruta, "r");
    if (archivo == NULL) {
        cargar_emergencia();
        return 0;
    }

    limpiar_mapa();
    char linea[MAX_ANCHO + 64];
    while (alto < MAX_ALTO && fgets(linea, sizeof(linea), archivo) != NULL) {
        // Quitar el salto de línea (en Windows son dos caracteres: \r\n)
        size_t largo = strlen(linea);
        while (largo > 0 && (linea[largo - 1] == '\n' || linea[largo - 1] == '\r')) {
            linea[--largo] = '\0';
        }
        if (largo == 0 || linea[0] == '#') continue; // línea vacía o comentario

        if (largo > MAX_ANCHO) largo = MAX_ANCHO;
        for (size_t c = 0; c < largo; c++) {
            char t = (char)toupper((unsigned char)linea[c]);
            int valida = (t == '.' || t == '=' || t == 'S' || t == 'T' || t == 'P' || es_edificio(t));
            mapa[alto][c] = valida ? t : '.';
        }
        if ((int)largo > ancho) ancho = (int)largo;
        alto++;
    }
    fclose(archivo);

    if (ancho == 0 || alto == 0) {
        cargar_emergencia();
        return 0;
    }
    armar_bloques();
    return 1;
}

int ciudad_ancho(void) { return ancho; }
int ciudad_alto(void)  { return alto; }

void ciudad_inicio(float *x, float *z) {
    *x = inicio_x;
    *z = inicio_z;
}

// ---------- Dibujo ----------

static void dibujar_edificio(const Bloque *b) {
    float h, r, g, bl;
    switch (b->tipo) {
        case 'A': h = 4.0f; r = 0.20f; g = 0.40f; bl = 0.85f; break;
        case 'B': h = 5.0f; r = 0.80f; g = 0.15f; bl = 0.15f; break;
        case 'C': h = 3.0f; r = 0.95f; g = 0.80f; bl = 0.15f; break;
        default:  h = 6.0f; r = 0.85f; g = 0.85f; bl = 0.88f; break; // 'D'
    }
    caja(b->x, h * 0.5f, b->z, b->w, h, b->d, r, g, bl);
    // Tapa más oscura arriba, como la plataforma de una pieza LEGO
    if (b->w > 1.5f && b->d > 1.5f) {
        caja(b->x, h + 0.08f, b->z, b->w - 0.4f, 0.16f, b->d - 0.4f,
             r * 0.8f, g * 0.8f, bl * 0.8f);
    }
}

static void dibujar_arbol(float x, float z) {
    caja(x, 0.40f, z, 0.30f, 0.80f, 0.30f, 0.45f, 0.28f, 0.12f); // tronco
    caja(x, 1.15f, z, 0.90f, 0.80f, 0.90f, 0.10f, 0.55f, 0.15f); // hojas
    caja(x, 1.70f, z, 0.50f, 0.40f, 0.50f, 0.15f, 0.65f, 0.20f); // copa
}

void ciudad_dibujar(void) {
    for (int i = 0; i < n_bloques; i++) {
        const Bloque *b = &bloques[i];
        if (b->tipo == '=') {
            // La calle va un poquito arriba del pasto para que no se mezclen
            baldosa(b->x, 0.03f, b->z, b->w, b->d, 0.25f, 0.25f, 0.27f);
        } else if (b->tipo == 'S') {
            baldosa(b->x, 0.05f, b->z, b->w, b->d, 0.62f, 0.62f, 0.60f);
        } else {
            dibujar_edificio(b);
        }
    }
    for (int i = 0; i < n_arboles; i++) {
        dibujar_arbol(arboles[i][0], arboles[i][1]);
    }
}

// ---------- Colisiones ----------

int ciudad_choca(float x, float z, float radio) {
    // Celdas que el círculo podría estar tocando
    int c0 = (int)floorf((x - radio) / CIUDAD_TAM + ancho * 0.5f);
    int c1 = (int)floorf((x + radio) / CIUDAD_TAM + ancho * 0.5f);
    int f0 = (int)floorf((z - radio) / CIUDAD_TAM + alto * 0.5f);
    int f1 = (int)floorf((z + radio) / CIUDAD_TAM + alto * 0.5f);

    for (int fila = f0; fila <= f1; fila++) {
        for (int col = c0; col <= c1; col++) {
            char t = tipo_en(col, fila);
            if (!es_solido(t)) continue;

            // Edificio: toda la celda. Árbol: solo el tronco (un poco más chico).
            float mitad = (t == 'T') ? 0.35f : CIUDAD_TAM * 0.5f;
            float cx = centro_x(col), cz = centro_z(fila);

            // Punto de la caja más cercano al círculo
            float px = fminf(fmaxf(x, cx - mitad), cx + mitad);
            float pz = fminf(fmaxf(z, cz - mitad), cz + mitad);
            float dx = x - px, dz = z - pz;
            if (dx * dx + dz * dz < radio * radio) return 1;
        }
    }
    return 0;
}
