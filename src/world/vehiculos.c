#include "world/vehiculos.h"
#include "world/ciudad.h"
#include "world/escombros.h"
#include "world/npcs.h"
#include "engine/voxel.h"
#include "engine/malla.h"
#include "core/util.h"
#include <vitaGL.h>
#include <math.h>

#define MAX_CARROS 24
#define N_PARQUEADOS 12   // los primeros 12 están estacionados; el resto circula solo
#define LADO 0.1f
#define CW 20          // ancho en cubitos (1.8)
#define CD 36          // largo en cubitos (4.0)
#define CH 19          // alto en cubitos
#define VEL_MAX 0.55f  // unidades por cuadro (~33 u/s)
#define EMPATE 2.4f    // distancia entre ejes

typedef struct { float x, z, ang, vx, vz, vel, volante; int tipo; Malla malla; unsigned color; int danio; float roll, pitch; float alt, valt, wp, wr;
               int ia, meta_cd, trabado, fuera; float meta; } Carro;
static Carro carros[MAX_CARROS];
static void ia_crear_todos(float cx, float cz);
static int n_carros = 0;

static const unsigned int COLORES[] = { 0xd9382c, 0x2f62c9, 0x3aa655, 0xf2f2f2, 0x8a3fc0, 0xe8742a, 0x303338, 0xf2c230 };

// Carro de cubitos de 0.1 (frente hacia +z): SUV cuadrado con ventanas abiertas, asientos y tablero,
// para que se vea el conductor por dentro. Todos iguales, solo cambia el color.
static void armar_carro(Malla *m, unsigned int color, int tipo, int danio) {
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
    if (danio) {                                                   // frente destrozado: capó arrugado, faros y parachoques arrancados
        for (int z = 25; z < CD; z++)
            for (int y = 1; y < CH; y++)
                for (int x = 0; x < CW; x++) {
                    if (!voxel_hay(&v, x, y, z)) continue;
                    unsigned h = (unsigned)(x * 73856093u ^ y * 19349663u ^ z * 83492791u) % 100u;
                    int quitar = (int)h < (z - 24) * 9 && (y > 2 || z > 31);
                    if (z >= 32 && y >= 3) quitar = quitar || h < 70;
                    if (y >= 9 && z >= 26) quitar = quitar || h < 40;      // el capó se hunde
                    if (quitar) voxel_poner(&v, x, y, z, '.');
                    else if (h % 3 == 0) voxel_poner(&v, x, y, z, 'd');   // chapa chamuscada
                }
    }
    voxel_malla(&v, LADO, CW * 0.5f, 0.0f, CD * 0.5f, m);
    malla_subir(m);
    voxel_liberar(&v);
}

void vehiculos_iniciar(float cx, float cz) {
    for (int i = 0; i < n_carros; i++) malla_liberar(&carros[i].malla);
    n_carros = 0;
    for (int radio = 4; radio < 70 && n_carros < N_PARQUEADOS; radio += 1) {
        for (int f = 1; f < ciudad_alto() - 1 && n_carros < N_PARQUEADOS; f++) {
            for (int c = 1; c < ciudad_ancho() - 1 && n_carros < N_PARQUEADOS; c++) {
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
                k->color = COLORES[(n_carros * 3) % 8]; k->danio = 0; k->roll = k->pitch = 0.0f; k->alt = k->valt = k->wp = k->wr = 0.0f; k->ia = 0;
                armar_carro(&k->malla, k->color, k->tipo, 0);
                n_carros++;
            }
        }
    }
    ia_crear_todos(cx, cz);
}

