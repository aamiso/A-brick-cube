#include "world/npcs.h"
#include "world/ciudad.h"
#include "world/vehiculos.h"
#include "world/escombros.h"
#include "world/misiones.h"
#include "engine/cubo.h"
#include "player/personaje.h"
#include "ui/dialogo.h"
#include "core/util.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_NPC 40
#define RADIO_NPC 0.4f

typedef struct {
    float x, z, ang, vel, fase, tiempo;
    float casa_x, casa_z;       // alrededor de dónde pasea
    float dir;                  // hacia dónde camina
    int espera;                 // cuadros hasta cambiar de idea
    int caminando, hablando;
    int muerto;                 // cuadros que faltan para que reaparezca (0 = vivo)
    int rol, variante, visitas;
    const char *nombre;
} Npc;

static Npc npcs[MAX_NPC];
static int n_npc = 0;
#define N_VAR 8
static Personaje var[N_VAR];
static int var_listas = 0;

// ---- Quiénes son y qué dicen (cada rol tiene 3 conversaciones; van rotando) ----
typedef struct { const char *oficio; const char *tanda[3][4]; } Rol;
static const Rol ROLES[] = {
 { "Vecino", { { "¡Hola! Qué buen día para pasear por la ciudad.", "Dicen que hay un hospital enorme en el centro.", 0, 0 },
               { "¿Ya probaste los carros? Con R aceleras y con L frenas.", "Con el freno de mano (cuadrado) puedes derrapar.", 0, 0 },
               { "Yo vivo por aquí desde hace años.", "Esta ciudad no deja de crecer.", 0, 0 } } },
 { "Vecina", { { "¡Buenas! ¿Perdido? El centro queda hacia donde están los rascacielos.", 0, 0, 0 },
               { "Si saltas puedes subirte a los escalones y bordes bajos.", "¡Hasta puedes llegar a algunos porches!", 0, 0 },
               { "Me encantan los árboles de esta calle.", "Cada uno es distinto, ¿te fijaste?", 0, 0 } } },
 { "Policía", { { "Buenas. Circula con cuidado, ¿sí?", "Y recuerda: no se maneja sobre las aceras.", 0, 0 },
                { "Esta patrulla no la toques, ¡es oficial!", "Bueno... solo si nadie mira.", 0, 0 },
                { "Todo tranquilo en la ciudad hoy.", 0, 0, 0 } } },
 { "Doctora", { { "Vengo del hospital, el edificio blanco con la H en el techo.", "Si te sientes mal, pasa a verme.", 0, 0 },
                { "Dicen que correr es bueno para la salud.", "Mantén apretado R y verás.", 0, 0 },
                { "Hoy fue un turno largo...", "Necesito un café.", 0, 0 } } },
 { "Turista", { { "¡Wow! Qué ciudad más bonita, parece de cubitos.", "Voy a sacar muchas fotos.", 0, 0 },
                { "Busco las torres gemelas. ¿Sabes dónde están?", "Dicen que se ven desde muy lejos.", 0, 0 },
                { "Este lugar tiene hotel, café y hasta una torre dorada.", 0, 0, 0 } } },
 { "Niño", { { "¡Mira, puedo saltar muy alto!", "Pero tú saltas más, ¿verdad?", 0, 0 },
             { "¿Me llevas en el carro?", "...Qué aburrido, dice mamá que no.", 0, 0 },
             { "Estoy jugando a atrapar nubes.", 0, 0, 0 } } },
 { "Banquero", { { "Buenos días. El banco abre pronto.", "Aquí todo es de mentiras, ¡pero se ve serio!", 0, 0 },
                 { "Dicen que el dinero no da la felicidad.", "Pero ayuda a comprar un café.", 0, 0 },
                 { "Tengo una reunión en el centro.", 0, 0, 0 } } },
 { "Repartidor", { { "¡Voy tarde con este pedido!", "Mejor no hables mucho conmigo, ¿ok?", 0, 0 },
                   { "Entrego cosas en toda la ciudad.", "Las calles anchas ayudan un montón.", 0, 0 },
                   { "Uf, qué cansancio.", "¿No tienes un carro para prestarme?", 0, 0 } } },
 { "Anciano", { { "Cuando yo era joven esto eran puros campos.", "Ahora hay edificios por todas partes.", 0, 0 },
                { "Camina despacio y mira todo, hijo.", "La ciudad se disfruta mejor así.", 0, 0 },
                { "Antes los carros eran más lentos.", "Y también más bonitos.", 0, 0 } } },
 { "Cafetero", { { "Buenas, soy del café de la esquina.", "Preparo el mejor café de cubitos.", 0, 0 },
                 { "¿Un cafecito? Pronto podrás entrar a los locales.", "Estamos trabajando en eso.", 0, 0 },
                 { "Aquí el café siempre está caliente.", 0, 0, 0 } } },
};
#define N_ROLES ((int)(sizeof(ROLES) / sizeof(ROLES[0])))
static const char *NOMBRES[] = { "Marta", "Luis", "Sofía", "Pedro", "Camila", "Diego", "Lucía", "Mateo", "Valeria", "Andrés", "Elena", "Tomás",
                                 "Paula", "Hugo", "Rosa", "Bruno", "Inés", "Óscar", "Nora", "Iván", "Julia", "Raúl", "Ana", "Sergio" };
