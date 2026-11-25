#include "world/lugares.h"
#include "engine/voxel.h"
#include "engine/malla.h"
#include "engine/cubo.h"
#include <vitaGL.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LADO        0.5f     // cuánto mide cada cubito en el mundo (media celda)
#define MAX_LUGARES 1500
#define RADIO_MODELO 72.0f   // hasta dónde se dibuja el modelo completo
#define RADIO_CAJA   115.0f  // más lejos se dibuja una cajita simple
#define CAPA_PARED   2       // capa donde se mira si hay pared (choque)

typedef struct {
    char letra;
    const char *archivo;
    Voxeles vox;
    Malla malla;
    unsigned char *solido;   // por columna de cubitos: 1 si choca
    float *altura;           // por columna: altura en el mundo
    int cargado;
    float alto_max;
    float r, g, b;           // color promedio (para la cajita de lejos)
} Tipo;

static Tipo tipos[] = {
    {'H', "casa_roja.txt",    {0}, {0}, 0, 0, 0, 0, 0, 0, 0},
    {'J', "casa_azul.txt",    {0}, {0}, 0, 0, 0, 0, 0, 0, 0},
    {'K', "casa_moderna.txt", {0}, {0}, 0, 0, 0, 0, 0, 0, 0},
    {'X', "banco.txt",        {0}, {0}, 0, 0, 0, 0, 0, 0, 0},
    {'Q', "cafe.txt",         {0}, {0}, 0, 0, 0, 0, 0, 0, 0},
    {'Y', "hospital.txt",     {0}, {0}, 0, 0, 0, 0, 0, 0, 0},
    {'R', "edificio_rojo.txt",{0}, {0}, 0, 0, 0, 0, 0, 0, 0},
    {'V', "torre_dorada.txt", {0}, {0}, 0, 0, 0, 0, 0, 0, 0},
    {'M', "farmacia.txt",     {0}, {0}, 0, 0, 0, 0, 0, 0, 0},
    {'U', "hotel.txt",        {0}, {0}, 0, 0, 0, 0, 0, 0, 0},
    {'O', "torres_gemelas.txt",   {0}, {0}, 0, 0, 0, 0, 0, 0, 0},
    {'I', "oficina_moderna.txt",  {0}, {0}, 0, 0, 0, 0, 0, 0, 0},
    {'Z', "edificio_colorido.txt",{0}, {0}, 0, 0, 0, 0, 0, 0, 0},
    {'s', "centro_comercial.txt", {0}, {0}, 0, 0, 0, 0, 0, 0, 0},
    {'c', "comisaria.txt",        {0}, {0}, 0, 0, 0, 0, 0, 0, 0},
    {'q', "comisaria.txt",        {0}, {0}, 0, 0, 0, 0, 0, 0, 0},
};
#define N_TIPOS ((int)(sizeof(tipos) / sizeof(tipos[0])))

typedef struct { int tipo; float x, z, w, d; } Lugar;
static Lugar lugares[MAX_LUGARES];
static int n_lugares = 0;

static int buscar(char letra) {
    for (int i = 0; i < N_TIPOS; i++) if (tipos[i].letra == letra) return i;
    return -1;
}

int lugar_es(char letra) { return buscar(letra) >= 0; }
int lugares_cantidad(void) { return n_lugares; }
int lugares_info(int i, char *letra, float *x, float *z) {
    if (i < 0 || i >= n_lugares) return 0;
    *letra = tipos[lugares[i].tipo].letra; *x = lugares[i].x; *z = lugares[i].z;
    return 1;
}
void lugares_limpiar(void) { n_lugares = 0; }

