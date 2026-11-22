#include "world/ciudad.h"
#include "world/lugares.h"
#include "engine/cubo.h"
#include "engine/malla.h"
#include <vitaGL.h>
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
//  LUGARES CON MODELO (cada uno rellena un rectángulo del tamaño indicado):
//    H casa roja 10x9   J casa azul 11x11   K casa moderna 13x11
//    X banco 15x13      Q café 16x14
//  EDIFICIOS (cajas con ventanas):
//    C tienda (6)   G bodega (9)   B ladrillo (10)   A oficinas (14)
//    L departamentos (15)   F gris (18)   N morado (22)
//    D torre blanca (26)   E rascacielos de vidrio (40)
//
//  Las celdas iguales que se tocan se dibujan como UN solo bloque.
//  Dos edificios de la misma letra no deben tocarse.
// ============================================================

#define MAX_ANCHO   400
#define MAX_ALTO    400
#define MAX_BLOQUES 40000
#define MAX_ARBOLES 20000

#define RADIO_GRANDE  95.0f   // hasta dónde se dibujan edificios y calles lejanas
#define RADIO_ARBOL   42.0f   // árboles
#define BAKES_POR_CUADRO 3    // cuántos edificios nuevos se arman por cuadro (para no trabarse)

typedef struct { char letra; float h; float r, g, b; } TipoEdificio;

static const TipoEdificio EDIFICIOS[] = {
    {'C',  6.0f, 0.95f, 0.80f, 0.20f},
    {'G',  9.0f, 0.95f, 0.58f, 0.22f},
    {'B', 10.0f, 0.72f, 0.28f, 0.22f},
    {'A', 14.0f, 0.28f, 0.48f, 0.85f},
    {'L', 15.0f, 0.60f, 0.42f, 0.30f},
    {'F', 18.0f, 0.60f, 0.63f, 0.67f},
    {'N', 22.0f, 0.58f, 0.36f, 0.76f},
    {'D', 26.0f, 0.90f, 0.90f, 0.93f},
    {'E', 40.0f, 0.34f, 0.74f, 0.84f},
};
#define N_TIPOS ((int)(sizeof(EDIFICIOS) / sizeof(EDIFICIOS[0])))

static char mapa[MAX_ALTO][MAX_ANCHO];
static int ancho = 0, alto = 0;
static float inicio_x = 0.0f, inicio_z = 0.0f;

typedef struct {
    float x, z, w, d;
    char tipo;
    char listo;      // 1 si ya se armó su malla
    Malla malla;
} Bloque;

static Bloque bloques[MAX_BLOQUES];
static int n_bloques = 0;
static float arboles[MAX_ARBOLES][2];
static int n_arboles = 0;
static int bakes_restantes = 0;

static const char *const MAPA_EMERGENCIA[] = {
    "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS",
    "S=======================================S",
    "SS.....................................SS",
    "S=AAAAAAA...BBBBBBBBB....T............=S",
    "S=AAAAAAA...BBBBBBBBB.................=S",
    "S=AAAAAAA...BBBBBBBBB....HHHHHHHHHH...=S",
    "S=AAAAAAA...BBBBBBBBB....HHHHHHHHHH...=S",
    "S=AAAAAAA................HHHHHHHHHH...=S",
    "S=...........P...........HHHHHHHHHH...=S",
    "S=.T....DDDDDD...........HHHHHHHHHH...=S",
    "S=......DDDDDD...........HHHHHHHHHH...=S",
    "S=.......................HHHHHHHHHH...=S",
    "S=.......................HHHHHHHHHH...=S",
    "S=.......................HHHHHHHHHH...=S",
    "SS.....................................SS",
    "S=======================================S",
    "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS",
};

// ---------- Utilidades ----------

static const TipoEdificio *edificio_de(char t) {
    for (int i = 0; i < N_TIPOS; i++) if (EDIFICIOS[i].letra == t) return &EDIFICIOS[i];
    return NULL;
}
static int es_edificio(char t) { return edificio_de(t) != NULL; }
static int es_solido(char t)   { return es_edificio(t) || t == 'T' || t == 'W'; }

char ciudad_tipo_en(int col, int fila) {
    if (col < 0 || col >= ancho || fila < 0 || fila >= alto) return '.';
    return mapa[fila][col];
}
#define tipo_en ciudad_tipo_en

float ciudad_centro_x(int col)  { return (col  - ancho * 0.5f + 0.5f) * CIUDAD_TAM; }
float ciudad_centro_z(int fila) { return (fila - alto  * 0.5f + 0.5f) * CIUDAD_TAM; }
#define centro_x ciudad_centro_x
#define centro_z ciudad_centro_z

