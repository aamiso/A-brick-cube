#include "world/misiones.h"
#include "world/ciudad.h"
#include "world/lugares.h"
#include "world/vehiculos.h"
#include "engine/cubo.h"
#include "ui/ui.h"
#include <math.h>
#include <stdio.h>

typedef enum { M_ENTREGA, M_CARRERA, M_DESTRUCCION } Tipo;
static const struct { char l; const char *n; } NOMBRES[] = {
    {'H', "la Casa roja"}, {'J', "la Casa azul"}, {'K', "la Casa moderna"}, {'X', "el Banco"}, {'Q', "el Café"}, {'Y', "el Hospital"},
    {'R', "el Edificio rojo"}, {'V', "la Torre Dorada"}, {'M', "la Farmacia"}, {'U', "el Hotel"}, {'O', "las Torres Gemelas"},
    {'I', "la Oficina"}, {'Z', "el Edificio colorido"}, {'s', "el Centro comercial"}, {'c', "la Comisaría"}};
static const char *nombre_de(char l) { for (unsigned i = 0; i < sizeof(NOMBRES) / sizeof(NOMBRES[0]); i++) if (NOMBRES[i].l == l) return NOMBRES[i].n; return "un lugar"; }

static int activa = 0, dinero = 0, oferta_ok = 0;
static Tipo tipo;
static float tx, tz;               // destino
static int pago, tiempo;           // pago y cuadros que quedan
static int choques_ini, choques_meta;
static char texto[160], titulo[96];
static int cuenta = 0, msg_t = 0, msg_ok = 0;
static char msg[64];
static unsigned semilla = 99u;
static unsigned azar(void) { semilla = semilla * 1664525u + 1013904223u; return semilla >> 8; }

// Un punto de acera cerca de (x,z) para poner el destino
static int acera_cerca(float x, float z, float *ox, float *oz) {
    float mejor = 1e9f; int ok = 0;
    for (int dz = -34; dz <= 34; dz++)
        for (int dx = -34; dx <= 34; dx++) {
            float cx = x + dx, cz = z + dz;
            int c = (int)floorf(cx / CIUDAD_TAM + ciudad_ancho() * 0.5f), f = (int)floorf(cz / CIUDAD_TAM + ciudad_alto() * 0.5f);
            if (ciudad_tipo_en(c, f) != 'S') continue;
            float d = (float)(dx * dx + dz * dz);
            if (d < mejor && !ciudad_choca(ciudad_centro_x(c), ciudad_centro_z(f), 0.6f)) { mejor = d; *ox = ciudad_centro_x(c); *oz = ciudad_centro_z(f); ok = 1; }
        }
    return ok;
}

int mision_activa(void) { return activa; }
int mision_dinero(void) { return dinero; }
const char *mision_oferta(void) { return texto; }

int mision_preparar(float px, float pz) {
    oferta_ok = 0;
    int n = lugares_cantidad(); if (n <= 0) return 0;
    int tipo_n = (int)(azar() % 3);
    if (tipo_n == 2) {
        tipo = M_DESTRUCCION; choques_meta = 3; pago = 250; tiempo = 120 * 60;
        snprintf(titulo, sizeof(titulo), "Destrucción: choca fuerte 3 veces");
        snprintf(texto, sizeof(texto), "Destrucción: sube a un carro y choca fuerte 3 veces en 2 minutos. Pago $%d.", pago);
        oferta_ok = 1; return 1;
    }
    for (int t = 0; t < 60; t++) {
        char l; float x, z;
        if (!lugares_info((int)(azar() % (unsigned)n), &l, &x, &z)) continue;
        float d = hypotf(x - px, z - pz);
        if (d < 50.0f || d > 170.0f) continue;
        if (!acera_cerca(x, z, &tx, &tz)) continue;
        d = hypotf(tx - px, tz - pz);
        tipo = tipo_n == 0 ? M_ENTREGA : M_CARRERA;
        pago = 100 + (int)(d * (tipo == M_CARRERA ? 1.6f : 1.2f));
        tiempo = (int)((d / (tipo == M_CARRERA ? 16.0f : 6.5f) + 25.0f) * 60.0f);
        if (tipo == M_ENTREGA) {
            snprintf(titulo, sizeof(titulo), "Entrega: destino %s", nombre_de(l));
            snprintf(texto, sizeof(texto), "Entrega: lleva este paquete. Destino: %s (%d m, %d s). Pago $%d.", nombre_de(l), (int)d, tiempo / 60, pago);
        } else {
            snprintf(titulo, sizeof(titulo), "Carrera en carro: destino %s", nombre_de(l));
            snprintf(texto, sizeof(texto), "Carrera: llega en carro. Destino: %s (%d m, %d s). Pago $%d.", nombre_de(l), (int)d, tiempo / 60, pago);
        }
        oferta_ok = 1; return 1;
    }
    return 0;
}

