#include "world/ciudad.h"
#include "world/lugares.h"
#include "engine/cubo.h"
#include "engine/malla.h"
#include "engine/voxel.h"
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

#ifdef __3DS__
#define RADIO_GRANDE  40.0f
#define RADIO_BORRA   72.0f
#define BAKES_POR_CUADRO 2
#else
#define RADIO_GRANDE  64.0f
#define RADIO_BORRA   120.0f
#define BAKES_POR_CUADRO 1
#endif

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
} Bloque;

static Bloque bloques[MAX_BLOQUES];
static int n_bloques = 0;
static float arboles[MAX_ARBOLES][2];
static int n_arboles = 0;
static int bakes_restantes = 0;

// Los edificios y árboles se arman en grupos de 32 x 32 celdas: cada grupo es UNA sola malla
// guardada en la memoria de video (así se dibuja con una sola llamada).
#define GCH   16
#define MAX_G 26
typedef struct { Malla m; char listo; } Grupo;
static Grupo grupos[MAX_G][MAX_G];
static Malla lejano;            // calles, aceras y agua de todo el mapa (una sola malla)
static int lejano_listo = 0;

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

static void liberar_mallas(void) {
    for (int f = 0; f < MAX_G; f++)
        for (int c = 0; c < MAX_G; c++)
            if (grupos[f][c].listo) { malla_liberar(&grupos[f][c].m); grupos[f][c].listo = 0; }
    if (lejano_listo) { malla_liberar(&lejano); lejano_listo = 0; }
}

static void limpiar_mapa(void) {
    liberar_mallas();
    memset(mapa, '.', sizeof(mapa));
    ancho = alto = 0;
    n_bloques = n_arboles = 0;
    lugares_limpiar();
}

// ---------- Preparar el mapa para dibujarlo ----------

// Calles, aceras y agua de todo el mapa en una sola malla (se pinta de lejos y de cerca)
static void armar_lejano(void) {
    malla_iniciar(&lejano);
    for (int pasada = 0; pasada < 3; pasada++) {
        char quiero = (pasada == 0) ? 'W' : (pasada == 1) ? '=' : 'S';
        for (int i = 0; i < n_bloques; i++) {
            const Bloque *b = &bloques[i];
            if (b->tipo != quiero) continue;
            if (quiero == 'W')      malla_tapa(&lejano, b->x, 0.03f, b->z, b->w, b->d, 0.18f, 0.48f, 0.86f);
            else if (quiero == '=') malla_tapa(&lejano, b->x, 0.03f, b->z, b->w, b->d, 0.30f, 0.31f, 0.34f);
            else                    malla_tapa(&lejano, b->x, 0.03f, b->z, b->w, b->d, 0.72f, 0.71f, 0.68f);
        }
    }
    malla_subir(&lejano);
    lejano_listo = 1;
}


static void armar_bloques(void) {
    static unsigned char usada[MAX_ALTO][MAX_ANCHO];
    memset(usada, 0, sizeof(usada));
    liberar_mallas();
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
            }
        }
    }

    armar_lejano();

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

// Árboles de cubitos (cada uno con su propio diseño, como en las referencias)
#define N_ARBOLES 4
static Malla arbol_malla[N_ARBOLES];
static int arboles_listos = 0;

