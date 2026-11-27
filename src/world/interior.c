#include "world/interior.h"
#include "engine/voxel.h"
#include "engine/malla.h"
#include "player/personaje.h"
#include "ui/dialogo.h"
#include <vitaGL.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define LADO 0.25f           // cada celda del cuarto mide 1.0 = 4 cubitos
#define MAX_C 24
#define MAX_F 16
#define MAX_N 8
#define RAD 0.32f

typedef struct { const char *nombre, *oficio; const char *tanda[3][3]; } Gente;
typedef struct {
    char letra; const char *nombre;
    unsigned fa, fb, pared, borde, mesa, cuerpo, acento;
    int w, h; const char *filas[MAX_F];
    Gente gente[MAX_N];
} Cuarto;

// Letras: N = persona · # pared · C mostrador · B cama · T mesa · S banco · P planta · W estante · M máquina · F nevera · R alfombra
static const Cuarto CUARTOS[] = {
 { 'Y', "Hospital", 0xe8f1f2, 0xcfe3e6, 0xf2f2f2, 0x2aa7b8, 0xffffff, 0xd8e0e4, 0x3a8fd0, 16, 10,
   { "BB.BB1.MM..BB.BB", "................", "......N.........", "................", "PP............PP",
     "................", ".......N........", "...CCCCCCCC.....", "...N............", "SS..S.........PP" },
   { { "Dra. Elena", "Doctora", { { "Bienvenido al hospital. ¿Te sientes bien?", "Descansa y toma agua.", 0 },
                                  { "Los cubitos también se enferman, ¿sabías?", "Es broma. Cuídate.", 0 },
                                  { "Si te lastimas saltando, aquí te curamos.", 0, 0 } } },
     { "Enf. Marta", "Enfermera", { { "Recepción del hospital. Todo en orden.", "Pronto habrá más camas.", 0 },
                                    { "El doctor está atendiendo. Espera un momento.", 0, 0 },
                                    { "Aquí tienes una sonrisa gratis.", 0, 0 } } },
     { "Don Pedro", "Paciente", { { "Me torcí el tobillo saltando de un carro.", "No lo recomiendo.", 0 },
                                  { "Llevo horas esperando...", "Pero la sala es bonita.", 0 },
                                  { "Los médicos de aquí son muy buenos.", 0, 0 } } } } },
 { 'Q', "Café", 0xc99a62, 0xb88650, 0x8c3b2b, 0x4a2a1f, 0x6b4426, 0xa9783f, 0xf2c230, 15, 9,
   { "WWW.MM..1..PP.W", "......N........", "FCCCCCCC.......", "..............P", "..T..S...T..S..",
     "...............", "S..T..S...N....", "...............", "..S.....T.S...." },
   { { "Barista Lucía", "Cafetera", { { "¡Bienvenido! Hoy el café está recién hecho.", "Cuesta cero cubitos, ¡es una demo!", 0 },
                                      { "Mi especial es el capuchino de cubos.", 0, 0 },
                                      { "Pronto podrás pedir de verdad.", "Estamos trabajando en eso.", 0 } } },
     { "Tomás", "Cliente", { { "Este es mi café favorito de toda la ciudad.", 0, 0 },
                             { "Vengo todos los días a leer un rato.", 0, 0 },
                             { "Shh... estoy pensando en mi próximo viaje.", 0, 0 } } } } },
 { 'M', "Farmacia", 0xe6f3e6, 0xd2e8d2, 0xf4f4f4, 0x3aa655, 0xffffff, 0xe0ece0, 0x3aa655, 12, 8,
   { "WWWW1WWWW.WW", "............", ".....N......", "............", "..CCCCCC....",
     "............", "P..........P", "S...S......." },
   { { "Dr. Raúl", "Farmacéutico", { { "Bienvenido a la farmacia.", "Aquí hay de todo, menos medicinas de verdad.", 0 },
                                    { "Recuerda: mucha agua y buen descanso.", 0, 0 },
                                    { "¿Buscas algo? Seguro no lo tengo.", "Es broma, ¡estoy aprendiendo!", 0 } } } } },
 { 'X', "Banco", 0xd9d4c4, 0xc9c3b0, 0x6a7a8a, 0x2a3440, 0xf2c230, 0x8a8f96, 0xf2c230, 14, 8,
   { "WW1.MM.2...WW.P", "..............", "....N...N.....", "..CCCCCCCCC...", "..............",
     "S..........S..", "P............P", "...N.........." },
   { { "Cajera Sofía", "Cajera", { { "Bienvenido al banco.", "Aquí todo es de mentiras, ¡pero se ve serio!", 0 },
                                  { "El dinero no da la felicidad.", "Pero ayuda a comprar café.", 0 },
                                  { "¿Quieres abrir una cuenta de cubitos?", 0, 0 } } },
     { "Sr. Andrés", "Gerente", { { "Tenemos una reunión muy importante.", 0, 0 },
                                  { "Todo está bajo control.", 0, 0 },
                                  { "No toques la caja fuerte.", 0, 0 } } },
     { "Doña Rosa", "Clienta", { { "Vine a cobrar mis cubitos.", 0, 0 },
                                 { "¡Qué fila tan corta hoy!", 0, 0 },
                                 { "Ay, los bancos de ahora...", 0, 0 } } } } },
 { 'q', "Comisaría", 0xd8dde6, 0xc3cbd8, 0xdfe5ee, 0x1f3f8a, 0xb8bec6, 0x3a5fb0, 0x2f62c9, 14, 8,
   { "WW.MM....WW.PP", "..............", "...N......N...", "..CCCCCCC.....", "..............",
     "BB.........BB.", "..............", "P..S......S..P" },
   { { "Oficial Ramírez", "Policía", { { "Bienvenido a la comisaría.", "Aquí todo está en orden, ciudadano.", 0 },
                                       { "Recuerda: no se maneja sobre las aceras.", 0, 0 },
                                       { "¿Perdiste algo? Lo buscamos.", "Seguro está en el hospital.", 0 } } },
     { "Detective Luna", "Detective", { { "Estoy resolviendo el caso del café frío.", 0, 0 },
                                        { "Todo cubo tiene su secreto.", 0, 0 },
                                        { "Shh, estoy concentrada.", 0, 0 } } } } },
 { 'c', "Comisaría", 0xcfd6e0, 0xbcc6d4, 0xdfe6f0, 0x2a4fb0, 0xffffff, 0x4a5a78, 0x2a4fb0, 16, 9,
   { "WW1MM..WW2.WW.FF", "................", "...N.......N....", "..CCCCC.........", "................",
     "S....T.......PP.", "S............N..", "................", "P..............." },
   { { "Oficial Ramírez", "Policía", { { "Bienvenido a la comisaría.", "Aquí todo está bajo control.", 0 },
                                      { "Si ves algo raro en la ciudad, avísame.", 0, 0 },
                                      { "Y recuerda: no se maneja sobre las aceras.", 0, 0 } } },
     { "Sargento Iván", "Sargento", { { "Esta ciudad es segura gracias a nosotros.", 0, 0 },
                                      { "Los carros de patrulla no se prestan.", "Ni siquiera a ti.", 0 },
                                      { "Hoy hubo muy poco trabajo, qué bien.", 0, 0 } } },
     { "Don Hugo", "Ciudadano", { { "Vine a poner una denuncia.", "Alguien se llevó mi bici de cubitos.", 0 },
                                  { "La espera es larga pero hay café.", 0, 0 },
                                  { "Buenas, buenas.", 0, 0 } } } } },
};
#define N_CUARTOS ((int)(sizeof(CUARTOS) / sizeof(CUARTOS[0])))
#include "world/interior_datos.h"
#define N_EXTRA ((int)(sizeof(EXTRA) / sizeof(EXTRA[0])))
#define N_ENLACES ((int)(sizeof(ENLACES) / sizeof(ENLACES[0])))