void mision_aceptar(void) {
    if (!oferta_ok) return;
    activa = 1; oferta_ok = 0; choques_ini = vehiculos_choques();
}

static void terminar(int ok) {
    activa = 0; msg_t = 200; msg_ok = ok;
    if (ok) { dinero += pago; snprintf(msg, sizeof(msg), "¡Misión cumplida! +$%d", pago); cuenta++; }
    else snprintf(msg, sizeof(msg), "Misión fallida: se acabó el tiempo");
}

void mision_actualizar(float x, float z, int en_carro) {
    if (msg_t > 0) msg_t--;
    if (!activa) return;
    if (--tiempo <= 0) { terminar(0); return; }
    if (tipo == M_DESTRUCCION) { if (vehiculos_choques() - choques_ini >= choques_meta) terminar(1); return; }
    float d = hypotf(x - tx, z - tz);
    if (d < (tipo == M_CARRERA ? 5.5f : 3.5f) && (tipo == M_ENTREGA || en_carro)) terminar(1);
}

int mision_marca(float *x, float *z) {
    if (!activa || tipo == M_DESTRUCCION) return 0;
    *x = tx; *z = tz; return 1;
}

void mision_dibujar_3d(float x, float z) {
    if (!activa || tipo == M_DESTRUCCION) return;
    if (fabsf(tx - x) > 150.0f || fabsf(tz - z) > 150.0f) return;
    cubo_activar();
    caja(tx, 12.0f, tz, 0.5f, 24.0f, 0.5f, 1.0f, 0.85f, 0.1f);       // rayo amarillo
    caja(tx, 0.1f, tz, 3.2f, 0.2f, 3.2f, 1.0f, 0.85f, 0.1f);
    caja(tx, 25.0f, tz, 1.4f, 1.4f, 1.4f, 1.0f, 0.95f, 0.3f);
}

void mision_dibujar_hud(void) {
    char t[64];
    snprintf(t, sizeof(t), "$ %d", dinero);
    ui_rect(14, 50, 88, 26, 0x000000B4); ui_texto(22, 54, 1, t, 0xFFE860FF);
    if (activa) {
        int w = (int)ui_ancho(titulo, 1) + 28; if (w < 300) w = 300;
        ui_rect(480 - w / 2, 14, w, 56, 0x000000B4);
        ui_texto(480 - w / 2 + 14, 20, 1, titulo, 0xFFE860FF);
        int s = tiempo / 60;
        if (tipo == M_DESTRUCCION) snprintf(t, sizeof(t), "Choques %d/%d · Tiempo %d:%02d", vehiculos_choques() - choques_ini, choques_meta, s / 60, s % 60);
        else snprintf(t, sizeof(t), "Tiempo %d:%02d", s / 60, s % 60);
        ui_texto(480 - w / 2 + 14, 42, 1, t, s < 10 ? 0xFF6060FF : 0xFFFFFFFF);
    }
    if (msg_t > 0) {
        int w = (int)ui_ancho(msg, 1) + 40;
        ui_rect(480 - w / 2, 90, w, 40, msg_ok ? 0x1d7a2ee0 : 0x9a2020e0);
        ui_texto(480 - w / 2 + 20, 100, 1, msg, 0xFFFFFFFF);
    }
}
