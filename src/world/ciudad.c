#include "world/ciudad.h"
#include "engine/cubo.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// ============================================================
//  EL MAPA (archivo data/mapas/ciudad.txt)
//  Cada letra es una celda de 1 x 1. Se lee de arriba hacia abajo.
//
//    .  pasto          =  calle         S  acera
//    T  árbol          W  agua (no se puede cruzar)
//    P  donde empieza el jugador
//
//  CASAS (chicas, altura 3):
//    H roja    J crema    K verde
//  EDIFICIOS (grandes):
//    C amarillo (tienda, 6)     G naranja (bodega, 9)     B ladrillo (10)
//    A azul (oficinas, 14)      L marrón (departamentos, 15)
//    F gris (18)                N morado (22)
//    D blanco (torre, 26)       E vidrio (rascacielos, 40)
//
//  Las celdas iguales que se tocan se dibujan como UN solo bloque.
//  Dos edificios de la misma letra no deben tocarse: pon pasto o acera en medio.
// ============================================================

#define MAX_ANCHO   400
#define MAX_ALTO    400
#define MAX_BLOQUES 40000
#define MAX_ARBOLES 20000

#define RADIO_GRANDE  95.0f   // a qué distancia se dibujan edificios
#define RADIO_CHICO   55.0f   // casas y calles
#define RADIO_ARBOL   42.0f   // árboles (son lo más caro de dibujar)

typedef struct { char letra; float h; float r, g, b; int casa; } TipoEdificio;

static const TipoEdificio EDIFICIOS[] = {
    {'H',  3.0f, 0.85f, 0.35f, 0.30f, 1},
    {'J',  3.0f, 0.95f, 0.85f, 0.62f, 1},
    {'K',  3.2f, 0.45f, 0.75f, 0.55f, 1},
    {'C',  6.0f, 0.95f, 0.80f, 0.15f, 0},
    {'G',  9.0f, 0.95f, 0.55f, 0.15f, 0},
    {'B', 10.0f, 0.70f, 0.22f, 0.18f, 0},
    {'A', 14.0f, 0.20f, 0.40f, 0.85f, 0},
    {'L', 15.0f, 0.55f, 0.38f, 0.25f, 0},
    {'F', 18.0f, 0.55f, 0.58f, 0.62f, 0},
    {'N', 22.0f, 0.55f, 0.30f, 0.75f, 0},
    {'D', 26.0f, 0.88f, 0.88f, 0.92f, 0},
    {'E', 40.0f, 0.30f, 0.75f, 0.85f, 0},
};
#define N_TIPOS ((int)(sizeof(EDIFICIOS) / sizeof(EDIFICIOS[0])))

static char mapa[MAX_ALTO][MAX_ANCHO];
static int ancho = 0, alto = 0;
static float inicio_x = 0.0f, inicio_z = 0.0f;

typedef struct {
    float x, z, w, d;
    char tipo;
} Bloque;

static Bloque bloques[MAX_BLOQUES];
static int n_bloques = 0;
static float arboles[MAX_ARBOLES][2];
static int n_arboles = 0;

static const char *const MAPA_EMERGENCIA[] = {
    "SSSSSSSSSSSSSSSSSSSSSSSS",
    "S======================S",
    "SS....................SS",
    "S=AAAAA...BBBBB...T...=S",
    "S=AAAAA...BBBBB.......=S",
    "S=AAAAA...BBBBB...HHH.=S",
    "S=AAAAA...BBBBB...HHH.=S",
    "S=AAAAA...........HHH.=S",
    "S=.........P..........=S",
    "S=.T...DDDDDD......T..=S",
    "S=.....DDDDDD.........=S",
    "SS....................SS",
    "S======================S",
    "SSSSSSSSSSSSSSSSSSSSSSSS",
};

// ---------- Utilidades ----------

static const TipoEdificio *edificio_de(char t) {
    for (int i = 0; i < N_TIPOS; i++) if (EDIFICIOS[i].letra == t) return &EDIFICIOS[i];
    return NULL;
}
static int es_edificio(char t) { return edificio_de(t) != NULL; }
static int es_solido(char t)   { return es_edificio(t) || t == 'T' || t == 'W'; }

static char tipo_en(int col, int fila) {
    if (col < 0 || col >= ancho || fila < 0 || fila >= alto) return '.';
    return mapa[fila][col];
}

static float centro_x(int col)  { return (col  - ancho * 0.5f + 0.5f) * CIUDAD_TAM; }
static float centro_z(int fila) { return (fila - alto  * 0.5f + 0.5f) * CIUDAD_TAM; }

static void limpiar_mapa(void) {
    memset(mapa, '.', sizeof(mapa));
    ancho = alto = 0;
    n_bloques = n_arboles = 0;
}

// ---------- Preparar el mapa para dibujarlo ----------

