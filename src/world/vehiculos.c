#include "world/vehiculos.h"
#include "world/ciudad.h"
#include "engine/voxel.h"
#include "engine/malla.h"
#include "core/util.h"
#include <vitaGL.h>
#include <math.h>

#define MAX_CARROS 10
#define LADO 0.2f
#define VEL_MAX 0.36f

typedef struct { float x, z, ang, vel; Malla malla; } Carro;
static Carro carros[MAX_CARROS];
static int n_carros = 0;

static const unsigned int COLORES[] = { 0xd9382c, 0x2f62c9, 0xf2c230, 0x3aa655, 0xf2f2f2, 0x8a3fc0, 0xe8742a, 0x303338 };

// Un carro de 8 x 16 x 7 cubitos (1.6 de ancho, 3.2 de largo). El frente mira a +z.
static void armar_carro(Malla *m, unsigned int color, int tipo) {
    Voxeles v;
    voxel_crear(&v, 8, 16, 7);
    voxel_color(&v, 'c', color);
    voxel_color(&v, 'g', 0x9fd8f0);
    voxel_color(&v, 'k', 0x1c1c20);
    voxel_color(&v, 'l', 0xfff2a0);
    voxel_color(&v, 'r', 0xe02020);
    voxel_color(&v, 'm', 0x9aa0a8);
    voxel_color(&v, 'd', ((color >> 1) & 0x7f7f7f));
    voxel_color(&v, 'w', 0xf2f2f2); voxel_color(&v, 'a', 0x2a64e0);
    voxel_caja(&v, 0, 1, 0, 8, 3, 16, 'c');          // carrocería
    if (tipo == 1) {                                  // camioneta: cabina adelante, caja atrás
        voxel_caja(&v, 1, 4, 8, 6, 2, 5, 'g'); voxel_caja(&v, 1, 6, 8, 6, 1, 5, 'd');
        voxel_caja(&v, 1, 4, 8, 6, 2, 1, 'd'); voxel_caja(&v, 1, 4, 12, 6, 2, 1, 'd');
        voxel_caja(&v, 1, 4, 0, 6, 1, 7, 'd'); voxel_caja(&v, 1, 4, 1, 6, 1, 5, '.');
        voxel_caja(&v, 0, 4, 0, 1, 1, 7, 'c'); voxel_caja(&v, 7, 4, 0, 1, 1, 7, 'c'); voxel_caja(&v, 1, 4, 0, 6, 1, 1, 'c');
    } else {
        voxel_caja(&v, 1, 4, 3, 6, 2, 8, 'g');       // cabina (vidrios)
        voxel_caja(&v, 1, 6, 3, 6, 1, 8, (tipo == 2) ? 'a' : 'd');   // techo
        voxel_caja(&v, 1, 4, 3, 6, 2, 1, 'd');       // pilares
        voxel_caja(&v, 1, 4, 10, 6, 2, 1, 'd');
        if (tipo == 2) {                              // patrulla: franja blanca y luces
            voxel_caja(&v, 0, 2, 4, 8, 1, 7, 'w');
            voxel_caja(&v, 2, 7, 6, 2, 1, 2, 'r'); voxel_caja(&v, 4, 7, 6, 2, 1, 2, 'a');
        }
    }
    voxel_caja(&v, 0, 0, 0, 8, 1, 16, 'm');          // parachoques / bajos
    voxel_caja(&v, 0, 0, 2, 1, 2, 3, 'k'); voxel_caja(&v, 7, 0, 2, 1, 2, 3, 'k');     // ruedas
    voxel_caja(&v, 0, 0, 11, 1, 2, 3, 'k'); voxel_caja(&v, 7, 0, 11, 1, 2, 3, 'k');
    voxel_caja(&v, 1, 2, 15, 2, 1, 1, 'l'); voxel_caja(&v, 5, 2, 15, 2, 1, 1, 'l'); // luces
    voxel_caja(&v, 1, 2, 0, 2, 1, 1, 'r');  voxel_caja(&v, 5, 2, 0, 2, 1, 1, 'r');
    voxel_malla(&v, LADO, 4.0f, 0.0f, 8.0f, m);
    malla_subir(m);
    voxel_liberar(&v);
}