typedef struct { float x, z; int g; int visitas; } Npc;
static const Cuarto *cuarto = 0;
static Malla malla;
static int hay_malla = 0;
static unsigned char sol[MAX_F + 2][MAX_C + 2];
static int W, H;                 // tamaño con paredes
static Npc gente[MAX_N];
static int n_gente = 0;
#define N_VAR 6
static Personaje var[N_VAR];
static int var_listas = 0;
static const unsigned CAM[N_VAR] = { 0xffffff, 0xe8e8f0, 0x3aa655, 0x8a3fc0, 0xe8742a, 0x455a64 };
static const unsigned PAN[N_VAR] = { 0xe0e8ec, 0x2f62c9, 0x30343a, 0x5a3a22, 0x30343a, 0x30343a };
static const unsigned PIE[N_VAR] = { 0xf2c230, 0xe0a85c, 0xc98a4a, 0xf2c230, 0x9a6a3c, 0xe8b878 };
static const unsigned PEL[N_VAR] = { 0x3a2414, 0x15120f, 0x8a5a22, 0xe0e0e0, 0x5a3a22, 0x6b3a1a };

// Cuarto número "i" del lugar "l" (el 0 es la entrada desde la calle)
static const Cuarto *buscar(char l, int i) {
    for (int k = 0; k < N_CUARTOS; k++) if (CUARTOS[k].letra == l && i-- == 0) return &CUARTOS[k];
    for (int k = 0; k < N_EXTRA; k++) if (EXTRA[k].letra == l && i-- == 0) return &EXTRA[k];
    return 0;
}
static char lugar_l = 0;
static int sala = 0, en_puerta = 0, pend_sala = 0;
static char pend_dig = 0;
int interior_hay(char l) { return buscar(l, 0) != 0; }
static char nombre_lugar[48];
const char *interior_nombre(char l) {                      // "Casa roja: sala" -> "Casa roja"
    const Cuarto *c = buscar(l, 0); if (!c) return "";
    snprintf(nombre_lugar, sizeof(nombre_lugar), "%s", c->nombre);
    char *d = strchr(nombre_lugar, ':'); if (d) *d = 0;
    return nombre_lugar;
}
const char *interior_titulo(void) { return cuarto ? cuarto->nombre : ""; }
static int es_puerta(char ch) { return ch >= '1' && ch <= '4'; }