void lugares_cargar_modelos(void) {
    for (int i = 0; i < N_TIPOS; i++) {
        Tipo *t = &tipos[i];
        if (t->cargado) continue;
        char ruta[256];
        snprintf(ruta, sizeof(ruta), "%s%s", RUTA_MODELOS, t->archivo);
        if (!voxel_cargar(&t->vox, ruta)) continue;

        Voxeles *vx = &t->vox;
        // La capa 0 es el suelo: su parte de arriba queda justo a la altura 0
        malla_iniciar(&t->malla);
        voxel_malla(vx, 1.0f, vx->W * 0.5f, 1.0f, vx->D * 0.5f, &t->malla);
        malla_subir(&t->malla);

        t->solido = (unsigned char *)calloc((size_t)vx->W * vx->D, 1);
        t->altura = (float *)calloc((size_t)vx->W * vx->D, sizeof(float));
        double sr = 0, sg = 0, sb = 0, cuenta = 0;
        t->alto_max = 0;
        for (int z = 0; z < vx->D; z++) {
            for (int x = 0; x < vx->W; x++) {
                int top = -1;
                for (int y = 0; y < vx->H; y++) {
                    if (!voxel_hay(vx, x, y, z)) continue;
                    top = y;
                    if (y >= 1) {
                        unsigned char l = vx->v[((size_t)y * vx->D + z) * vx->W + x];
                        unsigned int rgb = vx->hay[l] ? vx->pal[l] : 0x808080;
                        sr += (rgb >> 16) & 255; sg += (rgb >> 8) & 255; sb += rgb & 255; cuenta++;
                    }
                }
                float h = (top > 0) ? top * LADO : 0.0f;
                t->altura[z * vx->W + x] = h;
                if (h > t->alto_max) t->alto_max = h;
                if (voxel_hay(vx, x, CAPA_PARED, z)) t->solido[z * vx->W + x] = 1;
            }
        }
        if (cuenta > 0) { t->r = (float)(sr / cuenta / 255.0); t->g = (float)(sg / cuenta / 255.0); t->b = (float)(sb / cuenta / 255.0); }
        else { t->r = t->g = t->b = 0.6f; }
        t->cargado = 1;
    }
}

void lugares_agregar(char letra, float x, float z, float w, float d) {
    int t = buscar(letra);
    if (t < 0 || n_lugares >= MAX_LUGARES) return;
    Lugar *l = &lugares[n_lugares++];
    l->tipo = t; l->x = x; l->z = z; l->w = w; l->d = d;
}

void lugares_dibujar(float px, float pz) {
    for (int i = 0; i < n_lugares; i++) {
        const Lugar *l = &lugares[i];
        Tipo *t = &tipos[l->tipo];
        float dx = fabsf(l->x - px), dz = fabsf(l->z - pz);
        float mx = l->w * 0.5f, mz = l->d * 0.5f;
        if (t->cargado) {
            if (dx < RADIO_MODELO + mx && dz < RADIO_MODELO + mz) {
                glPushMatrix();
                glTranslatef(l->x, 0.0f, l->z);
                glScalef(LADO, LADO, LADO);
                malla_dibujar(&t->malla);
                glPopMatrix();
            }
        } else if (dx < RADIO_CAJA + mx && dz < RADIO_CAJA + mz) {
            // No se encontró el modelo: una cajita de color para no dejar un hueco
            caja(l->x, 2.0f, l->z, l->w * 0.7f, 4.0f, l->d * 0.7f, 0.8f, 0.4f, 0.3f);
        }
    }
}

// Rango de columnas de cubitos que toca un círculo
static int cols(const Lugar *l, const Tipo *t, float x, float r, int *c0, int *c1, int eje_z) {
    float centro = eje_z ? l->z : l->x;
    int n = eje_z ? t->vox.D : t->vox.W;
    *c0 = (int)floorf((x - r - centro) / LADO + n * 0.5f);
    *c1 = (int)floorf((x + r - centro) / LADO + n * 0.5f);
    if (*c0 < 0) *c0 = 0;
    if (*c1 > n - 1) *c1 = n - 1;
    return *c0 <= *c1;
}

#define PASO_MAX 0.35f   // un escalón de hasta esto se sube caminando