#define N_NOMBRES ((int)(sizeof(NOMBRES) / sizeof(NOMBRES[0])))

static const unsigned CAMISAS[N_VAR]   = { 0x3aa655, 0x8a3fc0, 0xe8742a, 0x19a7b8, 0xf2f2f2, 0xc2185b, 0x455a64, 0xd9b23a };
static const unsigned PANTALON[N_VAR]  = { 0x30343a, 0x2f62c9, 0x5a3a22, 0x30343a, 0x6b4a8a, 0x30343a, 0x8a8a8f, 0x3b5c8a };
static const unsigned PIELES[N_VAR]    = { 0xf2c230, 0xe0a85c, 0xc98a4a, 0xf2c230, 0x9a6a3c, 0xf2c230, 0xd9a05a, 0xe8b878 };
static const unsigned PELOS[N_VAR]     = { 0x3a2414, 0x15120f, 0x8a5a22, 0xe0e0e0, 0x5a3a22, 0xc2a030, 0x15120f, 0x6b3a1a };

static unsigned azar_s = 12345u;
static unsigned azar(void) { azar_s = azar_s * 1664525u + 1013904223u; return azar_s >> 8; }

void npcs_iniciar(float cx, float cz) {
    if (!var_listas) {
        for (int i = 0; i < N_VAR; i++) personaje_crear(&var[i], CAMISAS[i], PANTALON[i], PIELES[i], PELOS[i]);
        var_listas = 1;
    }
    n_npc = 0; azar_s = 12345u;
    int intentos = 0;
    while (n_npc < MAX_NPC && intentos++ < 20000) {
        int c = (int)(azar() % (unsigned)ciudad_ancho()), f = (int)(azar() % (unsigned)ciudad_alto());
        float x = ciudad_centro_x(c), z = ciudad_centro_z(f);
        if (fabsf(x - cx) > 110.0f || fabsf(z - cz) > 110.0f) continue;
        char t = ciudad_tipo_en(c, f);
        if (t != 'S') continue;                                   // solo en las aceras
        if (fabsf(x - cx) < 3.0f && fabsf(z - cz) < 3.0f) continue;
        if (ciudad_choca(x, z, 0.6f)) continue;
        int cerca = 0;
        for (int i = 0; i < n_npc; i++) if (fabsf(npcs[i].x - x) < 7.0f && fabsf(npcs[i].z - z) < 7.0f) cerca = 1;
        if (cerca) continue;
        Npc *n = &npcs[n_npc];
        memset(n, 0, sizeof(*n));
        n->x = n->casa_x = x; n->z = n->casa_z = z;
        n->dir = (float)(azar() % 360); n->ang = n->dir;
        n->rol = n_npc % N_ROLES; n->variante = (int)(azar() % N_VAR);
        n->nombre = NOMBRES[n_npc % N_NOMBRES];
        n->espera = (int)(azar() % 120);
        n_npc++;
    }
}

int npcs_choca(float x, float z, float radio, int ignorar) {
    for (int i = 0; i < n_npc; i++) {
        if (i == ignorar || npcs[i].muerto) continue;
        float dx = npcs[i].x - x, dz = npcs[i].z - z, r = RADIO_NPC + radio;
        if (dx * dx + dz * dz < r * r) return 1;
    }
    return 0;
}

static int libre(int i, float x, float z) {
    return !ciudad_choca(x, z, RADIO_NPC) && !vehiculos_choca(x, z, RADIO_NPC, -1) && !npcs_choca(x, z, RADIO_NPC, i);
}