static char celda(const Cuarto *c, int cx, int cz) {       // cx, cz dentro del cuarto (0..w-1, 0..h-1)
    if (cx < 0 || cz < 0 || cx >= c->w || cz >= c->h) return '.';
    const char *fila = c->filas[cz];
    return (int)strlen(fila) > cx ? fila[cx] : '.';
}

static void armar(const Cuarto *c) {
    W = c->w + 2; H = c->h + 2;
    Voxeles v;
    voxel_crear(&v, W * 4, H * 4, 14);
    voxel_color(&v, 'a', c->fa); voxel_color(&v, 'b', c->fb);
    voxel_color(&v, 'w', c->pared); voxel_color(&v, 't', c->borde);
    voxel_color(&v, 'x', c->mesa); voxel_color(&v, 'y', c->cuerpo); voxel_color(&v, 'z', c->acento);
    voxel_color(&v, 'k', 0x30343a); voxel_color(&v, 'g', 0x3a8f3a); voxel_color(&v, 'o', 0xc86a2a);
    voxel_color(&v, 'n', 0x9a6a3a); voxel_color(&v, 'm', 0x9aa0a8); voxel_color(&v, 'e', 0x9fdcf0);
    voxel_color(&v, 'W', 0xf4f4f4); voxel_color(&v, 'r', 0xd9382c); voxel_color(&v, 'D', 0x6b4a2a);
    memset(sol, 0, sizeof(sol));
    int ex = W / 2;                                           // la salida: 2 celdas al medio del sur
    for (int r = 0; r < H; r++) {
        for (int q = 0; q < W; q++) {
            int x = q * 4, z = r * 4;
            char ch = '.';
            int borde = (q == 0 || r == 0 || q == W - 1 || r == H - 1);
            int salida = (sala == 0 && r == H - 1 && (q == ex - 1 || q == ex));
            if (!borde) ch = celda(c, q - 1, r - 1);
            voxel_caja(&v, x, 0, z, 4, 1, 4, ((q + r) & 1) ? 'a' : 'b');     // piso
            if (salida) { voxel_caja(&v, x, 0, z, 4, 1, 4, 'D'); continue; }
            if (borde) {
                int alto = (r == H - 1) ? 2 : 12;                              // el frente (sur) es bajito
                voxel_caja(&v, x, 1, z, 4, alto - 1, 4, 'w');
                voxel_caja(&v, x, 1, z, 4, 1, 4, 't');
                if (alto > 2) {
                    voxel_caja(&v, x, alto, z, 4, 1, 4, 't');
                    if (r == 0 && q > 1 && q < W - 2 && q % 4 == 2) voxel_caja(&v, x + 1, 5, z + 3, 2, 4, 1, 'e');   // ventana
                }
                {   // ¿hay una puerta (número) justo al lado, dentro del cuarto? entonces la pared se vuelve puerta de madera
                    int nq = q, nr = r;
                    if (r == 0) nr = 1; else if (r == H - 1) nr = H - 2; else if (q == 0) nq = 1; else if (q == W - 1) nq = W - 2;
                    if ((nq != q || nr != r) && nq > 0 && nr > 0 && nq < W - 1 && nr < H - 1 && es_puerta(celda(c, nq - 1, nr - 1)))
                        voxel_caja(&v, x, 1, z, 4, alto - 1 > 8 ? 8 : alto - 1, 4, 'D');
                }
                sol[r][q] = 1;
                continue;
            }
            if (es_puerta(ch)) { voxel_caja(&v, x, 0, z, 4, 1, 4, 'D'); voxel_caja(&v, x + 1, 0, z + 1, 2, 1, 2, 'o'); continue; }
            switch (ch) {
            case 'C': voxel_caja(&v, x, 1, z, 4, 4, 4, 'y'); voxel_caja(&v, x, 5, z, 4, 1, 4, 'x'); sol[r][q] = 1; break;
            case 'B': voxel_caja(&v, x, 1, z, 4, 2, 4, 'z'); voxel_caja(&v, x, 3, z, 4, 1, 4, 'W'); voxel_caja(&v, x + 1, 4, z, 2, 1, 1, 'W');
                      voxel_caja(&v, x, 4, z + 1, 4, 1, 3, 'W'); sol[r][q] = 1; break;
            case 'T': voxel_caja(&v, x + 1, 1, z + 1, 2, 2, 2, 'n'); voxel_caja(&v, x, 3, z, 4, 1, 4, 'n'); sol[r][q] = 1; break;
            case 'S': voxel_caja(&v, x + 1, 1, z + 1, 2, 2, 2, 'z'); voxel_caja(&v, x + 1, 3, z + 1, 2, 1, 2, 'x'); sol[r][q] = 1; break;
            case 'P': voxel_caja(&v, x + 1, 1, z + 1, 2, 2, 2, 'o'); voxel_caja(&v, x, 3, z, 4, 3, 4, 'g'); voxel_caja(&v, x + 1, 6, z + 1, 2, 1, 2, 'g'); sol[r][q] = 1; break;
            case 'W': voxel_caja(&v, x, 1, z, 4, 8, 4, 'n'); voxel_caja(&v, x, 3, z + 3, 4, 1, 1, 'k'); voxel_caja(&v, x, 6, z + 3, 4, 1, 1, 'k'); sol[r][q] = 1; break;
            case 'M': voxel_caja(&v, x, 1, z, 4, 5, 4, 'm'); voxel_caja(&v, x + 1, 3, z + 3, 2, 2, 1, 'e'); sol[r][q] = 1; break;
            case 'F': voxel_caja(&v, x, 1, z, 4, 8, 4, 'W'); voxel_caja(&v, x, 5, z + 3, 4, 1, 1, 'm'); sol[r][q] = 1; break;
            case 'R': voxel_caja(&v, x, 0, z, 4, 1, 4, 'r'); break;
            default: break;
            }
        }
    }
    malla_iniciar(&malla);
    voxel_malla(&v, LADO, W * 2.0f, 0.0f, H * 2.0f, &malla);
    malla_subir(&malla);
    hay_malla = 1;
    voxel_liberar(&v);
}

