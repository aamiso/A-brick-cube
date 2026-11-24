#include "world/vehiculos.h"
#include "world/ciudad.h"
#include "engine/voxel.h"
#include "engine/malla.h"
#include "core/util.h"
#include <vitaGL.h>
#include <math.h>

#define MAX_CARROS 12
#define LADO 0.1f
#define CW 18          // ancho en cubitos (1.8)
#define CD 40          // largo en cubitos (4.0)
#define CH 18          // alto en cubitos
#define VEL_MAX 0.55f  // unidades por cuadro (~33 u/s)
#define EMPATE 2.4f    // distancia entre ejes

typedef struct { float x, z, ang, vx, vz, vel, volante; int tipo; Malla malla; } Carro;
static Carro carros[MAX_CARROS];
static int n_carros = 0;

static const unsigned int COLORES[] = { 0xd9382c, 0x2f62c9, 0x3aa655, 0xf2f2f2, 0x8a3fc0, 0xe8742a, 0x303338, 0x19a7b8 };

// Carros de cubitos de 0.1: frente hacia +z.  tipos: 0 sedán, 1 camioneta, 2 patrulla, 3 deportivo, 4 taxi
static void armar_carro(Malla *m, unsigned int color, int tipo) {
    Voxeles v;
    voxel_crear(&v, CW, CD, CH);
    unsigned int col = color;
    if (tipo == 2) col = 0x1f3f8a;
    if (tipo == 4) col = 0xf2c230;
    voxel_color(&v, 'c', col);
    voxel_color(&v, 'd', ((col >> 1) & 0x7f7f7f));
    voxel_color(&v, 'g', 0x8fcfe6);
    voxel_color(&v, 'G', 0x5aa6c4);
    voxel_color(&v, 'k', 0x1a1a1e);
    voxel_color(&v, 'h', 0xb8bec6);
    voxel_color(&v, 'l', 0xfff2a0);
    voxel_color(&v, 'r', 0xe02020);
    voxel_color(&v, 'm', 0x9aa0a8);
    voxel_color(&v, 'u', 0x2a2a30);
    voxel_color(&v, 'w', 0xf4f4f4);
    voxel_color(&v, 'a', 0x2a64e0);
    voxel_color(&v, 'y', 0xffb020);

    int bh = (tipo == 3) ? 3 : (tipo == 1 ? 5 : 4);     // alto del cuerpo (sobre el piso del chasis)
    int zs = 12, ze = 28, ch = 5;                        // cabina
    if (tipo == 1) { zs = 22; ze = 34; ch = 5; }         // camioneta: cabina adelante
    if (tipo == 3) { zs = 14; ze = 28; ch = 3; }
    int y0 = 2;
    voxel_caja(&v, 2, 1, 3, CW - 4, 1, CD - 6, 'u');     // bajos
    for (int z = 0; z < CD; z++) {
        int h = bh;
        if (z < 2 || z >= CD - 2) h = bh - 1;            // esquinas redondeadas
        if (tipo == 1 && z < zs) h = bh;                 // caja de la camioneta
        else if (z >= ze) h = bh - 1;                    // capó
        voxel_caja(&v, 1, y0, z, CW - 2, h, 1, 'c');
    }
    // cabina escalonada (parabrisas inclinado)
    for (int i = 0; i < ch; i++) {
        int z0 = zs + i, z1 = ze - i * 2;
        if (z1 <= z0) break;
        int ancho = CW - 4 - (i == ch - 1 ? 0 : 0);
        voxel_caja(&v, 2, y0 + bh + i, z0, ancho, 1, z1 - z0, i == ch - 1 ? 'c' : 'g');
        if (i < ch - 1) {
            voxel_caja(&v, 2, y0 + bh + i, z0, 1, 1, z1 - z0, 'd'); voxel_caja(&v, CW - 3, y0 + bh + i, z0, 1, 1, z1 - z0, 'd');   // marcos
            voxel_caja(&v, 2, y0 + bh + i, (z0 + z1) / 2, ancho, 1, 1, 'd');                                                       // pilar central
        }
    }
    if (tipo == 1) {                                     // caja trasera con borde y fondo
        voxel_caja(&v, 1, y0 + bh, 1, CW - 2, 1, zs - 2, 'c');
        voxel_caja(&v, 2, y0 + bh, 2, CW - 4, 1, zs - 4, 'u');
    }
    if (tipo == 3) voxel_caja(&v, 1, y0 + bh + 1, 1, CW - 2, 1, 2, 'c');                 // alerón
    if (tipo == 2) {                                     // patrulla: franja blanca, torreta
        voxel_caja(&v, 1, y0 + 1, zs - 4, CW - 2, 1, ze - zs + 8, 'w');
        voxel_caja(&v, 5, y0 + bh + ch, zs + 4, 3, 1, 3, 'r'); voxel_caja(&v, 10, y0 + bh + ch, zs + 4, 3, 1, 3, 'a');
    }
    if (tipo == 4) {                                     // taxi: cartel en el techo y franja
        voxel_caja(&v, 6, y0 + bh + ch, zs + 4, 6, 1, 3, 'w'); voxel_caja(&v, 1, y0 + 1, 3, CW - 2, 1, CD - 6, 'k');
    }
    // ruedas (negras con centro gris) a los 4 costados
    int rz[2] = { 5, CD - 9 };
    for (int i = 0; i < 2; i++) {
        for (int lado = 0; lado < 2; lado++) {
            int x = lado ? CW - 2 : 0;
            voxel_caja(&v, x, 0, rz[i], 2, 5, 5, 'k');
            voxel_caja(&v, x + (lado ? 1 : 0), 1, rz[i] + 1, 1, 3, 3, 'h');
            voxel_caja(&v, x + (lado ? 1 : 0), 2, rz[i] + 2, 1, 1, 1, 'k');
        }
    }
    // luces, parrilla, parachoques y placas
    voxel_caja(&v, 2, y0 + 1, CD - 1, 4, 2, 1, 'l'); voxel_caja(&v, CW - 6, y0 + 1, CD - 1, 4, 2, 1, 'l');
    voxel_caja(&v, 6, y0 + 1, CD - 1, 6, 2, 1, 'u');
    voxel_caja(&v, 1, 1, CD - 2, CW - 2, 1, 2, 'm'); voxel_caja(&v, 1, 1, 0, CW - 2, 1, 2, 'm');
    voxel_caja(&v, 2, y0 + 1, 0, 4, 2, 1, 'r'); voxel_caja(&v, CW - 6, y0 + 1, 0, 4, 2, 1, 'r');
    voxel_caja(&v, 7, y0, 0, 4, 1, 1, 'w');
    // líneas de puertas y espejos
    for (int lado = 0; lado < 2; lado++) {
        int x = lado ? CW - 2 : 1;
        voxel_caja(&v, x, y0 + 1, zs - 1, 1, bh - 1, 1, 'd');
        voxel_caja(&v, x, y0 + 1, (zs + ze) / 2 + 1, 1, bh - 1, 1, 'd');
        voxel_caja(&v, lado ? CW - 1 : 0, y0 + bh, ze - 3, 1, 1, 2, 'd');           // espejo
    }
    voxel_malla(&v, LADO, CW * 0.5f, 0.0f, CD * 0.5f, m);
    malla_subir(m);
    voxel_liberar(&v);
}

