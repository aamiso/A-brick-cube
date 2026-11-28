#include "ui/dialogo.h"
#include "ui/ui.h"
#include <string.h>
#include <math.h>

#define ESC 2
static const char *g_frases[16];
static int g_n = 0, g_i = 0, g_activo = 0;
static char g_nombre[40];
static float g_letras = 0.0f;       // cuántas letras ya se escribieron
static int g_total = 0;             // letras de la frase actual
static int g_parpadeo = 0;

static int contar(const char *s) { int n = 0; while (*s) { if (((unsigned char)*s & 0xC0) != 0x80) n++; s++; } return n; }

void dialogo_abrir(const char *nombre, const char *const *frases, int n) {
    if (n > 16) n = 16;
    for (int i = 0; i < n; i++) g_frases[i] = frases[i];
    g_n = n; g_i = 0; g_activo = n > 0; g_letras = 0.0f; g_parpadeo = 0;
    strncpy(g_nombre, nombre, sizeof(g_nombre) - 1); g_nombre[sizeof(g_nombre) - 1] = 0;
    g_total = n > 0 ? contar(frases[0]) : 0;
}

int dialogo_activo(void) { return g_activo; }

int dialogo_actualizar(int avanzar) {
    if (!g_activo) return 0;
    g_parpadeo++;
    if (g_letras < g_total) g_letras += 0.9f;               // máquina de escribir
    if (!avanzar) return 0;
    if (g_letras < g_total) { g_letras = (float)g_total; return 0; }   // primero muestra toda la frase
    g_i++;
    if (g_i >= g_n) { g_activo = 0; return 1; }
    g_letras = 0.0f; g_total = contar(g_frases[g_i]);
    return 0;
}

void dialogo_dibujar(void) {
    if (!g_activo) return;
    const float w = 880, h = 150;
    dialogo_dibujar_en(40, 544 - h - 24, w, h, ESC);
}

void dialogo_dibujar_en(float x, float y, float w, float h, int esc) {
    if (!g_activo) return;
    const float k = (float)esc / ESC;          // bordes y márgenes proporcionales a la letra
    const float b1 = 4 * k < 1 ? 1 : 4 * k, b2 = 8 * k < 2 ? 2 : 8 * k, b3 = 11 * k < 3 ? 3 : 11 * k;
    // nombre
    float nw = ui_ancho(g_nombre, esc) + 36 * k, nh = 50 * k;
    ui_rect(x, y - nh + 6 * k, nw, nh, 0x202020ff); ui_rect(x + b1, y - nh + 6 * k + b1, nw - 2 * b1, nh - b1, 0xf8f8f8ff);
    ui_texto(x + 18 * k, y - nh + 14 * k, esc, g_nombre, 0x303030ff);
    // cuadro con borde doble (como Pokémon)
    ui_rect(x, y, w, h, 0x202020ff);
    ui_rect(x + b1, y + b1, w - 2 * b1, h - 2 * b1, 0xf8f8f8ff);
    ui_rect(x + b2, y + b2, w - 2 * b2, h - 2 * b2, 0x586878ff);
    ui_rect(x + b3, y + b3, w - 2 * b3, h - 2 * b3, 0xfcfcfcff);
    // texto: solo las letras ya escritas
    char lin[4][96];
    int nl = ui_partir(g_frases[g_i], esc, w - 80 * k, lin, 4);
    int quedan = (int)g_letras;
    for (int l = 0; l < nl && quedan > 0; l++) {
        char tmp[96]; int n = 0, c = 0;
        const char *s = lin[l];
        while (*s && c < quedan) { tmp[n++] = *s++; while (((unsigned char)*s & 0xC0) == 0x80) tmp[n++] = *s++; c++; }
        tmp[n] = 0;
        ui_texto(x + 40 * k, y + 24 * k + l * 32 * k, esc, tmp, 0x303030ff);
        quedan -= c;
    }
    // flechita que parpadea cuando se terminó la frase
    if (g_letras >= g_total && (g_parpadeo / 20) % 2 == 0) {
        float ax = x + w - 56 * k, ay = y + h - 44 * k;
        ui_rect(ax, ay, 24 * k, 6 * k, 0xd02820ff); ui_rect(ax + 4 * k, ay + 6 * k, 16 * k, 6 * k, 0xd02820ff);
        ui_rect(ax + 8 * k, ay + 12 * k, 8 * k, 6 * k, 0xd02820ff);
    }
}