static float cx_de(int q) { return (q + 0.5f) - W * 0.5f; }
static float cz_de(int r) { return (r + 0.5f) - H * 0.5f; }

static void cargar(char letra, int idx, char dig, Jugador *j) {
    const Cuarto *c = buscar(letra, idx);
    if (!c) return;
    lugar_l = letra; sala = idx; en_puerta = 0;
    if (!var_listas) { for (int i = 0; i < N_VAR; i++) personaje_crear(&var[i], CAM[i], PAN[i], PIE[i], PEL[i]); var_listas = 1; }
    if (hay_malla) { malla_liberar(&malla); hay_malla = 0; }
    cuarto = c;
    armar(c);
    n_gente = 0;
    for (int r = 0; r < c->h; r++)
        for (int q = 0; q < c->w; q++)
            if (celda(c, q, r) == 'N' && n_gente < MAX_N && c->gente[n_gente].nombre) {
                gente[n_gente].x = cx_de(q + 1); gente[n_gente].z = cz_de(r + 1);
                gente[n_gente].g = n_gente; gente[n_gente].visitas = 0;
                sol[r + 1][q + 1] = 1;
                n_gente++;
            }
    j->x = cx_de(W / 2) - 0.5f + 0.5f; j->z = cz_de(H - 2);
    j->angulo = 180.0f;
    if (dig) {                                               // llega por una puerta: aparece justo adentro de ella
        for (int r = 0; r < c->h; r++) for (int q = 0; q < c->w; q++) if (celda(c, q, r) == dig) {
            int dq = q == 0 ? 1 : q == c->w - 1 ? -1 : 0, dr = r == 0 ? 1 : r == c->h - 1 ? -1 : 0;
            j->x = cx_de(q + 1 + dq); j->z = cz_de(r + 1 + dr);
            j->angulo = dr > 0 ? 0.0f : dr < 0 ? 180.0f : dq > 0 ? 90.0f : 270.0f;
        }
    }
    j->vel = 0; j->elev = j->suelo = 0.25f; j->vy = 0;
}