void vehiculos_dibujar(float px, float pz) {
    for (int i = 0; i < n_carros; i++) {
        const Carro *k = &carros[i];
#ifdef __3DS__
        if (fabsf(k->x - px) > 68.0f || fabsf(k->z - pz) > 68.0f) continue;
#else
        if (fabsf(k->x - px) > 90.0f || fabsf(k->z - pz) > 90.0f) continue;
#endif
        glPushMatrix();
        glTranslatef(k->x, 0.0f, k->z);
        glTranslatef(0.0f, k->alt, 0.0f);                // en el aire cuando sale volando
        glRotatef(k->ang, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, 0.9f, 0.0f);                  // gira alrededor del centro del carro
        glRotatef(k->pitch, 1.0f, 0.0f, 0.0f);          // suspensión: cabeceo y balanceo (y volteretas)
        glRotatef(k->roll, 0.0f, 0.0f, 1.0f);
        glTranslatef(0.0f, -0.9f, 0.0f);
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

void vehiculo_poner(int n, float x, float z, float ang) { carros[n].x = x; carros[n].z = z; carros[n].ang = ang; carros[n].vel = carros[n].vx = carros[n].vz = 0.0f; }
int vehiculo_danio(int n) { return carros[n].danio; }
void vehiculo_inclinacion(int n, float *roll, float *pitch, float *alt) { *roll = carros[n].roll; *pitch = carros[n].pitch; *alt = carros[n].alt; }

// 1 si un carro que va rápido toca el círculo (y da su velocidad): así se atropella
int vehiculos_atropella(float x, float z, float radio, float *vx, float *vz) {
    for (int n = 0; n < n_carros; n++) {
        if (fabsf(carros[n].vel) < 0.07f) continue;
        float dx = carros[n].x - x, dz = carros[n].z - z;
        if (dx * dx + dz * dz > 9.0f) continue;
        float r = carros[n].ang * PI / 180.0f, sx = sinf(r), sz = cosf(r);
        for (int i = -1; i <= 1; i++) {
            float ex = carros[n].x + sx * i * OFS - x, ez = carros[n].z + sz * i * OFS - z, rr = RAD + radio;
            if (ex * ex + ez * ez < rr * rr) { *vx = carros[n].vx; *vz = carros[n].vz; return 1; }
        }
    }
    return 0;
}

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
        if (i != activo && !carros[i].ia && (fabsf(carros[i].vx) + fabsf(carros[i].vz) > 0.003f || carros[i].alt > 0.0f)) vehiculo_manejar(i, 0.0f, 0.0f, 0.0f, 1);   // freno de mano: se detiene, nunca se va en reversa
}

static int n_choques = 0;
int vehiculos_choques(void) { return n_choques; }
static void choque_frontal(Carro *k, float v) {
    n_choques++;
    float r = k->ang * PI / 180.0f, fx = sinf(r), fz = cosf(r);
    float px = k->x + fx * 2.1f, pz = k->z + fz * 2.1f;
    escombros_crear(px, 0.9f, pz, fx * v * 0.5f, fz * v * 0.5f, k->color, 14, 0.2f, 0.12f);
    escombros_crear(px, 0.7f, pz, fx * v * 0.4f, fz * v * 0.4f, 0x303030, 6, 0.18f, 0.1f);
    escombros_crear(px, 0.8f, pz, fx * v * 0.4f, fz * v * 0.4f, 0xffee30, 3, 0.15f, 0.1f);
    unsigned col;
    if (ciudad_destruir(px + fx * 0.8f, 1.0f, pz + fz * 0.8f, 1.4f, &col))
        escombros_crear(px + fx * 0.8f, 1.2f, pz + fz * 0.8f, fx * v * 0.5f, fz * v * 0.5f, col, 26, 0.3f, 0.14f);
    if (v >= 0.38f) {                                              // el choque levanta el carro; muy rápido: da una vuelta completa
        k->alt = 0.001f;
        if (v < 0.48f) { k->valt = 0.12f; k->pitch = -20.0f; k->wp = 0.8f; k->wr = 0.0f; }
        else {
            k->valt = 0.20f + (v - 0.48f) * 1.6f;
            float T = 2.0f * k->valt / 0.012f;
            k->wp = -360.0f / T; k->wr = (v >= 0.52f) ? 360.0f / T : 0.0f;
        }
    }
    if (!k->danio) {                                               // se arma la versión destrozada
        k->danio = 1;
        malla_liberar(&k->malla); malla_iniciar(&k->malla);
        armar_carro(&k->malla, k->color, k->tipo, 1);
    }
}

// Si el carro quedó encajado dentro de otro después de un choque, los separa poco a poco (así nunca se quedan pegados)
static void separar(int n) {
    Carro *a = &carros[n];
    for (int m = 0; m < n_carros; m++) {
        if (m == n) continue;
        Carro *b = &carros[m];
        float dx0 = a->x - b->x, dz0 = a->z - b->z;
        if (dx0 * dx0 + dz0 * dz0 > 25.0f) continue;
        float ra = a->ang * PI / 180.0f, rb = b->ang * PI / 180.0f;
        float mejor = 0.0f, nx = 0.0f, nz = 0.0f;
        for (int i = -1; i <= 1; i++)
            for (int j = -1; j <= 1; j++) {
                float ex = a->x + sinf(ra) * i * OFS - (b->x + sinf(rb) * j * OFS);
                float ez = a->z + cosf(ra) * i * OFS - (b->z + cosf(rb) * j * OFS);
                float d = sqrtf(ex * ex + ez * ez), pen = 2.0f * RAD + 0.18f - d;     // deja un poquito de aire para que puedan moverse
                if (pen > mejor) { mejor = pen; if (d > 0.001f) { nx = ex / d; nz = ez / d; } else { nx = 1.0f; nz = 0.0f; } }
            }
        if (mejor <= 0.0f) continue;
        float p = mejor * 0.5f + 0.02f; if (p > 0.12f) p = 0.12f;
        float ax = a->x + nx * p, az = a->z + nz * p, bx = b->x - nx * p, bz = b->z - nz * p;
        if (!ciudad_choca(ax, az, RAD * 0.6f)) { a->x = ax; a->z = az; }
        if (!ciudad_choca(bx, bz, RAD * 0.6f)) { b->x = bx; b->z = bz; }
    }
}

// Carro en el aire (después de un choque fuerte): sin control, cae con la gravedad
static void en_el_aire(int n, Carro *k) {
    float v = k->vel;
    k->valt -= 0.012f; k->alt += k->valt; k->pitch += k->wp; k->roll += k->wr;
    float r = k->ang * PI / 180.0f, fx = sinf(r), fz = cosf(r);
    float nx = k->x + fx * v, nz = k->z + fz * v;
    if (!choca(n, nx, nz, k->ang)) { k->x = nx; k->z = nz; } else v *= 0.3f;
    v *= 0.995f;
    if (k->alt <= 0.0f) {
        k->alt = 0.0f;
        if (k->valt < -0.07f) {                                    // rebota un poco al caer
            k->valt = -k->valt * 0.28f; k->alt = 0.001f; k->wp *= 0.3f; k->wr *= 0.3f; v *= 0.7f;
            escombros_crear(k->x, 0.3f, k->z, 0.0f, 0.0f, 0x909090, 6, 0.15f, 0.08f);
        } else {
            k->valt = 0.0f; k->wp = k->wr = 0.0f;
            k->pitch = fmodf(k->pitch, 360.0f); if (k->pitch > 180.0f) k->pitch -= 360.0f; if (k->pitch < -180.0f) k->pitch += 360.0f;
            k->roll = fmodf(k->roll, 360.0f); if (k->roll > 180.0f) k->roll -= 360.0f; if (k->roll < -180.0f) k->roll += 360.0f;
        }
    }
    k->vel = v; k->vx = fx * v; k->vz = fz * v;
}

// Manejo nuevo (arcade simple y predecible): el carro SIEMPRE avanza hacia donde apunta.
//  gas/freno cambian la rapidez; el volante gira el carro según la rapidez; sin deslizamiento lateral.
//  mano = freno de mano: frena fuerte y gira más cerrado.
void vehiculo_manejar(int n, float volante, float gas, float freno, int mano) {
    Carro *k = &carros[n];
    if (k->alt > 0.0f) { en_el_aire(n, k); return; }
    separar(n);
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
    float vmax = k->danio ? VEL_MAX * 0.8f : VEL_MAX;   // con el frente roto corre menos
    if (v > vmax) v = vmax;
    if (v < -REV_MAX) v = -REV_MAX;
    float v_antes = k->vel;

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
    if (v > 0.38f && choca(n, nx, nz, k->ang)) {
        choque_frontal(k, v);
        v = (k->alt > 0.0f && v >= 0.48f) ? -0.08f : v * 0.4f;     // la vuelta completa lo manda hacia atrás
        nx = k->x + fx * v; nz = k->z + fz * v;
    }   // ¡a toda velocidad! se rompe el frente
    if (!choca(n, nx, nz, k->ang)) { k->x = nx; k->z = nz; }
    else if (!choca(n, nx, k->z, k->ang)) k->x = nx, v *= 0.6f;      // roza una pared: se desliza
    else if (!choca(n, k->x, nz, k->ang)) k->z = nz, v *= 0.6f;
    else {
        v = -v * 0.2f;                                               // choque de frente: rebota un poquito
    }

    // suspensión: se inclina al girar (balanceo) y al acelerar/frenar (cabeceo)
    float roll_obj = -k->volante * fminf(1.0f, fabsf(v) / 0.35f) * 7.0f;
    float pitch_obj = -(v - v_antes) * 260.0f;
    if (pitch_obj > 5.0f) pitch_obj = 5.0f;
    if (pitch_obj < -5.0f) pitch_obj = -5.0f;
    k->roll += (roll_obj - k->roll) * 0.15f;
    k->pitch += (pitch_obj - k->pitch) * 0.15f;

    k->vel = v;
    k->vx = fx * v; k->vz = fz * v;
}

int vehiculos_cantidad(void) { return n_carros; }
void vehiculo_tomar(int n) { carros[n].ia = 0; }

// ---------- Tráfico con IA: circulan por su carril, giran en las esquinas y frenan si hay algo adelante ----------
static unsigned ia_s = 4242u;
static unsigned ia_azar(void) { ia_s = ia_s * 1664525u + 1013904223u; return ia_s >> 8; }

static void ia_reubicar(Carro *k, float px, float pz, float dmin) {
    for (int t = 0; t < 400; t++) {
        int c = (int)(px / CIUDAD_TAM + ciudad_ancho() * 0.5f) + (int)(ia_azar() % 221) - 110;
        int f = (int)(pz / CIUDAD_TAM + ciudad_alto() * 0.5f) + (int)(ia_azar() % 221) - 110;
        if (c < 3 || f < 3 || c >= ciudad_ancho() - 3 || f >= ciudad_alto() - 3) continue;
        if (ciudad_tipo_en(c, f) != '=') continue;
        float x = ciudad_centro_x(c), z = ciudad_centro_z(f);
        if (fmaxf(fabsf(x - px), fabsf(z - pz)) < dmin) continue;
        int hor = ciudad_tipo_en(c - 2, f) == '=' && ciudad_tipo_en(c + 2, f) == '=';
        int ver = ciudad_tipo_en(c, f - 2) == '=' && ciudad_tipo_en(c, f + 2) == '=';
        if (hor == ver) continue;
        if (ciudad_choca(x, z, 1.5f) || vehiculos_choca(x, z, 3.0f, -1)) continue;
        k->x = x; k->z = z;
        k->ang = hor ? ((ia_azar() & 1) ? 90.0f : 270.0f) : ((ia_azar() & 1) ? 0.0f : 180.0f);
        k->meta = k->ang; k->vel = 0.0f; k->vx = k->vz = 0.0f; k->trabado = 0; k->meta_cd = 60;
        return;
    }
}

static void ia_crear_todos(float cx, float cz) {
    while (n_carros < MAX_CARROS) {
        Carro *k = &carros[n_carros];
        k->x = 99999.0f; k->z = 99999.0f; k->vx = k->vz = k->vel = k->volante = 0.0f;
        k->tipo = 0; k->color = COLORES[(n_carros * 5 + 1) % 8]; k->danio = 0; k->roll = k->pitch = 0.0f;
        k->alt = k->valt = k->wp = k->wr = 0.0f; k->ia = 1; k->fuera = 0; k->meta_cd = 0; k->trabado = 0; k->meta = 0.0f;
        malla_iniciar(&k->malla);
        armar_carro(&k->malla, k->color, 0, 0);
        n_carros++;
        ia_reubicar(k, cx, cz, 30.0f);
        if (k->x > 90000.0f) { k->ia = 0; }          // no hubo sitio: queda sin usar lejos
    }
}

static float dif_ang(float a, float b) { float d = b - a; while (d > 180.0f) d -= 360.0f; while (d < -180.0f) d += 360.0f; return d; }
static int es_calle(float x, float z) {
    int c = (int)floorf(x / CIUDAD_TAM + ciudad_ancho() * 0.5f), f = (int)floorf(z / CIUDAD_TAM + ciudad_alto() * 0.5f);
    return ciudad_tipo_en(c, f) == '=';
}

void vehiculos_ia(float px, float pz, int activo) {
    for (int i = 0; i < n_carros; i++) {
        Carro *k = &carros[i];
        if (!k->ia || i == activo) continue;
        if (fmaxf(fabsf(k->x - px), fabsf(k->z - pz)) > 140.0f) { ia_reubicar(k, px, pz, 45.0f); continue; }
        if (fmaxf(fabsf(k->x - px), fabsf(k->z - pz)) > 100.0f) continue;     // lejos: congelado
        float r = k->ang * PI / 180.0f, fx = sinf(r), fz = cosf(r);
        float rx = -fz, rz = fx;                                              // derecha del carro
        if (!es_calle(k->x, k->z)) { if (++k->fuera > 40) { k->fuera = 0; ia_reubicar(k, px, pz, 30.0f); continue; } } else k->fuera = 0;
        // ¿hay algo adelante? (otro carro, una persona o el jugador)
        int bloqueado = 0;
        for (float d = 2.8f; d <= 6.0f && !bloqueado; d += 1.6f) {
            float ax = k->x + fx * d, az = k->z + fz * d;
            if (vehiculos_choca(ax, az, 0.9f, i) || npcs_choca(ax, az, 0.5f, -1)) bloqueado = 1;
            float jx = ax - px, jz = az - pz; if (jx * jx + jz * jz < 1.6f * 1.6f) bloqueado = 1;
        }
        // decisión en cruces y finales de calle
        if (k->meta_cd > 0) k->meta_cd--;
        int c = (int)floorf(k->x / CIUDAD_TAM + ciudad_ancho() * 0.5f), f = (int)floorf(k->z / CIUDAD_TAM + ciudad_alto() * 0.5f);
        int en_cruce = ciudad_tipo_en(c, f) == '=' && ciudad_tipo_en(c - 4, f) == '=' && ciudad_tipo_en(c + 4, f) == '=' &&
                       ciudad_tipo_en(c, f - 4) == '=' && ciudad_tipo_en(c, f + 4) == '=';
        if (fabsf(dif_ang(k->ang, k->meta)) < 3.0f && k->meta_cd == 0) {
            if (en_cruce) {
                int op = (int)(ia_azar() % 4);                                // 0,1 recto · 2 derecha · 3 izquierda
                float m = k->ang + (op < 2 ? 0.0f : op == 2 ? 90.0f : -90.0f);
                float mr = m * PI / 180.0f;
                if (es_calle(k->x + sinf(mr) * 7.0f, k->z + cosf(mr) * 7.0f)) { k->meta = m; k->meta_cd = 120; }
            } else if (!es_calle(k->x + fx * 4.0f, k->z + fz * 4.0f)) {        // se acabó la calle: da la vuelta
                k->meta = k->ang + 180.0f; k->meta_cd = 120;
            }
        }
        float giro = dif_ang(k->ang, k->meta);
        float vt = bloqueado ? 0.0f : (fabsf(giro) > 3.0f ? 0.07f : 0.17f);
        k->vel += (vt - k->vel) * (bloqueado ? 0.25f : 0.05f);
        if (fabsf(giro) > 0.5f && k->vel > 0.03f) { float p = fminf(2.0f, fabsf(giro)); k->ang += giro > 0 ? p : -p; if (k->ang >= 360.0f) k->ang -= 360.0f; if (k->ang < 0.0f) k->ang += 360.0f; }
        // mantenerse en su carril (a la derecha del centro de la calle) cuando va recto
        if (fabsf(giro) < 3.0f && !en_cruce) {
            int horiz = fabsf(fx) > 0.7f;
            int n1 = 0, n2 = 0;
            while (n1 < 8 && (horiz ? ciudad_tipo_en(c, f - n1 - 1) : ciudad_tipo_en(c - n1 - 1, f)) == '=') n1++;
            while (n2 < 8 && (horiz ? ciudad_tipo_en(c, f + n2 + 1) : ciudad_tipo_en(c + n2 + 1, f)) == '=') n2++;
            if (n1 + n2 + 1 <= 8) {
                float centro = horiz ? (ciudad_centro_z(f - n1) + ciudad_centro_z(f + n2)) * 0.5f : (ciudad_centro_x(c - n1) + ciudad_centro_x(c + n2)) * 0.5f;
                float objetivo = centro + (horiz ? rz : rx) * 1.6f, actual = horiz ? k->z : k->x, e = objetivo - actual;
                if (e > 0.04f) e = 0.04f; if (e < -0.04f) e = -0.04f;
                if (horiz) k->z += e; else k->x += e;
            }
        }
        k->x += fx * k->vel; k->z += fz * k->vel;
        k->vx = fx * k->vel; k->vz = fz * k->vel;
        k->roll += (-giro * 0.12f * (k->vel / 0.17f) - k->roll) * 0.1f;
        if (bloqueado && k->vel < 0.01f) { if (++k->trabado > 360) { k->meta = k->ang + 180.0f; k->trabado = 0; k->meta_cd = 0; } } else k->trabado = 0;
    }
}