// 1 si algo sólido toca el círculo y sobresale por encima de la altura "y" + un escalón
int lugares_choca_y(float x, float z, float radio, float y) {
    for (int i = 0; i < n_lugares; i++) {
        const Lugar *l = &lugares[i];
        const Tipo *t = &tipos[l->tipo];
        if (!t->cargado) continue;
        float mx = t->vox.W * LADO * 0.5f, mz = t->vox.D * LADO * 0.5f;
        if (fabsf(x - l->x) > mx + radio || fabsf(z - l->z) > mz + radio) continue;
        int c0, c1, f0, f1;
        if (!cols(l, t, x, radio, &c0, &c1, 0)) continue;
        if (!cols(l, t, z, radio, &f0, &f1, 1)) continue;
        for (int f = f0; f <= f1; f++) {
            for (int c = c0; c <= c1; c++) {
                if (!t->solido[f * t->vox.W + c]) continue;
                if (t->altura[f * t->vox.W + c] <= y + PASO_MAX) continue;   // se puede subir encima
                float cx = l->x + (c + 0.5f - t->vox.W * 0.5f) * LADO;
                float cz = l->z + (f + 0.5f - t->vox.D * 0.5f) * LADO;
                float h = LADO * 0.5f;
                float qx = fminf(fmaxf(x, cx - h), cx + h);
                float qz = fminf(fmaxf(z, cz - h), cz + h);
                float ddx = x - qx, ddz = z - qz;
                if (ddx * ddx + ddz * ddz < radio * radio) return 1;
            }
        }
    }
    return 0;
}

int lugares_choca(float x, float z, float radio) { return lugares_choca_y(x, z, radio, 0.0f); }

// Altura del piso bajo el círculo: lo más alto que se puede pisar (hasta y + un escalón)
float lugares_piso(float x, float z, float radio, float y) {
    float mejor = 0.0f;
    for (int i = 0; i < n_lugares; i++) {
        const Lugar *l = &lugares[i];
        const Tipo *t = &tipos[l->tipo];
        if (!t->cargado) continue;
        float mx = t->vox.W * LADO * 0.5f, mz = t->vox.D * LADO * 0.5f;
        if (fabsf(x - l->x) > mx + radio || fabsf(z - l->z) > mz + radio) continue;
        int c0, c1, f0, f1;
        if (!cols(l, t, x, radio, &c0, &c1, 0)) continue;
        if (!cols(l, t, z, radio, &f0, &f1, 1)) continue;
        for (int f = f0; f <= f1; f++)
            for (int c = c0; c <= c1; c++) {
                if (!t->solido[f * t->vox.W + c]) continue;
                float h = t->altura[f * t->vox.W + c];
                if (h <= y + PASO_MAX && h > mejor) mejor = h;
            }
    }
    return mejor;
}

float lugares_altura(float x, float z) {
    float mejor = 0.0f;
    for (int i = 0; i < n_lugares; i++) {
        const Lugar *l = &lugares[i];
        const Tipo *t = &tipos[l->tipo];
        if (!t->cargado) continue;
        float mx = t->vox.W * LADO * 0.5f, mz = t->vox.D * LADO * 0.5f;
        if (fabsf(x - l->x) >= mx || fabsf(z - l->z) >= mz) continue;
        int c = (int)floorf((x - l->x) / LADO + t->vox.W * 0.5f);
        int f = (int)floorf((z - l->z) / LADO + t->vox.D * 0.5f);
        if (c < 0 || c >= t->vox.W || f < 0 || f >= t->vox.D) continue;
        float h = t->altura[f * t->vox.W + c];
        if (h > mejor) mejor = h;
    }
    return mejor;
}

char lugar_entrada_cerca(float x, float z, float alcance) {
    for (int i = 0; i < n_lugares; i++) {
        const Lugar *l = &lugares[i];
        char ch = tipos[l->tipo].letra;
        if (ch != 'Y' && ch != 'Q' && ch != 'M' && ch != 'X' && ch != 'q') continue;
        float dx = fabsf(x - l->x) - l->w * 0.5f, dz = fabsf(z - l->z) - l->d * 0.5f;
        if (dx < 0) dx = 0;
        if (dz < 0) dz = 0;
        if (dx * dx + dz * dz < alcance * alcance) return ch;
    }
    return 0;
}