static void armar_bloques(void) {
    static unsigned char usada[MAX_ALTO][MAX_ANCHO];
    memset(usada, 0, sizeof(usada));
    n_bloques = 0;
    n_arboles = 0;
    inicio_x = inicio_z = 0.0f;
    int hay_inicio = 0;

    // La P solo marca el inicio; su celda toma lo que hay al lado.
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
            if (!(t == '=' || t == 'S' || t == 'W' || es_edificio(t))) continue;
            if (usada[f][c]) continue;

            int w = 1;
            while (c + w < ancho && mapa[f][c + w] == t && !usada[f][c + w]) w++;
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
    static char linea[MAX_ANCHO + 64];
    while (alto < MAX_ALTO && fgets(linea, sizeof(linea), archivo) != NULL) {
        size_t largo = strlen(linea);
        while (largo > 0 && (linea[largo - 1] == '\n' || linea[largo - 1] == '\r')) {
            linea[--largo] = '\0';
        }
        if (largo == 0 || linea[0] == '#') continue;

        if (largo > MAX_ANCHO) largo = MAX_ANCHO;
        for (size_t c = 0; c < largo; c++) {
            char t = (char)toupper((unsigned char)linea[c]);
            int valida = (t == '.' || t == '=' || t == 'S' || t == 'T' || t == 'P' ||
                          t == 'W' || es_edificio(t));
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

static int cerca(const Bloque *b, float px, float pz, float radio) {
    return fabsf(b->x - px) < radio + b->w * 0.5f && fabsf(b->z - pz) < radio + b->d * 0.5f;
}

void ciudad_dibujar_suelo(float px, float pz) {
    // Tres pasadas en orden (agua, calles, aceras): cada una tapa a la anterior.
    for (int pasada = 0; pasada < 3; pasada++) {
        char quiero = (pasada == 0) ? 'W' : (pasada == 1) ? '=' : 'S';
        for (int i = 0; i < n_bloques; i++) {
            const Bloque *b = &bloques[i];
            if (b->tipo != quiero || !cerca(b, px, pz, RADIO_GRANDE)) continue;
            if (quiero == 'W')      baldosa(b->x, 0.0f, b->z, b->w, b->d, 0.12f, 0.42f, 0.85f);
            else if (quiero == '=') baldosa(b->x, 0.0f, b->z, b->w, b->d, 0.25f, 0.25f, 0.27f);
            else                    baldosa(b->x, 0.0f, b->z, b->w, b->d, 0.62f, 0.62f, 0.60f);
        }
    }
}

static void dibujar_edificio(const Bloque *b, const TipoEdificio *e) {
    if (e->casa) {
        // Casa: paredes + techo más ancho y más oscuro
        caja(b->x, e->h * 0.5f, b->z, b->w, e->h, b->d, e->r, e->g, e->b);
        caja(b->x, e->h + 0.45f, b->z, b->w + 0.5f, 0.9f, b->d + 0.5f,
             e->r * 0.55f + 0.15f, e->g * 0.4f, e->b * 0.4f);
        return;
    }
    // Base oscura (la planta baja), cuerpo y tapa, como una torre de LEGO
    float h = e->h;
    caja(b->x, h * 0.5f, b->z, b->w, h, b->d, e->r, e->g, e->b);
    if (h >= 10.0f) {
        caja(b->x, 0.7f, b->z, b->w + 0.2f, 1.4f, b->d + 0.2f,
             e->r * 0.45f, e->g * 0.45f, e->b * 0.45f);
    }
    if (b->w > 2.0f && b->d > 2.0f) {
        caja(b->x, h + 0.25f, b->z, b->w - 0.6f, 0.5f, b->d - 0.6f,
             e->r * 0.75f, e->g * 0.75f, e->b * 0.75f);
    }
    if (h >= 22.0f) { // antena en los más altos
        caja(b->x, h + 1.5f, b->z, 0.35f, 3.0f, 0.35f, 0.8f, 0.1f, 0.1f);
    }
}

static void dibujar_arbol(float x, float z) {
    caja(x, 0.50f, z, 0.30f, 1.00f, 0.30f, 0.45f, 0.28f, 0.12f);
    caja(x, 1.50f, z, 1.00f, 1.20f, 1.00f, 0.10f, 0.55f, 0.15f);
}

void ciudad_dibujar(float px, float pz) {
    for (int i = 0; i < n_bloques; i++) {
        const Bloque *b = &bloques[i];
        const TipoEdificio *e = edificio_de(b->tipo);
        if (e == NULL) continue;
        if (!cerca(b, px, pz, e->casa ? RADIO_CHICO : RADIO_GRANDE)) continue;
        dibujar_edificio(b, e);
    }
    for (int i = 0; i < n_arboles; i++) {
        if (fabsf(arboles[i][0] - px) > RADIO_ARBOL || fabsf(arboles[i][1] - pz) > RADIO_ARBOL) continue;
        dibujar_arbol(arboles[i][0], arboles[i][1]);
    }
}

// ---------- Colisiones ----------

int ciudad_choca(float x, float z, float radio) {
    int c0 = (int)floorf((x - radio) / CIUDAD_TAM + ancho * 0.5f);
    int c1 = (int)floorf((x + radio) / CIUDAD_TAM + ancho * 0.5f);
    int f0 = (int)floorf((z - radio) / CIUDAD_TAM + alto * 0.5f);
    int f1 = (int)floorf((z + radio) / CIUDAD_TAM + alto * 0.5f);

    for (int fila = f0; fila <= f1; fila++) {
        for (int col = c0; col <= c1; col++) {
            char t = tipo_en(col, fila);
            if (!es_solido(t)) continue;

            float mitad = (t == 'T') ? 0.35f : CIUDAD_TAM * 0.5f;
            float cx = centro_x(col), cz = centro_z(fila);
            float qx = fminf(fmaxf(x, cx - mitad), cx + mitad);
            float qz = fminf(fmaxf(z, cz - mitad), cz + mitad);
            float dx = x - qx, dz = z - qz;
            if (dx * dx + dz * dz < radio * radio) return 1;
        }
    }
    return 0;
}

float ciudad_altura_en(float x, float z) {
    int col = (int)floorf(x / CIUDAD_TAM + ancho * 0.5f);
    int fila = (int)floorf(z / CIUDAD_TAM + alto * 0.5f);
    char t = tipo_en(col, fila);
    const TipoEdificio *e = edificio_de(t);
    if (e) return e->h + (e->casa ? 0.9f : 0.5f);
    if (t == 'T') return 2.4f;
    return 0.0f;
}
