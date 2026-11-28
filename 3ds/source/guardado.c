// Guardar y cargar partida en la tarjeta SD de la 3DS
#include "guardado.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define CARPETA1 "sdmc:/3ds"
#define CARPETA2 "sdmc:/3ds/A-brick-cube"
#define ARCHIVO  "sdmc:/3ds/A-brick-cube/partida.dat"
#define VERSION  1

typedef struct {
    char magia[4];        // "ABCP"
    int version;
    float x, z, angulo, suelo;
    float hora;
    int dinero;
} Datos;

int guardado_escribir(const Partida *p) {
    mkdir(CARPETA1, 0777);
    mkdir(CARPETA2, 0777);
    Datos d;
    memset(&d, 0, sizeof(d));
    memcpy(d.magia, "ABCP", 4);
    d.version = VERSION;
    d.x = p->x; d.z = p->z; d.angulo = p->angulo; d.suelo = p->suelo;
    d.hora = p->hora; d.dinero = p->dinero;
    FILE *f = fopen(ARCHIVO, "wb");
    if (!f) return 0;
    int ok = fwrite(&d, sizeof(d), 1, f) == 1;
    if (fclose(f) != 0) ok = 0;
    return ok;
}

int guardado_leer(Partida *p) {
    FILE *f = fopen(ARCHIVO, "rb");
    if (!f) return 0;
    Datos d;
    int ok = fread(&d, sizeof(d), 1, f) == 1;
    fclose(f);
    if (!ok || memcmp(d.magia, "ABCP", 4) != 0 || d.version != VERSION) return 0;
    if (d.x != d.x || d.z != d.z) return 0;          // NaN: archivo dañado
    p->x = d.x; p->z = d.z; p->angulo = d.angulo; p->suelo = d.suelo;
    p->hora = d.hora; p->dinero = d.dinero;
    return 1;
}
