#include "ui/minimapa.h"
#include "ui/ui.h"
#include "world/ciudad.h"
#include "world/vehiculos.h"
#include <math.h>

#define R 38          // celdas a cada lado del jugador
#define ESC 2         // píxeles por celda
#define LADO (2 * R * ESC)

static unsigned color_de(char t) {
    switch (t) {
        case '=': return 0x3a3d44FF;
        case 'S': return 0xa4a8b0FF;
        case 'T': return 0x2f7a3aFF;
        case 'W': return 0x3a78c8FF;
        case '.': case 'P': return 0x5cb050FF;
        case 'Y': return 0xe05050FF;
        case 'V': return 0xe8b923FF;
        default:  return 0xb09a86FF;     // edificios
    }
}

void minimapa_dibujar(float px, float pz, float ang, int hay_marca, float mx, float mz) {
    float x0 = 960 - LADO - 14, y0 = 14;
    ui_rect(x0 - 3, y0 - 3, LADO + 6, LADO + 6, 0x000000C8);
    int cc = (int)floorf(px / CIUDAD_TAM + ciudad_ancho() * 0.5f), cf = (int)floorf(pz / CIUDAD_TAM + ciudad_alto() * 0.5f);
    for (int f = -R; f < R; f++) {
        int c = -R;
        while (c < R) {                                   // junta celdas iguales en una sola barra
            char t = ciudad_tipo_en(cc + c, cf + f);
            if (t == 0) t = 'T';
            unsigned col = color_de(t);
            int c2 = c + 1;
            while (c2 < R) { char u = ciudad_tipo_en(cc + c2, cf + f); if (u == 0) u = 'T'; if (color_de(u) != col) break; c2++; }
            ui_rect(x0 + (c + R) * ESC, y0 + (f + R) * ESC, (c2 - c) * ESC, ESC, col);
            c = c2;
        }
    }
    for (int i = 0; i < vehiculos_cantidad(); i++) {      // carros: puntitos rojos
        float vx, vz, a, v; vehiculo_pos(i, &vx, &vz, &a, &v);
        float dx = (vx - px) / CIUDAD_TAM * ESC, dz = (vz - pz) / CIUDAD_TAM * ESC;
        if (fabsf(dx) < LADO / 2 - 2 && fabsf(dz) < LADO / 2 - 2) ui_rect(x0 + LADO / 2 + dx - 1, y0 + LADO / 2 + dz - 1, 3, 3, 0xe03030FF);
    }
    if (hay_marca) {                                      // misión: cuadro amarillo (pegado al borde si está lejos)
        float dx = (mx - px) / CIUDAD_TAM * ESC, dz = (mz - pz) / CIUDAD_TAM * ESC, m = LADO / 2 - 5;
        if (dx > m) dx = m; if (dx < -m) dx = -m; if (dz > m) dz = m; if (dz < -m) dz = -m;
        ui_rect(x0 + LADO / 2 + dx - 3, y0 + LADO / 2 + dz - 3, 7, 7, 0x000000FF);
        ui_rect(x0 + LADO / 2 + dx - 2, y0 + LADO / 2 + dz - 2, 5, 5, 0xffe030FF);
    }
    float r = ang * 3.14159265f / 180.0f;                 // jugador: punto blanco con raya hacia donde mira
    ui_rect(x0 + LADO / 2 - 3, y0 + LADO / 2 - 3, 7, 7, 0x000000FF);
    ui_rect(x0 + LADO / 2 - 2, y0 + LADO / 2 - 2, 5, 5, 0xffffffFF);
    ui_rect(x0 + LADO / 2 + sinf(r) * 6 - 1, y0 + LADO / 2 + cosf(r) * 6 - 1, 3, 3, 0xffffffFF);
}
