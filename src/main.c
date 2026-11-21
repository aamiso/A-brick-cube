// A Brick Cube Vita - Fase 3: ciudad cargada desde un mapa de texto (data/mapas/ciudad.txt)
//
// Controles:
//   Stick izquierdo = caminar
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

int main(void) {
    vglInit(0x800000);
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
    cubo_construir();

    // Lee el mapa (si no lo encuentra, usa uno chiquito incluido en el juego)
    ciudad_cargar("app0:data/mapas/ciudad.txt");

    Jugador jugador;
    Camara camara;
    jugador_iniciar(&jugador);
    ciudad_inicio(&jugador.x, &jugador.z); // empieza donde dice la P del mapa
    camara_iniciar(&camara);

    while (1) {
        SceCtrlData pad;
        sceCtrlPeekBufferPositive(0, &pad, 1);
        if (pad.buttons & SCE_CTRL_START) break;

        // --- Actualizar ---
        camara_actualizar(&camara, leer_stick(pad.rx), leer_stick(pad.ry));
        jugador_actualizar(&jugador, leer_stick(pad.lx), leer_stick(pad.ly), camara.giro);

        // --- Dibujar ---
        glViewport(0, 0, 960, 544);
        glClearColor(0.5f, 0.75f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-0.96f, 0.96f, -0.544f, 0.544f, 1.0f, 200.0f);

        camara_aplicar(&camara, jugador.x, jugador.z);

        cubo_activar();
        suelo_dibujar();
        ciudad_dibujar();
        jugador_dibujar(&jugador);
        cubo_desactivar();

        vglSwapBuffers(GL_FALSE);
    }

    sceKernelExitProcess(0);
    return 0;
}