static unsigned azar2(int a, int b) {
    unsigned h = (unsigned)a * 374761393u + (unsigned)b * 668265263u + 12345u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

static void limpiar_mapa(void) {
    for (int i = 0; i < n_bloques; i++) if (bloques[i].listo) malla_liberar(&bloques[i].malla);
    memset(mapa, '.', sizeof(mapa));
    ancho = alto = 0;
    n_bloques = n_arboles = 0;
    lugares_limpiar();
}

// ---------- Preparar el mapa para dibujarlo ----------

static void armar_bloques(void) {
    static unsigned char usada[MAX_ALTO][MAX_ANCHO];
    memset(usada, 0, sizeof(usada));
    for (int i = 0; i < n_bloques; i++) if (bloques[i].listo) malla_liberar(&bloques[i].malla);
    n_bloques = 0;
    n_arboles = 0;
    lugares_limpiar();
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
            int lugar = lugar_es(t);
            if (!(t == '=' || t == 'S' || t == 'W' || es_edificio(t) || lugar)) continue;
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

            float bx = (centro_x(c) + centro_x(c + w - 1)) * 0.5f;
            float bz = (centro_z(f) + centro_z(f + d - 1)) * 0.5f;
            if (lugar) {
                lugares_agregar(t, bx, bz, w * CIUDAD_TAM, d * CIUDAD_TAM);
            } else if (n_bloques < MAX_BLOQUES) {
                Bloque *b = &bloques[n_bloques++];
                b->x = bx; b->z = bz;
                b->w = w * CIUDAD_TAM;
                b->d = d * CIUDAD_TAM;
                b->tipo = t;
                b->listo = 0;
                malla_iniciar(&b->malla);
            }
        }
    }

    if (!hay_inicio) {
        float mejor = 1e30f;
        for (int f = 0; f < alto; f++) {
            for (int c = 0; c < ancho; c++) {
                if (es_solido(mapa[f][c]) || lugar_es(mapa[f][c])) continue;
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

// Árboles de cubitos: 3 formas distintas
static Malla arbol_malla[3];
static int arboles_listos = 0;
static void armar_arboles(void) {
    if (arboles_listos) return;
    for (int i = 0; i < 3; i++) malla_iniciar(&arbol_malla[i]);
    // 0: redondo
    malla_caja(&arbol_malla[0], 0, 0.50f, 0, 0.30f, 1.00f, 0.30f, 0.45f, 0.30f, 0.15f);
    malla_caja(&arbol_malla[0], 0, 1.45f, 0, 1.30f, 0.90f, 1.30f, 0.28f, 0.62f, 0.22f);
    malla_caja(&arbol_malla[0], 0, 2.05f, 0, 0.90f, 0.50f, 0.90f, 0.36f, 0.70f, 0.26f);
    malla_caja(&arbol_malla[0], 0.55f, 1.15f, 0.30f, 0.50f, 0.50f, 0.50f, 0.24f, 0.56f, 0.20f);
    // 1: alto (pino)
    malla_caja(&arbol_malla[1], 0, 0.40f, 0, 0.30f, 0.80f, 0.30f, 0.42f, 0.28f, 0.14f);
    malla_caja(&arbol_malla[1], 0, 1.10f, 0, 1.40f, 0.60f, 1.40f, 0.14f, 0.45f, 0.24f);
    malla_caja(&arbol_malla[1], 0, 1.70f, 0, 1.00f, 0.60f, 1.00f, 0.18f, 0.52f, 0.27f);
    malla_caja(&arbol_malla[1], 0, 2.30f, 0, 0.60f, 0.60f, 0.60f, 0.22f, 0.60f, 0.30f);
    // 2: chico y redondo, con flores
    malla_caja(&arbol_malla[2], 0, 0.30f, 0, 0.30f, 0.60f, 0.30f, 0.45f, 0.30f, 0.15f);
    malla_caja(&arbol_malla[2], 0, 1.05f, 0, 1.10f, 0.90f, 1.10f, 0.34f, 0.68f, 0.26f);
    malla_caja(&arbol_malla[2], 0.40f, 1.40f, 0.56f, 0.22f, 0.22f, 0.10f, 0.95f, 0.45f, 0.65f);
    malla_caja(&arbol_malla[2], -0.30f, 0.95f, 0.56f, 0.22f, 0.22f, 0.10f, 0.98f, 0.85f, 0.30f);
    arboles_listos = 1;
}

int ciudad_cargar(const char *ruta) {
    armar_arboles();
    FILE *archivo = fopen(ruta, "r");
    if (archivo == NULL) {
        cargar_emergencia();
        lugares_cargar_modelos();
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
                          t == 'W' || es_edificio(t) || lugar_es(t));
            mapa[alto][c] = valida ? t : '.';
        }
        if ((int)largo > ancho) ancho = (int)largo;
        alto++;
    }
    fclose(archivo);

    if (ancho == 0 || alto == 0) {
        cargar_emergencia();
        lugares_cargar_modelos();
        return 0;
    }
    armar_bloques();
    lugares_cargar_modelos();
    return 1;
}

int ciudad_ancho(void) { return ancho; }
int ciudad_alto(void)  { return alto; }

void ciudad_inicio(float *x, float *z) {
    *x = inicio_x;
    *z = inicio_z;
}

// ---------- Dibujo del suelo lejano (cuando está cerca lo dibuja terreno.c) ----------

static int cerca(const Bloque *b, float px, float pz, float radio) {
    return fabsf(b->x - px) < radio + b->w * 0.5f && fabsf(b->z - pz) < radio + b->d * 0.5f;
}

void ciudad_dibujar_suelo(float px, float pz) {
    for (int pasada = 0; pasada < 3; pasada++) {
        char quiero = (pasada == 0) ? 'W' : (pasada == 1) ? '=' : 'S';
        for (int i = 0; i < n_bloques; i++) {
            const Bloque *b = &bloques[i];
            if (b->tipo != quiero || !cerca(b, px, pz, RADIO_GRANDE)) continue;
            if (quiero == 'W')      baldosa(b->x, 0.0f, b->z, b->w, b->d, 0.18f, 0.48f, 0.86f);
            else if (quiero == '=') baldosa(b->x, 0.0f, b->z, b->w, b->d, 0.30f, 0.31f, 0.34f);
            else                    baldosa(b->x, 0.0f, b->z, b->w, b->d, 0.72f, 0.71f, 0.68f);
        }
    }
}

// ---------- Edificios con ventanas ----------

static void ventana(Malla *m, int lado, float u, float y0, float y1, float w, float fijo,
                    float r, float g, float b) {
    float a[3], bb[3], c[3], d[3], br;
    switch (lado) {
        case 0:  // frente (+z): u = x
            a[0]=u;     a[1]=y0; a[2]=fijo; bb[0]=u+w; bb[1]=y0; bb[2]=fijo;
            c[0]=u+w;   c[1]=y1; c[2]=fijo; d[0]=u;    d[1]=y1; d[2]=fijo; br = BRILLO_FRENTE; break;
        case 1:  // atrás (-z)
            a[0]=u;     a[1]=y0; a[2]=fijo; bb[0]=u+w; bb[1]=y0; bb[2]=fijo;
            c[0]=u+w;   c[1]=y1; c[2]=fijo; d[0]=u;    d[1]=y1; d[2]=fijo; br = BRILLO_ATRAS; break;
        case 2:  // derecha (+x): u = z
            a[0]=fijo;  a[1]=y0; a[2]=u;    bb[0]=fijo; bb[1]=y0; bb[2]=u+w;
            c[0]=fijo;  c[1]=y1; c[2]=u+w;  d[0]=fijo;  d[1]=y1; d[2]=u;    br = BRILLO_DER; break;
        default: // izquierda (-x)
            a[0]=fijo;  a[1]=y0; a[2]=u;    bb[0]=fijo; bb[1]=y0; bb[2]=u+w;
            c[0]=fijo;  c[1]=y1; c[2]=u+w;  d[0]=fijo;  d[1]=y1; d[2]=u;    br = BRILLO_IZQ; break;
    }
    malla_quad(m, a, bb, c, d, r * br, g * br, b * br);
}

static void armar_fachada(Bloque *b, const TipoEdificio *e) {
    Malla *m = &b->malla;
    float h = e->h;
    int vidrio = (e->letra == 'E');
    float ww = vidrio ? 1.6f : 1.0f, hueco = vidrio ? 0.4f : 1.0f;

    // cuerpo, base oscura y tapa del techo
    malla_caja(m, b->x, h * 0.5f, b->z, b->w, h, b->d, e->r, e->g, e->b);
    if (h >= 10.0f)
        malla_caja(m, b->x, 0.7f, b->z, b->w + 0.2f, 1.4f, b->d + 0.2f, e->r * 0.45f, e->g * 0.45f, e->b * 0.45f);
    if (b->w > 2.0f && b->d > 2.0f)
        malla_caja(m, b->x, h + 0.25f, b->z, b->w - 0.6f, 0.5f, b->d - 0.6f, e->r * 0.75f, e->g * 0.75f, e->b * 0.75f);
    // cosas en el techo
    if (b->w >= 6.0f && b->d >= 6.0f) {
        unsigned q = azar2((int)(b->x * 10), (int)(b->z * 10));
        malla_caja(m, b->x - b->w * 0.2f + (q % 3) * 0.3f, h + 0.9f, b->z - b->d * 0.2f, 1.6f, 0.8f, 1.6f, 0.55f, 0.58f, 0.62f);
        if (q & 4) malla_caja(m, b->x + b->w * 0.2f, h + 0.8f, b->z + b->d * 0.15f, 1.2f, 0.6f, 2.0f, 0.55f, 0.58f, 0.62f);
    }
    if (h >= 22.0f) malla_caja(m, b->x, h + 1.8f, b->z, 0.35f, 3.2f, 0.35f, 0.8f, 0.1f, 0.1f);

    // ventanas en las 4 caras
    for (int lado = 0; lado < 4; lado++) {
        int horiz = (lado < 2);
        float largo = horiz ? b->w : b->d;
        float centro = horiz ? b->x : b->z;
        float fijo = (lado == 0) ? b->z + b->d * 0.5f + 0.02f :
                     (lado == 1) ? b->z - b->d * 0.5f - 0.02f :
                     (lado == 2) ? b->x + b->w * 0.5f + 0.02f : b->x - b->w * 0.5f - 0.02f;
        int n = (int)floorf((largo - 1.0f + hueco) / (ww + hueco));
        if (n < 1) continue;
        float total = n * ww + (n - 1) * hueco;
        float u0 = centro - total * 0.5f;
        int nivel = 0;
        for (float y = 1.9f; y + 1.2f <= h - 0.6f; y += 2.2f, nivel++) {
            for (int k = 0; k < n; k++) {
                // la puerta ocupa el hueco del medio en la planta baja del frente
                unsigned q = azar2(lado * 131 + k * 17 + (int)(b->x * 3), nivel * 29 + (int)(b->z * 3));
                float r = 0.20f, g = 0.30f, bl = 0.40f;
                if (vidrio) { r = 0.50f; g = 0.82f; bl = 0.92f; if (q % 5 == 0) { r = 0.70f; g = 0.92f; bl = 0.98f; } }
                else if (q % 6 == 0) { r = 0.98f; g = 0.86f; bl = 0.50f; }
                ventana(m, lado, u0 + k * (ww + hueco), y, y + 1.2f, ww, fijo, r, g, bl);
            }
        }
    }
    // puerta y toldito en el frente (+z)
    if (b->w >= 4.0f) {
        float zf = b->z + b->d * 0.5f;
        ventana(m, 0, b->x - 0.6f, 0.0f, 1.9f, 1.2f, zf + 0.03f, 0.32f, 0.20f, 0.12f);
        malla_caja(m, b->x, 2.15f, zf + 0.4f, 2.4f, 0.18f, 0.8f, 0.82f, 0.22f, 0.20f);
    }
}

static void dibujar_bloque(Bloque *b, const TipoEdificio *e) {
    if (!b->listo) {
        if (bakes_restantes <= 0) return;   // se arma en el próximo cuadro
        bakes_restantes--;
        armar_fachada(b, e);
        b->listo = 1;
    }
    malla_dibujar(&b->malla);
}

static void dibujar_arbol(float x, float z) {
    unsigned q = azar2((int)(x * 4), (int)(z * 4));
    float s = 0.9f + (q % 5) * 0.05f;
    glPushMatrix();
    glTranslatef(x, 0.0f, z);
    glRotatef((float)((q >> 3) % 4) * 90.0f, 0.0f, 1.0f, 0.0f);
    glScalef(s, s, s);
    malla_dibujar(&arbol_malla[(q >> 6) % 3]);
    glPopMatrix();
}

void ciudad_dibujar(float px, float pz) {
    bakes_restantes = BAKES_POR_CUADRO;
    for (int i = 0; i < n_bloques; i++) {
        Bloque *b = &bloques[i];
        const TipoEdificio *e = edificio_de(b->tipo);
        if (e == NULL) continue;
        if (!cerca(b, px, pz, RADIO_GRANDE)) continue;
        dibujar_bloque(b, e);
    }
    for (int i = 0; i < n_arboles; i++) {
        if (fabsf(arboles[i][0] - px) > RADIO_ARBOL || fabsf(arboles[i][1] - pz) > RADIO_ARBOL) continue;
        dibujar_arbol(arboles[i][0], arboles[i][1]);
    }
    lugares_dibujar(px, pz);
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
    return lugares_choca(x, z, radio);
}

float ciudad_altura_en(float x, float z) {
    int col = (int)floorf(x / CIUDAD_TAM + ancho * 0.5f);
    int fila = (int)floorf(z / CIUDAD_TAM + alto * 0.5f);
    char t = tipo_en(col, fila);
    float h = 0.0f;
    const TipoEdificio *e = edificio_de(t);
    if (e) h = e->h + 0.8f;
    else if (t == 'T') h = 2.4f;
    float hl = lugares_altura(x, z);
    return hl > h ? hl : h;
}