void vehiculos_iniciar(float cx, float cz) {
    for (int i = 0; i < n_carros; i++) malla_liberar(&carros[i].malla);
    n_carros = 0;
    for (int radio = 4; radio < 70 && n_carros < MAX_CARROS; radio += 1) {
        for (int f = 1; f < ciudad_alto() - 1 && n_carros < MAX_CARROS; f++) {
            for (int c = 1; c < ciudad_ancho() - 1 && n_carros < MAX_CARROS; c++) {
                float x = ciudad_centro_x(c), z = ciudad_centro_z(f);
                float d = fmaxf(fabsf(x - cx), fabsf(z - cz));
                if (d < radio || d >= radio + 1) continue;
                if (ciudad_tipo_en(c, f) != '=') continue;
                int hor = ciudad_tipo_en(c - 2, f) == '=' && ciudad_tipo_en(c + 2, f) == '=';
                int ver = ciudad_tipo_en(c, f - 2) == '=' && ciudad_tipo_en(c, f + 2) == '=';
                if (hor == ver) continue;
                if (ciudad_choca(x, z, 1.2f)) continue;
                int cerca = 0;
                for (int i = 0; i < n_carros; i++)
                    if (fabsf(carros[i].x - x) < 14.0f && fabsf(carros[i].z - z) < 14.0f) cerca = 1;
                if (cerca) continue;
                Carro *k = &carros[n_carros];
                k->x = x; k->z = z; k->vx = k->vz = k->vel = k->volante = 0.0f;
                k->ang = hor ? 90.0f : 0.0f;
                k->tipo = n_carros % 5;
                malla_iniciar(&k->malla);
                armar_carro(&k->malla, COLORES[(n_carros * 3) % 8], k->tipo);
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

void vehiculo_velocidad(int n, float *vx, float *vz) { *vx = carros[n].vx; *vz = carros[n].vz; }

#define OFS 1.1f
#define RAD 0.9f

int vehiculos_choca(float x, float z, float radio, int ignorar) {
    for (int n = 0; n < n_carros; n++) {
        if (n == ignorar) continue;
        float dx = carros[n].x - x, dz = carros[n].z - z;
        if (dx * dx + dz * dz > 9.0f) continue;
        float r = carros[n].ang * PI / 180.0f, sx = sinf(r), sz = cosf(r);
        for (int i = -1; i <= 1; i++) {
            float ex = carros[n].x + sx * i * OFS - x, ez = carros[n].z + sz * i * OFS - z, rr = RAD + radio;
            if (ex * ex + ez * ez < rr * rr) return 1;
        }
    }
    return 0;
}

static int choca(int n, float x, float z, float ang) {
    float r = ang * PI / 180.0f, sx = sinf(r), sz = cosf(r);
    for (int i = -1; i <= 1; i++)
        if (ciudad_choca(x + sx * i * OFS, z + sz * i * OFS, RAD) ||
            vehiculos_choca(x + sx * i * OFS, z + sz * i * OFS, RAD, n)) return 1;
    return 0;
}

// Los carros que nadie maneja siguen rodando un rato y se frenan solos
void vehiculos_pasivos(int activo) {
    for (int i = 0; i < n_carros; i++)
        if (i != activo && (fabsf(carros[i].vx) + fabsf(carros[i].vz) > 0.003f)) vehiculo_manejar(i, 0.0f, 0.0f, 0.25f, 0);
}

// Manejo estilo arcade (como GTA): acelerador, freno/reversa, volante suave, agarre lateral y freno de mano que derrapa.
void vehiculo_manejar(int n, float volante, float gas, float freno, int mano) {
    Carro *k = &carros[n];
    float r = k->ang * PI / 180.0f, fx = sinf(r), fz = cosf(r), rx = fz, rz = -fx;
    float vf = k->vx * fx + k->vz * fz, vl = k->vx * rx + k->vz * rz;

    // motor: la fuerza baja al acercarse a la velocidad máxima
    if (gas > 0.0f) vf += gas * 0.0065f * (1.0f - fmaxf(vf, 0.0f) / VEL_MAX);
    if (freno > 0.0f) {
        if (vf > 0.02f) vf -= freno * 0.022f;                                    // frena
        else            vf -= freno * 0.010f * (1.0f + vf / (VEL_MAX * 0.35f)); // reversa lenta
    }
    if (gas <= 0.0f && freno <= 0.0f) { vf *= 0.992f; if (fabsf(vf) < 0.004f) vf = 0.0f; }
    vf *= 0.9985f;
    if (vf < -VEL_MAX * 0.35f) vf = -VEL_MAX * 0.35f;

    // agarre: la velocidad de costado se apaga rápido (poco con freno de mano = derrape)
    vl *= mano ? 0.95f : 0.80f;
    if (mano) vf *= 0.985f;

    // volante: gira suave y se endurece con la velocidad
    k->volante += (volante - k->volante) * 0.22f;
    float rapido = fabsf(vf) / VEL_MAX;
    float max_giro = 0.58f / (1.0f + 2.8f * rapido * rapido);        // radianes (33° a baja velocidad)
    float gir = -tanf(k->volante * max_giro) * vf / EMPATE;          // radianes por cuadro
    if (mano) gir *= 1.5f;
    float nuevo = k->ang + gir * 57.29578f;
    if (!choca(n, k->x, k->z, nuevo)) k->ang = nuevo;
    else vf *= 0.9f;

    r = k->ang * PI / 180.0f; fx = sinf(r); fz = cosf(r); rx = fz; rz = -fx;
    k->vx = fx * vf + rx * vl; k->vz = fz * vf + rz * vl;

    float nx = k->x + k->vx, nz = k->z + k->vz;
    int c1 = choca(n, nx, k->z, k->ang), c2 = choca(n, k->x, nz, k->ang);
    if (!c1) k->x = nx; else { k->vx = -k->vx * 0.25f; }
    if (!c2) k->z = nz; else { k->vz = -k->vz * 0.25f; }
    if (c1 || c2) { k->vx *= 0.8f; k->vz *= 0.8f; }
    k->vel = k->vx * sinf(k->ang * PI / 180.0f) + k->vz * cosf(k->ang * PI / 180.0f);
}
