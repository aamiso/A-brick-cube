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
    const float x = 40, w = 880, h = 150, y = 544 - h - 24;
    // nombre
    float nw = ui_ancho(g_nombre, ESC) + 36;
    ui_rect(x, y - 44, nw, 50, 0x202020ff); ui_rect(x + 4, y - 40, nw - 8, 46, 0xf8f8f8ff);
    ui_texto(x + 18, y - 44 + 8, ESC, g_nombre, 0x303030ff);
    // cuadro con borde doble (como Pokémon)
    ui_rect(x, y, w, h, 0x202020ff);
    ui_rect(x + 4, y + 4, w - 8, h - 8, 0xf8f8f8ff);
    ui_rect(x + 8, y + 8, w - 16, h - 16, 0x586878ff);
    ui_rect(x + 11, y + 11, w - 22, h - 22, 0xfcfcfcff);
    // texto: solo las letras ya escritas
    char lin[4][96];
    int nl = ui_partir(g_frases[g_i], ESC, w - 80, lin, 4);
    int quedan = (int)g_letras;
    for (int l = 0; l < nl && quedan > 0; l++) {
        char tmp[96]; int n = 0, c = 0;
        const char *s = lin[l];
        while (*s && c < quedan) { tmp[n++] = *s++; while (((unsigned char)*s & 0xC0) == 0x80) tmp[n++] = *s++; c++; }
        tmp[n] = 0;
        ui_texto(x + 40, y + 24 + l * 32, ESC, tmp, 0x303030ff);
        quedan -= c;
    }
    // flechita que parpadea cuando se terminó la frase
    if (g_letras >= g_total && (g_parpadeo / 20) % 2 == 0) {
        float ax = x + w - 56, ay = y + h - 44;
        ui_rect(ax, ay, 24, 6, 0xd02820ff); ui_rect(ax + 4, ay + 6, 16, 6, 0xd02820ff);
        ui_rect(ax + 8, ay + 12, 8, 6, 0xd02820ff);
    }
}