void interior_entrar(char letra, Jugador *j) { cargar(letra, 0, 0, j); }
void interior_cambiar(Jugador *j) { cargar(lugar_l, pend_sala, pend_dig, j); }

void interior_salir(void) {
    if (hay_malla) { malla_liberar(&malla); hay_malla = 0; }
    cuarto = 0;
}

static int choca(float x, float z) {
    int q0 = (int)floorf(x + W * 0.5f - RAD), q1 = (int)floorf(x + W * 0.5f + RAD);
    int r0 = (int)floorf(z + H * 0.5f - RAD), r1 = (int)floorf(z + H * 0.5f + RAD);
    for (int r = r0; r <= r1; r++)
        for (int q = q0; q <= q1; q++) {
            if (q < 0 || r < 0 || q >= W || r >= H) return 1;
            if (!sol[r][q]) continue;
            float minx = q - W * 0.5f, minz = r - H * 0.5f;
            float nx = fmaxf(minx, fminf(x, minx + 1.0f)), nz = fmaxf(minz, fminf(z, minz + 1.0f));
            float dx = x - nx, dz = z - nz;
            if (dx * dx + dz * dz < RAD * RAD) return 1;
        }
    return 0;
}

int interior_actualizar(Jugador *j, float lx, float ly, int correr) {
    j->tiempo += 1.0f / 60.0f;
    float m = sqrtf(lx * lx + ly * ly);
    if (m > 0.05f) {
        float v = (correr ? 0.10f : 0.062f) * (m > 1.0f ? 1.0f : m);
        float dx = lx / m * v, dz = ly / m * v;
        if (!choca(j->x + dx, j->z)) j->x += dx;
        if (!choca(j->x, j->z + dz)) j->z += dz;
        j->angulo = atan2f(lx, ly) * 57.29578f;
        j->fase += correr ? 0.28f : 0.18f;
        j->vel += ((correr ? 1.7f : 1.0f) - j->vel) * 0.25f;
    } else j->vel += (0.0f - j->vel) * 0.25f;
    j->elev = j->suelo = 0.25f;
    {   // ¿pisó una puerta (1-4)? lleva a otro cuarto del mismo lugar
        int q = (int)floorf(j->x + W * 0.5f), r = (int)floorf(j->z + H * 0.5f);
        char ch = (q >= 1 && r >= 1 && q <= W - 2 && r <= H - 2) ? celda(cuarto, q - 1, r - 1) : 0;
        if (!es_puerta(ch)) en_puerta = 0;
        else if (!en_puerta)
            for (int k = 0; k < N_ENLACES; k++)
                if (ENLACES[k].lugar == lugar_l && ENLACES[k].room == sala && ENLACES[k].dig == ch) {
                    pend_sala = ENLACES[k].to_room; pend_dig = ENLACES[k].to_dig; en_puerta = 1; return 2;
                }
    }
    return sala == 0 && j->z > cz_de(H - 1) - 0.4f && fabsf(j->x) < 1.2f;      // pisó la alfombra de salida
}

void interior_dibujar(float tiempo) {
    if (!hay_malla) return;
    glPushMatrix(); malla_dibujar(&malla); glPopMatrix();
    for (int i = 0; i < n_gente; i++) {
        glPushMatrix();
        glTranslatef(0.0f, 0.25f, 0.0f);
        personaje_dibujar(&var[(i + (cuarto->letra & 3)) % N_VAR], gente[i].x, gente[i].z, 0.0f, 0.0f, 0.0f, tiempo + i * 1.7f, 0.0f);
        glPopMatrix();
    }
}

int interior_npc_cercano(const Jugador *j, float alcance) {
    int mejor = -1; float md = alcance * alcance;
    for (int i = 0; i < n_gente; i++) {
        float dx = gente[i].x - j->x, dz = gente[i].z - j->z, d = dx * dx + dz * dz;
        if (d < md) { md = d; mejor = i; }
    }
    return mejor;
}
const char *interior_npc_nombre(int n) { return cuarto->gente[gente[n].g].nombre; }

static char nombre_buf[64];
void interior_hablar(int n, const Jugador *j) {
    (void)j;
    const Gente *g = &cuarto->gente[gente[n].g];
    const char *const *t = g->tanda[gente[n].visitas++ % 3];
    int k = 0; while (k < 3 && t[k]) k++;
    snprintf(nombre_buf, sizeof(nombre_buf), "%s · %s", g->nombre, g->oficio);
    dialogo_abrir(nombre_buf, t, k);
}