void npcs_actualizar(float px, float pz) {
    for (int i = 0; i < n_npc; i++) {
        Npc *n = &npcs[i];
        if (n->muerto) {                                              // destruido: reaparece en su casa
            if (--n->muerto == 0) { n->x = n->casa_x; n->z = n->casa_z; n->vel = 0.0f; n->hablando = 0; n->espera = 30; }
            continue;
        }
        if (fabsf(n->x - px) > 70.0f || fabsf(n->z - pz) > 70.0f) continue;
        n->tiempo += 1.0f / 60.0f;
        { float cvx, cvz;
          if (!n->hablando && vehiculos_atropella(n->x, n->z, RADIO_NPC, &cvx, &cvz)) {   // ¡atropellado! se hace cubitos
            int v = n->variante;
            escombros_crear(n->x, 0.9f, n->z, cvx * 1.4f, cvz * 1.4f, CAMISAS[v], 9, 0.22f, 0.10f);
            escombros_crear(n->x, 0.5f, n->z, cvx * 1.4f, cvz * 1.4f, PANTALON[v], 8, 0.22f, 0.10f);
            escombros_crear(n->x, 1.4f, n->z, cvx * 1.4f, cvz * 1.4f, PIELES[v], 5, 0.2f, 0.12f);
            escombros_crear(n->x, 1.6f, n->z, cvx * 1.4f, cvz * 1.4f, PELOS[v], 4, 0.2f, 0.12f);
            n->muerto = 900; continue;
          } }
        if (vehiculos_choca(n->x, n->z, RADIO_NPC, -1)) {            // un carro lo alcanzó: sale por el lado libre
            for (int k = 0; k < 16; k++) {
                float a = k * 0.3927f;
                for (float d = 0.4f; d <= 2.4f; d += 0.4f) {
                    float sx = n->x + sinf(a) * d, sz = n->z + cosf(a) * d;
                    if (!vehiculos_choca(sx, sz, RADIO_NPC, -1) && !ciudad_choca(sx, sz, RADIO_NPC)) { n->x = sx; n->z = sz; k = 99; break; }
                }
            }
            n->espera = 20;
        }
        if (n->hablando) { n->vel += (0.0f - n->vel) * 0.2f; continue; }
        if (--n->espera <= 0) {
            n->caminando = !n->caminando || (azar() % 4 == 0);
            n->espera = 60 + (int)(azar() % 180);
            if (n->caminando) {
                float dx = n->casa_x - n->x, dz = n->casa_z - n->z;
                if (dx * dx + dz * dz > 100.0f) n->dir = atan2f(dx, dz) * 57.29578f + (float)((int)(azar() % 60) - 30);  // vuelve a casa
                else n->dir = (float)(azar() % 360);
            }
        }
        float objetivo = n->caminando ? 1.0f : 0.0f;
        n->vel += (objetivo - n->vel) * 0.1f;
        if (n->caminando) {
            float d = n->dir - n->ang;
            while (d > 180.0f) d -= 360.0f;
            while (d < -180.0f) d += 360.0f;
            n->ang += d * 0.12f;
            float r = n->ang * 3.14159265f / 180.0f;
            float nx = n->x + sinf(r) * 0.05f, nz = n->z + cosf(r) * 0.05f;
            if (libre(i, nx, nz)) { n->x = nx; n->z = nz; n->fase += 0.18f; }
            else { n->dir = n->ang + 90.0f + (float)(azar() % 120); n->espera = 20; }
        }
    }
}

void npcs_dibujar(float px, float pz) {
    for (int i = 0; i < n_npc; i++) {
        const Npc *n = &npcs[i];
        if (n->muerto) continue;
        if (fabsf(n->x - px) > 60.0f || fabsf(n->z - pz) > 60.0f) continue;
        personaje_dibujar(&var[n->variante], n->x, n->z, n->ang, n->vel, n->fase, n->tiempo, 0.0f);
        if (npc_da_mision(i) && !mision_activa() && !n->hablando) {          // cubito amarillo flotando: ofrece una misión
            cubo_activar();
            caja(n->x, 3.3f + sinf(n->tiempo * 3.0f) * 0.15f, n->z, 0.35f, 0.35f, 0.35f, 1.0f, 0.85f, 0.1f);
        }
    }
}

int npc_cercano(float x, float z, float alcance) {
    int mejor = -1; float md = alcance * alcance;
    for (int i = 0; i < n_npc; i++) {
        if (npcs[i].muerto) continue;
        float dx = npcs[i].x - x, dz = npcs[i].z - z, d = dx * dx + dz * dz;
        if (d < md) { md = d; mejor = i; }
    }
    return mejor;
}

const char *npc_nombre(int n) { return npcs[n].nombre; }

static char nombre_buf[64];
void npc_hablar(int n, float px, float pz) {
    Npc *p = &npcs[n];
    p->hablando = 1;
    p->ang = atan2f(px - p->x, pz - p->z) * 57.29578f;      // mira al jugador
    const Rol *r = &ROLES[p->rol];
    const char *const *tanda = r->tanda[p->visitas % 3];
    int cuantas = 0;
    while (cuantas < 4 && tanda[cuantas]) cuantas++;
    p->visitas++;
    snprintf(nombre_buf, sizeof(nombre_buf), "%s · %s", p->nombre, r->oficio);
    dialogo_abrir(nombre_buf, tanda, cuantas);
}

void npc_soltar(int n) { npcs[n].hablando = 0; npcs[n].espera = 30; }

int npc_da_mision(int n) { return n % 3 == 0; }