static void arbol_hacer(int n, void (*forma)(Voxeles *)) {
    Voxeles v;
    voxel_crear(&v, 13, 13, 18);
    voxel_color(&v, 't', 0x6b4a2f); voxel_color(&v, 'T', 0x8a6a42);
    voxel_color(&v, 'a', 0x7ccd2e); voxel_color(&v, 'b', 0x5fb524); voxel_color(&v, 'c', 0x3f9a3a);
    voxel_color(&v, 'd', 0x2e7d3a); voxel_color(&v, 'e', 0x1f6a34); voxel_color(&v, 'w', 0xe8d9b8); voxel_color(&v, 'x', 0x8d7a5e);
    voxel_color(&v, 'f', 0xf4a6c8);
    forma(&v);
    // sin huecos: cada columna se rellena entre su cubito más bajo y el más alto
    for (int z = 0; z < v.D; z++)
        for (int x = 0; x < v.W; x++) {
            int top = -1;
            for (int y = 0; y < v.H; y++) if (voxel_hay(&v, x, y, z)) top = y;
            char abajo = 0;
            for (int y = 0; y <= top; y++) {
                if (voxel_hay(&v, x, y, z)) abajo = (char)v.v[((size_t)y * v.D + z) * v.W + x];
                else if (abajo) voxel_poner(&v, x, y, z, abajo);
            }
        }
    malla_iniciar(&arbol_malla[n]);
    voxel_malla(&v, 0.25f, 6.5f, 0.0f, 6.5f, &arbol_malla[n]);
    voxel_liberar(&v);
}
// 0: copa de cubos grandes (árbol de bloques)
static void forma0(Voxeles *v) {
    voxel_caja(v, 5, 0, 5, 3, 5, 3, 't');
    voxel_caja(v, 4, 4, 4, 5, 2, 5, 'x');
    voxel_caja(v, 2, 6, 2, 9, 5, 9, 'a');
    voxel_caja(v, 3, 11, 3, 7, 2, 7, 'b');
    voxel_caja(v, 1, 7, 5, 2, 3, 4, 'c');
    voxel_caja(v, 10, 8, 4, 2, 3, 4, 'c');
}
// 1: pino por pisos
static void forma1(Voxeles *v) {
    voxel_caja(v, 6, 0, 6, 2, 3, 2, 't');
    voxel_caja(v, 1, 3, 1, 11, 2, 11, 'e');
    voxel_caja(v, 2, 5, 2, 9, 2, 9, 'd');
    voxel_caja(v, 3, 7, 3, 7, 2, 7, 'c');
    voxel_caja(v, 4, 9, 4, 5, 2, 5, 'b');
    voxel_caja(v, 5, 11, 5, 3, 2, 3, 'a');
    voxel_caja(v, 6, 13, 6, 1, 1, 1, 'a');
}
// 2: abedul: tronco claro con rayas y copa cúbica
static void forma2(Voxeles *v) {
    for (int y = 0; y < 9; y++) voxel_caja(v, 6, y, 6, 2, 1, 2, (y % 3 == 1) ? 'x' : 'w');
    voxel_caja(v, 3, 9, 3, 8, 7, 8, 'a');
    voxel_caja(v, 4, 16, 4, 6, 1, 6, 'b');
}
// 3: roble pequeño con flores
static void forma3(Voxeles *v) {
    voxel_caja(v, 5, 0, 5, 3, 4, 3, 't');
    voxel_caja(v, 3, 4, 5, 2, 3, 3, 't'); voxel_caja(v, 8, 4, 5, 2, 3, 3, 't');
    voxel_caja(v, 1, 6, 3, 6, 4, 7, 'b'); voxel_caja(v, 7, 6, 3, 6, 4, 7, 'a');
    voxel_caja(v, 3, 9, 4, 7, 3, 6, 'c');
    voxel_poner(v, 2, 7, 2, 'f'); voxel_poner(v, 10, 8, 2, 'f'); voxel_poner(v, 5, 11, 3, 'f'); voxel_poner(v, 11, 6, 6, 'f');
}
static void armar_arboles(void) {
    if (arboles_listos) return;
    arbol_hacer(0, forma0); arbol_hacer(1, forma1); arbol_hacer(2, forma2); arbol_hacer(3, forma3);
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
            char t = linea[c];
            if (!lugar_es(t)) t = (char)toupper((unsigned char)t);      // las letras de lugares (s, c) se respetan en minúscula
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

void ciudad_dibujar_suelo(float px, float pz) {
    (void)px; (void)pz;
    if (lejano_listo) malla_dibujar(&lejano);
}

// ---------- Edificios con ventanas ----------

// ---------- Edificios hechos de cubitos (voxels) ----------

static unsigned int rgb_de(float r, float g, float b) {
    if (r > 1) r = 1; if (g > 1) g = 1; if (b > 1) b = 1;
    return ((unsigned)(r * 255) << 16) | ((unsigned)(g * 255) << 8) | (unsigned)(b * 255);
}

// ¿Es una cara exterior (hacia los lados)? devuelve 1..4 según el lado, 0 si no
static int cara_lado(const Voxeles *v, int x, int y, int z) {
    if (!voxel_hay(v, x, y, z + 1)) return 1;   // +z (frente)
    if (!voxel_hay(v, x, y, z - 1)) return 2;
    if (!voxel_hay(v, x + 1, y, z)) return 3;
    if (!voxel_hay(v, x - 1, y, z)) return 4;
    return 0;
}

// Huecos que dejan los choques en los edificios
#define MAX_HUECOS 64
typedef struct { float x, y, z, r; } Hueco;
static Hueco huecos[MAX_HUECOS];
static int n_huecos = 0;

static void armar_fachada(Malla *m, const Bloque *b, const TipoEdificio *e) {
    float lado = (e->h >= 14.0f) ? 1.0f : 0.5f;
    int vidrio = (e->letra == 'E');
    int nx, nz, ny;
    for (;;) {
        nx = (int)(b->w / lado); nz = (int)(b->d / lado); ny = (int)(e->h / lado);
        if (nx < 2) nx = 2;
        if (nz < 2) nz = 2;
        if (nx <= 100 && nz <= 100) break;
        lado *= 2.0f;
    }
    const int M = 2, EXTRA = 8;
    Voxeles v;
    voxel_crear(&v, nx + 2 * M, nz + 2 * M, ny + EXTRA);
    voxel_color(&v, 'c', rgb_de(e->r, e->g, e->b));
    voxel_color(&v, 't', rgb_de(e->r * 1.15f + 0.05f, e->g * 1.15f + 0.05f, e->b * 1.15f + 0.05f));
    voxel_color(&v, 'd', rgb_de(e->r * 0.45f, e->g * 0.45f, e->b * 0.45f));
    voxel_color(&v, 'r', rgb_de(e->r * 0.7f, e->g * 0.7f, e->b * 0.7f));
    voxel_color(&v, 'w', vidrio ? 0x7fd0ec : 0x34506a);
    voxel_color(&v, 'y', vidrio ? 0xb4ecfa : 0xfadc80);
    voxel_color(&v, 'a', 0x8c949e);
    voxel_color(&v, 'p', 0x52331f);
    voxel_color(&v, 'n', 0xd13a32);
    voxel_color(&v, 'k', 0xe02020);

    int fino = (lado < 1.0f);
    int base = fino ? 2 : 1;                          // alto de la base oscura
    int pisos_t = (e->h >= 18.0f) ? (ny * 7 / 10) : ny; // los edificios altos se angostan arriba
    int ins = (nx > 8 && nz > 8) ? (nx / 6 > 3 ? 3 : nx / 6) : 0;
    // cuerpo
    voxel_caja(&v, M, 0, M, nx, pisos_t, nz, 'c');
    int top_y = pisos_t, top_x = M, top_z = M, top_w = nx, top_d = nz;
    if (pisos_t < ny && ins > 0) {
        voxel_caja(&v, M + ins, pisos_t, M + ins, nx - 2 * ins, ny - pisos_t, nz - 2 * ins, 'c');
        top_x = M + ins; top_z = M + ins; top_w = nx - 2 * ins; top_d = nz - 2 * ins; top_y = ny;
    } else if (pisos_t < ny) {
        voxel_caja(&v, M, pisos_t, M, nx, ny - pisos_t, nz, 'c');
        top_y = ny;
    }
    // cornisa a mitad (da la forma escalonada)
    if (pisos_t < ny && ins > 0)
        voxel_caja(&v, M - 1 + 0, pisos_t - 1, M - 1, nx + 2, 1, nz + 2, 't');

    // ventanas: se recorre cada cubito de pared
    for (int y = base; y < top_y - 1; y++)
        for (int z = 0; z < v.D; z++)
            for (int x = 0; x < v.W; x++) {
                if (!voxel_hay(&v, x, y, z) || v.v[((size_t)y * v.D + z) * v.W + x] != 'c') continue;
                int c = cara_lado(&v, x, y, z);
                if (!c) continue;
                int u = (c <= 2) ? x : z;
                int u0 = (c <= 2) ? M : M;
                int uu = u - u0;
                int largo = (c <= 2) ? nx : nz;
                if (uu < 1 || uu > largo - 2) continue;      // no en las esquinas
                if (uu < 0) continue;
                int col, fila;
                if (vidrio) { col = (uu % 4) != 0; fila = (y % 3) != 0; }
                else if (fino) { col = (uu % 3) != 0; fila = (y % 4) >= 2; }
                else { col = (uu % 2) == 1; fila = (y % 2) == 1; }
                if (!col || !fila) continue;
                unsigned q = azar2(x * 131 + z * 17 + (int)(b->x * 3), y * 29 + c + (int)(b->z * 3));
                char w = (q % 6 == 0) ? 'y' : 'w';
                voxel_poner(&v, x, y, z, w);
            }
    // base oscura (planta baja)
    for (int y = 0; y < base; y++)
        for (int z = M; z < M + nz; z++)
            for (int x = M; x < M + nx; x++)
                if (cara_lado(&v, x, y, z)) voxel_poner(&v, x, y, z, 'd');
    // techo con borde
    int tx0 = top_x, tz0 = top_z;
    for (int x = tx0; x < tx0 + top_w; x++)
        for (int z = tz0; z < tz0 + top_d; z++) {
            int borde = (x == tx0 || z == tz0 || x == tx0 + top_w - 1 || z == tz0 + top_d - 1);
            voxel_poner(&v, x, top_y - 1, z, borde ? 't' : 'r');
            if (borde) voxel_poner(&v, x, top_y, z, 't');
        }
    // cosas en el techo
    if (top_w >= 6 && top_d >= 6) {
        unsigned q = azar2((int)(b->x * 10), (int)(b->z * 10));
        int bw = fino ? 3 : 2;
        voxel_caja(&v, tx0 + 2 + (q % 2), top_y, tz0 + 2, bw, 2, bw, 'a');
        if (q & 4) voxel_caja(&v, tx0 + top_w - 2 - bw, top_y, tz0 + top_d / 2, bw, 1, bw + 1, 'a');
    }
    if (e->h >= 22.0f) voxel_caja(&v, tx0 + top_w / 2, top_y, tz0 + top_d / 2, 1, EXTRA - 1, 1, 'k');
    // puerta y toldo en el frente (+z)
    if (nx >= 4) {
        int dw = fino ? 3 : 2, dh = fino ? 4 : 2;
        int dx = M + nx / 2 - dw / 2, zf = M + nz - 1;
        voxel_caja(&v, dx, 0, zf, dw, dh, 1, 'p');
        voxel_caja(&v, dx - 1, dh, zf + 1, dw + 2, 1, 1, 'n');
    }

    for (int h = 0; h < n_huecos; h++) {                 // cubitos que se llevó un choque
        const Hueco *q = &huecos[h];
        if (fabsf(q->x - b->x) > b->w * 0.5f + 1.0f || fabsf(q->z - b->z) > b->d * 0.5f + 1.0f) continue;
        for (int y = 0; y < v.H; y++)
            for (int z = 0; z < v.D; z++)
                for (int x = 0; x < v.W; x++) {
                    float wx = b->x + (x + 0.5f - v.W * 0.5f) * lado, wz = b->z + (z + 0.5f - v.D * 0.5f) * lado, wy = (y + 0.5f) * lado;
                    float dx = wx - q->x, dy = wy - q->y, dz = wz - q->z;
                    if (dx * dx + dy * dy + dz * dz < q->r * q->r) voxel_poner(&v, x, y, z, '.');
                }
    }
    Malla tmp;
    malla_iniciar(&tmp);
    voxel_malla(&v, lado, v.W * 0.5f, 0.0f, v.D * 0.5f, &tmp);
    malla_agregar(m, &tmp, b->x, 0.0f, b->z, 1.0f, 0.0f);
    malla_liberar(&tmp);
    voxel_liberar(&v);
}

static void armar_grupo(int gc, int gf) {
    Grupo *g = &grupos[gf][gc];
    malla_iniciar(&g->m);
    for (int i = 0; i < n_bloques; i++) {
        const Bloque *b = &bloques[i];
        const TipoEdificio *e = edificio_de(b->tipo);
        if (e == NULL) continue;
        int c = (int)floorf(b->x / CIUDAD_TAM + ancho * 0.5f) / GCH;
        int f = (int)floorf(b->z / CIUDAD_TAM + alto * 0.5f) / GCH;
        if (c != gc || f != gf) continue;
        armar_fachada(&g->m, b, e);
    }
    for (int i = 0; i < n_arboles; i++) {
        float x = arboles[i][0], z = arboles[i][1];
        int c = (int)floorf(x / CIUDAD_TAM + ancho * 0.5f) / GCH;
        int f = (int)floorf(z / CIUDAD_TAM + alto * 0.5f) / GCH;
        if (c != gc || f != gf) continue;
        unsigned q = azar2((int)(x * 4), (int)(z * 4));
        float esc = 0.9f + (q % 5) * 0.05f;
        malla_agregar(&g->m, &arbol_malla[(q >> 6) % N_ARBOLES], x, 0.0f, z, esc, (float)((q >> 3) % 4) * 90.0f);
    }
    malla_subir(&g->m);
    g->listo = 1;
}

static void centro_grupo(int gc, int gf, float *cx, float *cz) {
    *cx = ((gc + 0.5f) * GCH - ancho * 0.5f) * CIUDAD_TAM;
    *cz = ((gf + 0.5f) * GCH - alto * 0.5f) * CIUDAD_TAM;
}

// Arma de antemano todo lo que está cerca (se usa al empezar, para que no haya tirones)
void ciudad_precalentar(float px, float pz) {
    int nc = (ancho + GCH - 1) / GCH, nf = (alto + GCH - 1) / GCH;
    if (nc > MAX_G) nc = MAX_G;
    if (nf > MAX_G) nf = MAX_G;
    for (int gf = 0; gf < nf; gf++)
        for (int gc = 0; gc < nc; gc++) {
            float cx, cz; centro_grupo(gc, gf, &cx, &cz);
            if (fmaxf(fabsf(cx - px), fabsf(cz - pz)) - GCH * 0.5f > RADIO_GRANDE) continue;
            if (!grupos[gf][gc].listo) armar_grupo(gc, gf);
        }
}

void ciudad_dibujar(float px, float pz, float giro) {
    bakes_restantes = BAKES_POR_CUADRO;
    float rad = giro * 3.14159265f / 180.0f, fx = sinf(rad), fz = -cosf(rad);
    int nc = (ancho + GCH - 1) / GCH, nf = (alto + GCH - 1) / GCH;
    if (nc > MAX_G) nc = MAX_G;
    if (nf > MAX_G) nf = MAX_G;
    for (int gf = 0; gf < nf; gf++) {
        for (int gc = 0; gc < nc; gc++) {
            Grupo *g = &grupos[gf][gc];
            float cx, cz; centro_grupo(gc, gf, &cx, &cz);
            float d = fmaxf(fabsf(cx - px), fabsf(cz - pz)) - GCH * 0.5f;
            if (d > RADIO_BORRA) { if (g->listo) { malla_liberar(&g->m); g->listo = 0; } continue; }
            if (d > RADIO_GRANDE) continue;
            // detrás de la cámara no hay nada que ver
            if ((cx - px) * fx + (cz - pz) * fz < -GCH * 0.8f - 16.0f) continue;
            if (!g->listo) {
                if (bakes_restantes <= 0) continue;
                bakes_restantes--;
                armar_grupo(gc, gf);
            }
            malla_dibujar(&g->m);
        }
    }
    lugares_dibujar(px, pz);
}

// Un choque fuerte arranca un pedazo del edificio que está en (x,z). Devuelve 1 y su color si había uno.
int ciudad_destruir(float x, float y, float z, float r, unsigned *color) {
    for (int i = 0; i < n_bloques; i++) {
        const Bloque *b = &bloques[i];
        if (fabsf(x - b->x) > b->w * 0.5f + 0.5f || fabsf(z - b->z) > b->d * 0.5f + 0.5f) continue;
        const TipoEdificio *e = edificio_de(b->tipo);
        if (e == NULL || y > e->h) return 0;
        if (n_huecos >= MAX_HUECOS) return 0;
        huecos[n_huecos++] = (Hueco){ x, y, z, r };
        *color = rgb_de(e->r, e->g, e->b);
        int c = (int)floorf(b->x / CIUDAD_TAM + ancho * 0.5f) / GCH;
        int f = (int)floorf(b->z / CIUDAD_TAM + alto * 0.5f) / GCH;
        if (grupos[f][c].listo) { malla_liberar(&grupos[f][c].m); grupos[f][c].listo = 0; }
        return 1;
    }
    return 0;
}

// ---------- Colisiones ----------

int ciudad_choca_y(float x, float z, float radio, float y) {
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
    return lugares_choca_y(x, z, radio, y);
}

int ciudad_choca(float x, float z, float radio) { return ciudad_choca_y(x, z, radio, 0.0f); }

float ciudad_piso(float x, float z, float radio, float y) { return lugares_piso(x, z, radio, y); }

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
