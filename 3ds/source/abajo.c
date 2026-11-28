// Pantalla de abajo de la 3DS: todo a 1 píxel = 1 píxel, letra de 8x14 nítida.
//
//  +--------------------------------------+
//  | 09:30                          $ 120 |  barra de arriba
//  +----------------+---------------------+
//  |                | Misión / avisos     |
//  |   minimapa     |                     |
//  |   (152x152)    |                     |
//  +----------------+---------------------+
//  | aviso (B: Hablar...)     [ Guardar ] |
//  +--------------------------------------+
//  Los diálogos y ofertas aparecen encima de la parte de abajo.
#include "abajo.h"
#include "ui/ui.h"
#include "ui/minimapa.h"
#include "ui/dialogo.h"
#include "world/reloj.h"
#include "world/misiones.h"
#include <stdio.h>

#define FONDO   0x1b1f27FF
#define PANEL   0x2a303bFF
#define BLANCO  0xFFFFFFFF
#define GRIS    0x9aa3b2FF
#define AMARILLO 0xFFE860FF

// El minimapa original se dibuja arriba a la derecha de 960x544 (borde desde x=791, y=11).
#define MINI_X0 791
#define MINI_Y0 11
#define MINI_LADO 158
#define MINI_X 4
#define MINI_Y 26

static void texto_centro(float cx, float y, const char *t, unsigned col) {
    ui_texto(cx - ui_ancho(t, 1) * 0.5f, y, 1, t, col);
}

static void caja_texto(float x, float y, float w, const char *t, unsigned col, int max_lin) {
    char lin[6][96];
    if (max_lin > 6) max_lin = 6;
    int n = ui_partir(t, 1, w, lin, max_lin);
    for (int i = 0; i < n; i++) ui_texto(x, y + i * 15, 1, lin[i], col);
}

void abajo_dibujar(const EstadoAbajo *e) {
    char t[96];
    ui_rect(0, 0, ABAJO_W, ABAJO_H, FONDO);

    // --- Barra de arriba: hora y dinero ---
    ui_rect(0, 0, ABAJO_W, 22, 0x000000FF);
    float h = reloj_hora(); int hh = (int)h, mm = (int)((h - hh) * 60.0f);
    snprintf(t, sizeof(t), "%02d:%02d", hh, mm);
    ui_texto(6, 4, 1, t, BLANCO);
    snprintf(t, sizeof(t), "$ %d", mision_dinero());
    ui_texto(ABAJO_W - 6 - ui_ancho(t, 1), 4, 1, t, AMARILLO);
    if (e->lugar) texto_centro(ABAJO_W * 0.5f, 4, e->lugar, GRIS);

    // --- Minimapa (el mismo del juego, movido aquí) ---
    if (!e->lugar) {
        float mkx = 0, mkz = 0; int hm = mision_marca(&mkx, &mkz);
        ui_desplazar(MINI_X - MINI_X0, MINI_Y - MINI_Y0);
        minimapa_dibujar(e->px, e->pz, e->ang, hm, mkx, mkz);
        ui_desplazar(0, 0);
    } else {
        ui_rect(MINI_X, MINI_Y, MINI_LADO, MINI_LADO, PANEL);
        caja_texto(MINI_X + 8, MINI_Y + 10, MINI_LADO - 16, e->lugar, BLANCO, 3);
        caja_texto(MINI_X + 8, MINI_Y + 60, MINI_LADO - 16, "Camina hacia la puerta para salir.", GRIS, 4);
    }

    // --- Columna derecha: misión ---
    float cx = MINI_X + MINI_LADO + 6, cw = ABAJO_W - cx - 4;
    ui_rect(cx, MINI_Y, cw, MINI_LADO, PANEL);
    const char *tit = mision_titulo();
    if (tit[0]) {
        ui_texto(cx + 6, MINI_Y + 6, 1, "MISIÓN", AMARILLO);
        caja_texto(cx + 6, MINI_Y + 24, cw - 12, tit, BLANCO, 4);
        mision_estado(t, sizeof(t));
        caja_texto(cx + 6, MINI_Y + 90, cw - 12, t, BLANCO, 2);
    } else {
        ui_texto(cx + 6, MINI_Y + 6, 1, "Sin misión", GRIS);
        caja_texto(cx + 6, MINI_Y + 26, cw - 12, "Habla con la gente (B) para conseguir trabajos.", GRIS, 4);
    }
    int ok = 0; const char *m = mision_mensaje(&ok);
    if (m) {
        ui_rect(cx + 3, MINI_Y + MINI_LADO - 40, cw - 6, 36, ok ? 0x1d7a2eFF : 0x9a2020FF);
        caja_texto(cx + 8, MINI_Y + MINI_LADO - 37, cw - 16, m, BLANCO, 2);
    }

    // --- Parte de abajo: aviso y botón Guardar ---
    float y0 = MINI_Y + MINI_LADO + 6;
    if (e->guardado_msg) {
        ui_rect(4, y0, BOTON_X - 10, ABAJO_H - y0 - 4, e->guardado_ok ? 0x1d7a2eFF : 0x9a2020FF);
        caja_texto(10, y0 + 6, BOTON_X - 22, e->guardado_msg, BLANCO, 3);
    } else if (e->aviso) {
        ui_rect(4, y0, BOTON_X - 10, ABAJO_H - y0 - 4, PANEL);
        caja_texto(10, y0 + 6, BOTON_X - 22, e->aviso, BLANCO, 3);
    } else {
        caja_texto(10, y0 + 6, BOTON_X - 22, e->en_carro ? "R acelerar · L frenar · X bajar" : "A saltar · R correr · L cámara", GRIS, 3);
    }
    unsigned bc = e->boton_apretado ? 0x2f6fd0FF : 0x3d7be0FF;
    ui_rect(BOTON_X, BOTON_Y, BOTON_W, BOTON_H, 0x000000FF);
    ui_rect(BOTON_X + 2, BOTON_Y + 2, BOTON_W - 4, BOTON_H - 4, bc);
    texto_centro(BOTON_X + BOTON_W * 0.5f, BOTON_Y + 7, "Guardar", BLANCO);
    ui_texto(BOTON_X + 4, BOTON_Y - 15, 1, "SELECT o", GRIS);

    // --- Encima: oferta de misión o diálogo ---
    if (e->oferta) {
        ui_rect(4, 96, ABAJO_W - 8, ABAJO_H - 100, 0x000000F0);
        caja_texto(12, 104, ABAJO_W - 24, e->oferta, BLANCO, 6);
        ui_texto(12, ABAJO_H - 24, 1, "B: Aceptar     A: Rechazar", AMARILLO);
    }
    if (dialogo_activo()) dialogo_dibujar_en(6, 122, ABAJO_W - 12, 112, 1);
}