void vehiculos_iniciar(float cx, float cz) {
    for (int i = 0; i < n_carros; i++) malla_liberar(&carros[i].malla);
    n_carros = 0;
    // Busca tramos de calle (3 cuadros de calle seguidos) cerca del inicio
    for (int radio = 4; radio < 60 && n_carros < MAX_CARROS; radio += 1) {
        for (int f = 1; f < ciudad_alto() - 1 && n_carros < MAX_CARROS; f++) {
            for (int c = 1; c < ciudad_ancho() - 1 && n_carros < MAX_CARROS; c++) {
                float x = ciudad_centro_x(c), z = ciudad_centro_z(f);
                float d = fmaxf(fabsf(x - cx), fabsf(z - cz));
                if (d < radio || d >= radio + 1) continue;
                if (ciudad_tipo_en(c, f) != '=') continue;
                int hor = ciudad_tipo_en(c - 1, f) == '=' && ciudad_tipo_en(c + 1, f) == '=';
                int ver = ciudad_tipo_en(c, f - 1) == '=' && ciudad_tipo_en(c, f + 1) == '=';
                if (hor == ver) continue;   // cruce o borde: no
                if (ciudad_choca(x, z, 1.0f)) continue;
                int cerca = 0;
                for (int i = 0; i < n_carros; i++)
                    if (fabsf(carros[i].x - x) < 14.0f && fabsf(carros[i].z - z) < 14.0f) cerca = 1;
                if (cerca) continue;
                Carro *k = &carros[n_carros];
                k->x = x; k->z = z; k->vel = 0.0f;
                k->ang = hor ? 90.0f : 0.0f;
                malla_iniciar(&k->malla);
                { int tp = n_carros % 3; armar_carro(&k->malla, tp == 2 ? 0x1f3f8a : COLORES[n_carros % 8], tp); }
                n_carros++;
            }
        }
    }
}

void vehiculos_dibujar(float px, float pz) {
    for (int i = 0; i < n_carros; i++) {
        const Carro *k = &carros[i];
        if (fabsf(k->x - px) > 90.0f || fabsf(k->z - pz) > 90.0f) continue;
        glPushMatrix();
        glTranslatef(k->x, 0.0f, k->z);
        glRotatef(k->ang, 0.0f, 1.0f, 0.0f);
        malla_dibujar(&k->malla);
        glPopMatrix();
    }
}

int vehiculo_cercano(float x, float z, float alcance) {
    int mejor = -1; float md = alcance * alcance;
    for (int i = 0; i < n_carros; i++) {
        float dx = carros[i].x - x, dz = carros[i].z - z, d = dx * dx + dz * dz;
        if (d < md) { md = d; mejor = i; }
    }
    return mejor;
}

void vehiculo_pos(int n, float *x, float *z, float *angulo, float *vel) {
    *x = carros[n].x; *z = carros[n].z; *angulo = carros[n].ang; *vel = carros[n].vel;
}

// ¿Toca un círculo en (x, z) algún carro (menos el "ignorar")?  Cada carro = 3 círculos.
int vehiculos_choca(float x, float z, float radio, int ignorar) {
    for (int n = 0; n < n_carros; n++) {
        if (n == ignorar) continue;
        float dx = carros[n].x - x, dz = carros[n].z - z;
        if (dx * dx + dz * dz > 6.0f) continue;
        float r = carros[n].ang * PI / 180.0f, sx = sinf(r), sz = cosf(r);
        for (int i = -1; i <= 1; i++) {
            float ex = carros[n].x + sx * i * 1.0f - x, ez = carros[n].z + sz * i * 1.0f - z, rr = 0.75f + radio;
            if (ex * ex + ez * ez < rr * rr) return 1;
        }
    }
    return 0;
}

// ¿Choca el carro n estando en (x, z) mirando a ang? (3 círculos + otros carros)
static int choca(int n, float x, float z, float ang) {
    float r = ang * PI / 180.0f, sx = sinf(r), sz = cosf(r);
    for (int i = -1; i <= 1; i++)
        if (ciudad_choca(x + sx * i * 1.0f, z + sz * i * 1.0f, 0.75f) ||
            vehiculos_choca(x + sx * i * 1.0f, z + sz * i * 1.0f, 0.75f, n)) return 1;
    return 0;
}

void vehiculo_manejar(int n, float lx, float ly, int freno) {
    Carro *k = &carros[n];
    float acel = -ly;                         // arriba del stick = adelante
    if (acel > 0.0f)      k->vel += acel * 0.014f;
    else if (acel < 0.0f) k->vel += acel * (k->vel > 0.02f ? 0.022f : 0.007f);   // frena y luego reversa
    else                  k->vel *= 0.985f;
    if (freno) k->vel *= 0.92f;
    k->vel *= 0.997f;
    if (k->vel > VEL_MAX) k->vel = VEL_MAX;
    if (k->vel < -VEL_MAX * 0.4f) k->vel = -VEL_MAX * 0.4f;
    if (fabsf(k->vel) < 0.002f && acel == 0.0f) k->vel = 0.0f;

    // El volante gira más cuanto más rápido va (y no gira parado)
    float v = fabsf(k->vel) / VEL_MAX;
    // Volante suave: poco giro con el stick a medias, y menos giro a toda velocidad
    float fino = lx * fabsf(lx);
    float giro = -fino * 2.0f * sqrtf(v < 0.25f ? v * 2.0f : 0.5f + v * 0.5f) * (1.0f - 0.35f * v);
    if (k->vel < 0.0f) giro = -giro;
    float nuevo = k->ang + giro;
    if (!choca(n, k->x, k->z, nuevo)) k->ang = nuevo;

    float r = k->ang * PI / 180.0f;
    float px = k->x + sinf(r) * k->vel, pz = k->z + cosf(r) * k->vel;
    if (!choca(n, px, k->z, k->ang)) k->x = px; else k->vel *= -0.3f;
    if (!choca(n, k->x, pz, k->ang)) k->z = pz; else k->vel *= -0.3f;
}
