#include "world/vehiculos.h"
#include "world/ciudad.h"
#include "engine/voxel.h"
#include "engine/malla.h"
#include "core/util.h"
#include <vitaGL.h>
#include <math.h>

#define MAX_CARROS 12
#define LADO 0.1f
#define CW 20          // ancho en cubitos (1.8)
#define CD 36          // largo en cubitos (4.0)
#define CH 19          // alto en cubitos
#define VEL_MAX 0.55f  // unidades por cuadro (~33 u/s)
#define EMPATE 2.4f    // distancia entre ejes

typedef struct { float x, z, ang, vx, vz, vel, volante; int tipo; Malla malla; } Carro;
static Carro carros[MAX_CARROS];
static int n_carros = 0;

static const unsigned int COLORES[] = { 0xd9382c, 0x2f62c9, 0x3aa655, 0xf2f2f2, 0x8a3fc0, 0xe8742a, 0x303338, 0xf2c230 };

// Carro de cubitos de 0.1 (frente hacia +z): SUV cuadrado con ventanas abiertas, asientos y tablero,
// para que se vea el conductor por dentro. Todos iguales, solo cambia el color.
static void armar_carro(Malla *m, unsigned int color, int tipo) {
    (void)tipo;
    Voxeles v;
    voxel_crear(&v, CW, CD, CH);
    voxel_color(&v, 'c', color);
    voxel_color(&v, 'd', ((color >> 1) & 0x7f7f7f));
    voxel_color(&v, 'k', 0x151518);
    voxel_color(&v, 'h', 0xb4b8be);
    voxel_color(&v, 'l', 0xffee30);
    voxel_color(&v, 'r', 0xe02020);
    voxel_color(&v, 'm', 0xa8adb4);
    voxel_color(&v, 'u', 0x26262c);
    voxel_color(&v, 'w', 0xfff8d0);
    voxel_color(&v, 's', 0x2c4a5c);     // asientos
    voxel_color(&v, 'g', 0x8fd8ee);     // vidrio
    voxel_color(&v, 'G', 0x3c88a4);     // vidrio oscuro (se ven los asientos)
    voxel_color(&v, 'y', 0xffa818);
    // carrocería ancha y baja, chasis
    voxel_caja(&v, 2, 1, 2, 16, 1, 32, 'u');
    voxel_caja(&v, 1, 2, 0, 18, 6, 36, 'c');
    voxel_caja(&v, 4, 8, 26, 12, 1, 7, 'c');                      // capó con bloque levantado
    voxel_caja(&v, 6, 9, 27, 8, 1, 4, 'c');
    // interior
    voxel_caja(&v, 2, 8, 20, 16, 2, 3, 's');                      // tablero
    voxel_caja(&v, 12, 10, 19, 4, 1, 1, 'k');                     // volante
    int xs[2] = { 3, 12 };
    for (int i = 0; i < 2; i++) {
        voxel_caja(&v, xs[i], 8, 10, 5, 2, 5, 's');
        voxel_caja(&v, xs[i], 10, 8, 5, 3, 2, 's');
    }
    voxel_caja(&v, 3, 8, 5, 14, 2, 3, 's');
    // techo, pilares y vidrios
    voxel_caja(&v, 1, 18, 4, 18, 1, 21, 'c');
    voxel_caja(&v, 1, 8, 22, 2, 10, 3, 'c'); voxel_caja(&v, 17, 8, 22, 2, 10, 3, 'c');   // pilares A
    voxel_caja(&v, 1, 8, 13, 2, 10, 2, 'c'); voxel_caja(&v, 17, 8, 13, 2, 10, 2, 'c');   // pilares B
    voxel_caja(&v, 1, 8, 4, 2, 10, 2, 'c');  voxel_caja(&v, 17, 8, 4, 2, 10, 2, 'c');    // pilares C
    voxel_caja(&v, 3, 8, 4, 14, 2, 1, 'c');                                               // base de la ventana trasera (abierta)
    voxel_caja(&v, 3, 10, 24, 14, 8, 1, 'g');                                             // parabrisas
    voxel_caja(&v, 3, 10, 24, 14, 2, 1, 'G'); voxel_caja(&v, 4, 13, 24, 4, 3, 1, 'G'); voxel_caja(&v, 12, 12, 24, 4, 4, 1, 'G');
    voxel_caja(&v, 1, 11, 15, 1, 6, 7, 'g'); voxel_caja(&v, 18, 11, 15, 1, 6, 7, 'g');   // vidrios de las puertas
    voxel_caja(&v, 1, 11, 6, 1, 6, 7, 'g'); voxel_caja(&v, 18, 11, 6, 1, 6, 7, 'g');
    voxel_caja(&v, 0, 11, 22, 1, 3, 3, 'c'); voxel_caja(&v, 19, 11, 22, 1, 3, 3, 'c');   // espejos
    voxel_caja(&v, 1, 8, 12, 1, 4, 1, 'd'); voxel_caja(&v, 18, 8, 12, 1, 4, 1, 'd');     // líneas de puerta
    voxel_caja(&v, 1, 9, 16, 1, 1, 2, 'm'); voxel_caja(&v, 18, 9, 16, 1, 1, 2, 'm');     // manijas
    // frente: faros, parrilla, parachoques
    voxel_caja(&v, 1, 4, 35, 5, 3, 1, 'l'); voxel_caja(&v, 14, 4, 35, 5, 3, 1, 'l');
    voxel_caja(&v, 2, 5, 35, 2, 1, 1, 'w'); voxel_caja(&v, 16, 5, 35, 2, 1, 1, 'w');
    voxel_caja(&v, 6, 4, 35, 8, 3, 1, 'u'); voxel_caja(&v, 7, 5, 35, 6, 1, 1, 'k');
    voxel_caja(&v, 0, 3, 34, 20, 1, 2, 'm');
    voxel_caja(&v, 1, 1, 34, 18, 2, 2, 'k'); voxel_caja(&v, 7, 1, 35, 6, 1, 1, 'u');
    voxel_caja(&v, 2, 2, 35, 4, 1, 1, 'y'); voxel_caja(&v, 14, 2, 35, 4, 1, 1, 'y');
    // atrás
    voxel_caja(&v, 1, 4, 0, 4, 3, 1, 'r'); voxel_caja(&v, 15, 4, 0, 4, 3, 1, 'r');
    voxel_caja(&v, 0, 3, 0, 20, 1, 2, 'm'); voxel_caja(&v, 1, 1, 0, 18, 2, 2, 'k');
    voxel_caja(&v, 8, 4, 0, 4, 2, 1, 'w');
    // ruedas con rin
    int rz[2] = { 4, CD - 10 };
    for (int i = 0; i < 2; i++) {
        for (int lado = 0; lado < 2; lado++) {
            int x = lado ? CW - 2 : 0;
            voxel_caja(&v, x, 0, rz[i], 2, 6, 6, 'k');
            voxel_caja(&v, x + (lado ? 1 : 0), 1, rz[i] + 1, 1, 4, 4, 'h');
            voxel_caja(&v, x + (lado ? 1 : 0), 2, rz[i] + 2, 1, 2, 2, 'k');
        }
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

// Manejo nuevo (arcade simple y predecible): el carro SIEMPRE avanza hacia donde apunta.
//  gas/freno cambian la rapidez; el volante gira el carro según la rapidez; sin deslizamiento lateral.
//  mano = freno de mano: frena fuerte y gira más cerrado.
void vehiculo_manejar(int n, float volante, float gas, float freno, int mano) {
    Carro *k = &carros[n];
    float v = k->vel;                                   // rapidez con signo (+ adelante)
    const float REV_MAX = VEL_MAX * 0.3f;

    if (gas > 0.0f) {
        if (v < 0.0f) v += gas * 0.03f;                 // viene en reversa: primero frena
        else v += gas * 0.008f * (1.0f - v / VEL_MAX);
    }
    if (freno > 0.0f) {
        if (v > 0.0f) { v -= freno * 0.02f; if (v < 0.0f) v = 0.0f; }
        else v -= freno * 0.006f;                       // reversa
    }
    if (gas <= 0.0f && freno <= 0.0f) {                 // sin tocar nada: se detiene poco a poco
        v *= 0.985f;
        if (fabsf(v) < 0.004f) v = 0.0f;
    }
    if (mano) { v *= 0.93f; if (fabsf(v) < 0.004f) v = 0.0f; }
    if (v > VEL_MAX) v = VEL_MAX;
    if (v < -REV_MAX) v = -REV_MAX;

    // volante: suave, y gira menos cuando vas rápido; casi no gira si estás parado
    k->volante += (volante - k->volante) * 0.25f;
    float rel = fabsf(v) / VEL_MAX;
    float agarre = fminf(1.0f, fabsf(v) / 0.10f);                    // parado = no gira
    float giro = k->volante * (2.6f - 1.6f * rel) * agarre;          // grados por cuadro
    if (mano) giro *= 1.6f;
    if (v < 0.0f) giro = -giro;
    float nuevo = k->ang - giro;
    if (!choca(n, k->x, k->z, nuevo)) k->ang = nuevo;
    else v *= 0.9f;

    float r = k->ang * PI / 180.0f, fx = sinf(r), fz = cosf(r);
    float nx = k->x + fx * v, nz = k->z + fz * v;
    if (!choca(n, nx, nz, k->ang)) { k->x = nx; k->z = nz; }
    else if (!choca(n, nx, k->z, k->ang)) k->x = nx, v *= 0.6f;      // roza una pared: se desliza
    else if (!choca(n, k->x, nz, k->ang)) k->z = nz, v *= 0.6f;
    else v = -v * 0.2f;                                              // choque de frente: rebota un poquito

    k->vel = v;
    k->vx = fx * v; k->vz = fz * v;
}
