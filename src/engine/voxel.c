#include "engine/voxel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LADO 128
#define MAX_ALTO 200

static int hex1(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

void voxel_crear(Voxeles *vx, int W, int D, int H) {
    memset(vx, 0, sizeof(*vx));
    vx->W = W; vx->D = D; vx->H = H;
    vx->v = (unsigned char *)calloc((size_t)W * D * H, 1);
}

void voxel_liberar(Voxeles *vx) {
    free(vx->v);
    memset(vx, 0, sizeof(*vx));
}

void voxel_color(Voxeles *vx, char letra, unsigned int rgb) {
    vx->pal[(unsigned char)letra] = rgb;
    vx->hay[(unsigned char)letra] = 1;
}

void voxel_poner(Voxeles *vx, int x, int y, int z, char letra) {
    if (x < 0 || y < 0 || z < 0 || x >= vx->W || y >= vx->H || z >= vx->D) return;
    vx->v[((size_t)y * vx->D + z) * vx->W + x] = (unsigned char)(letra == '.' ? 0 : letra);
}

void voxel_caja(Voxeles *vx, int x0, int y0, int z0, int w, int h, int d, char letra) {
    for (int y = y0; y < y0 + h; y++)
        for (int z = z0; z < z0 + d; z++)
            for (int x = x0; x < x0 + w; x++) voxel_poner(vx, x, y, z, letra);
}

int voxel_hay(const Voxeles *vx, int x, int y, int z) {
    if (x < 0 || y < 0 || z < 0 || x >= vx->W || y >= vx->H || z >= vx->D) return 0;
    return vx->v[((size_t)y * vx->D + z) * vx->W + x] != 0;
}

// Lee el texto dos veces: la primera mide el modelo, la segunda llena los cubitos.
static int leer(Voxeles *vx, const char *texto, int llenar) {
    int W = 0, D = 0, capas = 0, fila = 0;
    int en_capa = 0;
    const char *p = texto;
    while (*p) {
        const char *fin = strchr(p, '\n');
        size_t largo = fin ? (size_t)(fin - p) : strlen(p);
        char linea[256];
        size_t n = largo < sizeof(linea) - 1 ? largo : sizeof(linea) - 1;
        memcpy(linea, p, n); linea[n] = '\0';
        p += largo; if (*p == '\n') p++;

        // recortar espacios de los lados
        char *a = linea;
        while (*a && isspace((unsigned char)*a)) a++;
        size_t l = strlen(a);
        while (l > 0 && isspace((unsigned char)a[l - 1])) a[--l] = '\0';
        if (l == 0 || (a[0] == '/' && a[1] == '/')) continue;

        if (strncmp(a, "modelo", 6) == 0 && (a[6] == '\0' || isspace((unsigned char)a[6]))) continue;
        if (strncmp(a, "color", 5) == 0 && isspace((unsigned char)a[5])) {
            const char *q = a + 5;
            while (*q && isspace((unsigned char)*q)) q++;
            char letra = *q ? *q++ : 0;
            while (*q && isspace((unsigned char)*q)) q++;
            if (*q == '#') q++;
            int ok = 1; unsigned int rgb = 0;
            for (int k = 0; k < 6; k++) {
                int h = hex1(q[k]);
                if (h < 0) { ok = 0; break; }
                rgb = rgb * 16 + (unsigned)h;
            }
            if (ok && letra && llenar) voxel_color(vx, letra, rgb);
            continue;
        }
        if (strncmp(a, "capa", 4) == 0 && (a[4] == '\0' || isspace((unsigned char)a[4]))) {
            capas++; en_capa = 1; fila = 0;
            continue;
        }
        if (!en_capa) continue;
        if ((int)l > W) W = (int)l;
        if (fila + 1 > D) D = fila + 1;
        if (llenar && capas - 1 < vx->H && fila < vx->D) {
            for (size_t k = 0; k < l && (int)k < vx->W; k++) {
                char c = a[k];
                if (c == '.' || isspace((unsigned char)c)) continue;
                voxel_poner(vx, (int)k, capas - 1, fila, c);
            }
        }
        fila++;
    }
    if (!llenar) {
        if (W <= 0 || D <= 0 || capas <= 0 || W > MAX_LADO || D > MAX_LADO || capas > MAX_ALTO) return 0;
        voxel_crear(vx, W, D, capas);
        return vx->v != NULL;
    }
    return 1;
}

int voxel_desde_texto(Voxeles *vx, const char *texto) {
    memset(vx, 0, sizeof(*vx));
    if (!leer(vx, texto, 0)) return 0;
    leer(vx, texto, 1);
    return 1;
}

int voxel_cargar(Voxeles *vx, const char *ruta) {
    memset(vx, 0, sizeof(*vx));
    FILE *f = fopen(ruta, "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (tam <= 0 || tam > 600000) { fclose(f); return 0; }
    char *buf = (char *)malloc((size_t)tam + 1);
    if (!buf) { fclose(f); return 0; }
    size_t leidos = fread(buf, 1, (size_t)tam, f);
    fclose(f);
    buf[leidos] = '\0';
    int ok = voxel_desde_texto(vx, buf);
    free(buf);
    return ok;
}

// ---------- Armar la malla ----------

typedef struct { int n[3]; float b; float v[4][3]; } Cara;
static const Cara CARAS[6] = {
    {{0, 1, 0},  BRILLO_ARRIBA, {{0,1,0},{1,1,0},{1,1,1},{0,1,1}}},
    {{0, 0, 1},  BRILLO_FRENTE, {{0,0,1},{1,0,1},{1,1,1},{0,1,1}}},
    {{0, 0, -1}, BRILLO_ATRAS,  {{0,0,0},{1,0,0},{1,1,0},{0,1,0}}},
    {{1, 0, 0},  BRILLO_DER,    {{1,0,0},{1,0,1},{1,1,1},{1,1,0}}},
    {{-1, 0, 0}, BRILLO_IZQ,    {{0,0,0},{0,0,1},{0,1,1},{0,1,0}}},
    {{0, -1, 0}, 0.60f,         {{0,0,0},{0,0,1},{1,0,1},{1,0,0}}},   // cara de abajo (se ve bajo voladizos y balcones)
};

// Une las caras vecinas del mismo color en rectángulos grandes (mucho menos triángulos).
void voxel_malla(const Voxeles *vx, float lado, float ox, float oy, float oz, Malla *m) {
    const int dim[3] = { vx->W, vx->H, vx->D };
    const float org[3] = { ox, oy, oz };
    static const int SA[6] = { 1, 2, 2, 0, 0, 1 }, UA[6] = { 0, 0, 0, 2, 2, 0 }, VA[6] = { 2, 1, 1, 1, 1, 2 };
    unsigned char *mask = (unsigned char *)malloc((size_t)(vx->W > vx->D ? vx->W : vx->D) * (vx->H > vx->D ? vx->H : vx->D) * 2 + 16);
    if (!mask) return;
    for (int f = 0; f < 6; f++) {
        const Cara *c = &CARAS[f];
        int sa = SA[f], ua = UA[f], va = VA[f];
        int nu = dim[ua], nv = dim[va];
        for (int sl = 0; sl < dim[sa]; sl++) {
            int idx[3];
            for (int v = 0; v < nv; v++)
                for (int u = 0; u < nu; u++) {
                    idx[sa] = sl; idx[ua] = u; idx[va] = v;
                    unsigned char l = vx->v[((size_t)idx[1] * vx->D + idx[2]) * vx->W + idx[0]];
                    if (l && voxel_hay(vx, idx[0] + c->n[0], idx[1] + c->n[1], idx[2] + c->n[2])) l = 0;
                    mask[v * nu + u] = l;
                }
            for (int v = 0; v < nv; v++)
                for (int u = 0; u < nu; u++) {
                    unsigned char l = mask[v * nu + u];
                    if (!l) continue;
                    int du = 1, dv = 1;
                    while (u + du < nu && mask[v * nu + u + du] == l) du++;
                    for (; v + dv < nv; dv++) {
                        int ok = 1;
                        for (int k = 0; k < du; k++) if (mask[(v + dv) * nu + u + k] != l) { ok = 0; break; }
                        if (!ok) break;
                    }
                    for (int j = 0; j < dv; j++) for (int k = 0; k < du; k++) mask[(v + j) * nu + u + k] = 0;
                    unsigned int rgb = vx->hay[l] ? vx->pal[l] : 0xff00ff;
                    float r = ((rgb >> 16) & 255) / 255.0f, g = ((rgb >> 8) & 255) / 255.0f, b = (rgb & 255) / 255.0f;
                    float q[4][3];
                    for (int k = 0; k < 4; k++) {
                        float pos[3];
                        pos[sa] = sl + c->v[k][sa];
                        pos[ua] = u + c->v[k][ua] * du;
                        pos[va] = v + c->v[k][va] * dv;
                        for (int e = 0; e < 3; e++) q[k][e] = (pos[e] - org[e]) * lado;
                    }
                    malla_quad(m, q[0], q[1], q[2], q[3], r * c->b, g * c->b, b * c->b);
                }
        }
    }
    free(mask);
}
