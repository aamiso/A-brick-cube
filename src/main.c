// A Brick Cube Vita - Fase 5: ciudad voxel con casas, banco y café (modelos), calles con detalle y personaje animado (data/mapas/ciudad.txt)
//
// Controles:
//   Stick izquierdo = caminar (o manejar)
//   R = correr (mantener)    X = saltar    L = cambiar de cámara (normal / sobre el hombro)
//   TRIÁNGULO = subir / bajar del carro    CUADRADO = freno de mano
//   Stick derecho   = girar la cámara (izq/der) y subirla o bajarla (arriba/abajo)
//   START           = salir

#include <vitaGL.h>
#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>

#include "core/entrada.h"
#include "engine/cubo.h"
#include "engine/camara.h"
#include "player/jugador.h"
#include "world/suelo.h"
#include "world/ciudad.h"
#include "world/frontera.h"
#include "world/terreno.h"
#include "world/vehiculos.h"
#include <math.h>

int main(void) {
    vglInit(0x1000000);
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    cubo_construir();

    // Lee el mapa (si no lo encuentra, usa uno chiquito incluido en el juego)
    ciudad_cargar("app0:data/mapas/ciudad.txt");

    Jugador jugador;
    Camara camara;
    jugador_iniciar(&jugador);
    ciudad_inicio(&jugador.x, &jugador.z); // empieza donde dice la P del mapa
    ciudad_precalentar(fx, fz);
    camara_iniciar(&camara);
    vehiculos_iniciar(fx, fz);
    int en_carro = -1;
    unsigned prev = 0;

    while (1) {
        SceCtrlData pad;
        sceCtrlPeekBufferPositive(0, &pad, 1);
        if (pad.buttons & SCE_CTRL_START) break;

        // --- Actualizar ---
        unsigned nuevos = pad.buttons & ~prev;
        prev = pad.buttons;
        float lx = leer_stick(pad.lx), ly = leer_stick(pad.ly);
        float rx = leer_stick(pad.rx), ry = leer_stick(pad.ry);
        camara_actualizar(&camara, rx, ry, (nuevos & SCE_CTRL_LTRIGGER) != 0);
        float fx = jugador.x, fz = jugador.z;   // lo que sigue la cámara

        if (en_carro < 0) {
            jugador_actualizar(&jugador, lx, ly, camara.giro, (pad.buttons & SCE_CTRL_RTRIGGER) != 0,
                               (nuevos & SCE_CTRL_CROSS) != 0);
            if (nuevos & SCE_CTRL_TRIANGLE) en_carro = vehiculo_cercano(jugador.x, jugador.z, 3.5f);
            fx = jugador.x; fz = jugador.z;
            camara.dist_meta = 6.5f;
        } else {
            float ang, vel;
            vehiculo_manejar(en_carro, lx, ly, (pad.buttons & SCE_CTRL_SQUARE) != 0);
            vehiculo_pos(en_carro, &fx, &fz, &ang, &vel);
            camara.dist_meta = 9.5f;
            if (fabsf(rx) < 0.05f && fabsf(vel) > 0.05f) camara_seguir(&camara, vel > 0 ? ang : ang + 180.0f, 0.03f);
            jugador.x = fx; jugador.z = fz; jugador.vel = 0.0f;
            if (nuevos & SCE_CTRL_TRIANGLE) {
                // se baja por el lado derecho (o el primer lado libre)
                float r = ang * 3.14159265f / 180.0f;
                int ok = 0;
                for (int k = 0; k < 4 && !ok; k++) {
                    float a2 = r + 1.5708f * (k == 0 ? 1 : k == 1 ? -1 : k == 2 ? 2 : 0);
                    float ex = fx + sinf(a2) * 1.9f, ez = fz + cosf(a2) * 1.9f;
                    if (!ciudad_choca(ex, ez, 0.45f)) { jugador.x = ex; jugador.z = ez; ok = 1; }
                }
                if (ok) { jugador.angulo = ang; en_carro = -1; }
            }
        }

        // --- Dibujar ---
        glViewport(0, 0, 960, 544);
        glClearColor(0.5f, 0.75f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-0.96f, 0.96f, -0.544f, 0.544f, 1.0f, 330.0f);

        camara_aplicar(&camara, fx, fz);

        cubo_activar();
        // 1) Todo lo plano (mar, arena, pasto, calles) se pinta en orden y sin profundidad:
        //    así nunca se mezclan ni se ven rayitas entre ellos.
        glDisable(GL_DEPTH_TEST);
        frontera_dibujar_suelo(fx, fz);
        suelo_dibujar(fx, fz);
        ciudad_dibujar_suelo(fx, fz);
        terreno_dibujar(fx, fz, camara.giro);   // detalle de cerca: rayas, bordillos, cebras
        // 2) Lo que tiene volumen, con profundidad.
        glEnable(GL_DEPTH_TEST);
        ciudad_dibujar(fx, fz, camara.giro);   // edificios, árboles y lugares con modelo
        frontera_dibujar(fx, fz);
        vehiculos_dibujar(fx, fz);
        if (en_carro < 0) jugador_dibujar(&jugador);
        cubo_desactivar();

        vglSwapBuffers(GL_FALSE);
    }

    sceKernelExitProcess(0);
    return 0;
}
