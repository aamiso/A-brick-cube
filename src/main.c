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
#include "world/npcs.h"
#include "ui/ui.h"
#include "ui/dialogo.h"
#include <math.h>
#include <stdio.h>

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
    ciudad_precalentar(jugador.x, jugador.z);
    camara_iniciar(&camara);
    vehiculos_iniciar(jugador.x, jugador.z);
    npcs_iniciar(jugador.x, jugador.z);
    int en_carro = -1;
    int hablando = -1;
    unsigned prev = 0;
    int mirar = 0;
    int anim = 0, anim_n = -1, anim_sube = 0;     // animación de subir / bajar del carro
    float ax0 = 0, az0 = 0, ax1 = 0, az1 = 0;

    while (1) {
        SceCtrlData pad;
        sceCtrlPeekBufferPositive(0, &pad, 1);
        if (pad.buttons & SCE_CTRL_START) break;

        // --- Actualizar ---
        unsigned nuevos = pad.buttons & ~prev;
        prev = pad.buttons;
        float lx = leer_stick(pad.lx), ly = leer_stick(pad.ly);
        float rx = leer_stick(pad.rx), ry = leer_stick(pad.ry);
        // En el carro la cámara va fija detrás (como GTA): el stick derecho no la mueve
        camara_actualizar(&camara, en_carro >= 0 ? 0.0f : rx, en_carro >= 0 ? 0.0f : ry, en_carro >= 0 ? 0 : (nuevos & SCE_CTRL_LTRIGGER) != 0);
        npcs_actualizar(jugador.x, jugador.z);
        vehiculos_pasivos(en_carro >= 0 ? en_carro : (anim > 0 ? anim_n : -1));
        if (en_carro < 0) camara.sin_choque = 0;
        float fx = jugador.x, fz = jugador.z;   // lo que sigue la cámara

        if (anim > 0) {
            // Subiendo o bajando: salta hacia la puerta (o hacia afuera) sin controles
            anim--;
            int total = anim_sube ? 24 : 20;
            float k = 1.0f - (float)anim / total;
            jugador.x = ax0 + (ax1 - ax0) * k; jugador.z = az0 + (az1 - az0) * k;
            jugador.elev = sinf(3.14159265f * k) * 0.55f;
            jugador.angulo = atan2f(ax1 - ax0, az1 - az0) * 57.29578f;
            jugador.vel = 0.6f; jugador.fase += 0.3f; jugador.tiempo += 1.0f / 60.0f;
            if (anim_n >= 0) vehiculo_manejar(anim_n, 0.0f, 0.0f, 1.0f, 0);
            fx = jugador.x; fz = jugador.z;
            if (anim == 0) {
                jugador.elev = 0.0f; jugador.vel = 0.0f;
                if (anim_sube) { en_carro = anim_n; camara.alto = 12.0f; }
            }
        } else if (en_carro < 0) {
            if (hablando >= 0) {
                jugador_actualizar(&jugador, 0, 0, camara.giro, 0, 0);
                if (dialogo_actualizar((nuevos & (SCE_CTRL_CROSS | SCE_CTRL_CIRCLE)) != 0)) { npc_soltar(hablando); hablando = -1; }
            } else {
                jugador_actualizar(&jugador, lx, ly, camara.giro, (pad.buttons & SCE_CTRL_RTRIGGER) != 0,
                                   (nuevos & SCE_CTRL_CROSS) != 0);
                if (nuevos & SCE_CTRL_CIRCLE) {
                    int q = npc_cercano(jugador.x, jugador.z, 2.4f);
                    if (q >= 0) { hablando = q; npc_hablar(q, jugador.x, jugador.z); }
                }
            }
            if (hablando < 0 && (nuevos & SCE_CTRL_TRIANGLE)) {
                int n = vehiculo_cercano(jugador.x, jugador.z, 3.5f);
                if (n >= 0) {
                    float a0, v0;
                    ax0 = jugador.x; az0 = jugador.z;
                    vehiculo_pos(n, &ax1, &az1, &a0, &v0);
                    anim = 24; anim_n = n; anim_sube = 1;
                }
            }
            fx = jugador.x; fz = jugador.z;
            camara.dist_meta = 6.5f;
        } else {
            float ang, vel, vx, vz;
            // R = acelerar, L = freno / reversa, stick (arriba/abajo) hace lo mismo, cuadrado = freno de mano
            float gas = (pad.buttons & SCE_CTRL_RTRIGGER) ? 1.0f : (ly < 0.0f ? -ly : 0.0f);
            float frn = (pad.buttons & SCE_CTRL_LTRIGGER) ? 1.0f : (ly > 0.0f ? ly : 0.0f);
            vehiculo_manejar(en_carro, lx, gas, frn, (pad.buttons & SCE_CTRL_SQUARE) != 0);
            vehiculo_pos(en_carro, &fx, &fz, &ang, &vel);
            vehiculo_velocidad(en_carro, &vx, &vz);
            float rapidez = sqrtf(vx * vx + vz * vz);
            // Cámara de persecución: va detrás del carro con un poco de retraso; el stick derecho mira alrededor
            camara.hombro = 0;
            camara.sin_choque = 1;
            camara.dist_meta = 7.5f + 3.0f * (rapidez / 0.55f);
            camara_seguir(&camara, ang, 0.35f);                      // siempre detrás del carro
            camara.alto += (10.0f - camara.alto) * 0.05f;
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
                if (ok) {
                    ax0 = fx; az0 = fz; ax1 = jugador.x; az1 = jugador.z;
                    jugador.x = ax0; jugador.z = az0;
                    anim = 20; anim_n = en_carro; anim_sube = 0; en_carro = -1;
                }
            }
        }

        // --- Dibujar ---
        glViewport(0, 0, 960, 544);
        glClearColor(0.5f, 0.75f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-0.576f, 0.576f, -0.3264f, 0.3264f, 0.6f, 330.0f);

        camara.py += ((en_carro < 0 ? jugador.suelo : 0.0f) - camara.py) * 0.15f;
        camara_aplicar(&camara, fx, fz);

        cubo_activar();
        // 1) Todo lo plano (mar, arena, pasto, calles) se pinta en orden y sin profundidad:
        //    así nunca se mezclan ni se ven rayitas entre ellos.
        glEnable(GL_DEPTH_TEST);
        frontera_dibujar_suelo(fx, fz);
        suelo_dibujar(fx, fz);
        ciudad_dibujar_suelo(fx, fz);
        terreno_dibujar(fx, fz, camara.giro);   // detalle de cerca: rayas, bordillos, cebras
        // 2) Lo que tiene volumen, con profundidad.
        glEnable(GL_DEPTH_TEST);
        ciudad_dibujar(fx, fz, camara.giro);   // edificios, árboles y lugares con modelo
        frontera_dibujar(fx, fz);
        vehiculos_dibujar(fx, fz);
        npcs_dibujar(fx, fz);
        if (en_carro < 0) jugador_dibujar(&jugador);
        cubo_desactivar();

        // 3) Interfaz 2D
        ui_inicio();
        if (hablando >= 0) dialogo_dibujar();
        else if (en_carro < 0 && anim <= 0) {
            int q = npc_cercano(jugador.x, jugador.z, 2.4f);
            if (q >= 0) {
                char t[80]; snprintf(t, sizeof(t), "○ Hablar con %s", npc_nombre(q));
                int w = (int)ui_ancho(t, 1);
                ui_rect(480 - w / 2 - 12, 440, w + 24, 34, 0x000000B4);
                ui_texto(480 - w / 2, 448, 1, t, 0xFFFFFFFF);
            }
        }
        ui_fin();

        vglSwapBuffers(GL_FALSE);
    }

    sceKernelExitProcess(0);
    return 0;
}
