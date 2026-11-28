// A Brick Cube Vita - Fase 5: ciudad voxel con casas, banco y café (modelos), calles con detalle y personaje animado (data/mapas/ciudad.txt)
//
// Controles:
//   Circle Pad = caminar / conducir
//   R = correr o acelerar; A = saltar; B = hablar/aceptar; X = entrar/salir del carro
//   L = alternar cámara; cruceta = girar e inclinar la cámara; Y = freno de mano
//   START = salir

#include <3ds.h>
#include <citro3d.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "vitaGL.h"
#include "vshader_shbin.h"
#define SCE_CTRL_START (1u<<0)
#define SCE_CTRL_CROSS (1u<<1)
#define SCE_CTRL_CIRCLE (1u<<2)
#define SCE_CTRL_TRIANGLE (1u<<3)
#define SCE_CTRL_SQUARE (1u<<4)
#define SCE_CTRL_RTRIGGER (1u<<5)
#define SCE_CTRL_LTRIGGER (1u<<6)
typedef struct { unsigned buttons; unsigned char lx,ly; } SceCtrlData;
static DVLB_s *brick_shader;
static shaderProgram_s brick_program;
static C3D_RenderTarget *brick_target, *brick_bottom, *brick_current;
static int brick_u_mvp;
extern void brick3ds_compat_init(void);
extern void brick3ds_compat_frame_begin(void);
extern void brick3ds_compat_exit(void);
/* Lee el Circle Pad con zona muerta radial. El Circle Pad entrega aprox.
   -156..156; normalizamos el vector completo para evitar diagonales desiguales
   y quitamos la zona muerta una sola vez (sin el segundo filtro de leer_stick). */
static void brick3ds_read_circle(const circlePosition *cp, float *x, float *y) {
    const float max_axis = 156.0f;
    const float dead_zone = 18.0f;
    float dx = (float)cp->dx;
    float dy = (float)cp->dy;
    float mag = sqrtf(dx * dx + dy * dy);
    if (mag <= dead_zone) { *x = 0.0f; *y = 0.0f; return; }
    float limited = mag > max_axis ? max_axis : mag;
    float strength = (limited - dead_zone) / (max_axis - dead_zone);
    if (strength > 1.0f) strength = 1.0f;
    *x = (dx / mag) * strength;
    *y = -(dy / mag) * strength; /* arriba = valor negativo, igual que leer_stick() */
}

static const u32 brick_transfer_flags =
 GX_TRANSFER_FLIP_VERT(0)|GX_TRANSFER_OUT_TILED(0)|GX_TRANSFER_RAW_COPY(0)|
 GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8)|GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8)|
 GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO);
void brick3ds_set_mvp(const C3D_Mtx *m){C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER,brick_u_mvp,m);}
void brick3ds_frame_begin(void){C3D_FrameBegin(C3D_FRAME_SYNCDRAW);brick3ds_compat_frame_begin();C3D_FrameDrawOn(brick_target);brick_current=brick_target;}
static void brick3ds_draw_bottom(void){C3D_FrameDrawOn(brick_bottom);brick_current=brick_bottom;}
extern void brick3ds_flush(void);
void brick3ds_present(void){brick3ds_flush();C3D_FrameEnd(0);}
void brick3ds_clear(float r,float g,float b,float a,unsigned mask){u32 rgba=((u32)(u8)(r*255)<<24)|((u32)(u8)(g*255)<<16)|((u32)(u8)(b*255)<<8)|(u32)(u8)(a*255);u32 f=0;if(mask&GL_COLOR_BUFFER_BIT)f|=C3D_CLEAR_COLOR;if(mask&GL_DEPTH_BUFFER_BIT)f|=C3D_CLEAR_DEPTH;C3D_RenderTargetClear(brick_current?brick_current:brick_target,f,rgba,0);}
static int brick_renderer_init(void){
 if(!C3D_Init(0x100000))return 0;   /* 1 MB: la ciudad hace muchos dibujos por cuadro */
 brick_target=C3D_RenderTargetCreate(240,400,GPU_RB_RGBA8,GPU_RB_DEPTH24_STENCIL8);if(!brick_target)return 0;
 C3D_RenderTargetSetOutput(brick_target,GFX_TOP,GFX_LEFT,brick_transfer_flags);
 /* Pantalla de abajo (320x240): hora, dinero, minimapa y textos */
 brick_bottom=C3D_RenderTargetCreate(240,320,GPU_RB_RGBA8,GPU_RB_DEPTH16);if(!brick_bottom)return 0;
 C3D_RenderTargetSetOutput(brick_bottom,GFX_BOTTOM,GFX_LEFT,brick_transfer_flags);
  brick_shader=DVLB_ParseFile((u32*)vshader_shbin,vshader_shbin_size);if(!brick_shader)return 0;
 shaderProgramInit(&brick_program);shaderProgramSetVsh(&brick_program,&brick_shader->DVLE[0]);C3D_BindProgram(&brick_program);
 brick_u_mvp=shaderInstanceGetUniformLocation(brick_program.vertexShader,"mvp");
 brick3ds_compat_init();
 C3D_TexEnv *e=C3D_GetTexEnv(0);C3D_TexEnvInit(e);C3D_TexEnvSrc(e,C3D_Both,GPU_PRIMARY_COLOR,0,0);C3D_TexEnvFunc(e,C3D_Both,GPU_REPLACE);
 C3D_DepthMap(true,-1.0f,0.0f);C3D_DepthTest(true,GPU_GEQUAL,GPU_WRITE_ALL);C3D_CullFace(GPU_CULL_NONE);return 1;
}
static void brick_renderer_exit(void){brick3ds_compat_exit();if(brick_bottom)C3D_RenderTargetDelete(brick_bottom);if(brick_target)C3D_RenderTargetDelete(brick_target);shaderProgramFree(&brick_program);if(brick_shader)DVLB_Free(brick_shader);C3D_Fini();}

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
#include "world/escombros.h"
#include "world/reloj.h"
#include "world/misiones.h"
#include "ui/minimapa.h"
#include "world/interior.h"
#include "world/lugares.h"
#include "ui/ui.h"
#include "ui/dialogo.h"
#include "abajo.h"
#include "guardado.h"
#include <math.h>
#include <stdio.h>

/* Dibuja la pantalla de abajo (después de la de arriba, dentro del mismo cuadro). */
static void dibujar_abajo(const EstadoAbajo *e) {
    brick3ds_draw_bottom();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ui_inicio_area(ABAJO_W, ABAJO_H);
    abajo_dibujar(e);
    ui_fin();
}

/* Error antes de poder dibujar: lo muestra en la pantalla de abajo y espera START. */
static int error_fatal(const char *msg) {
    consoleInit(GFX_BOTTOM, NULL);
    printf("%s\n\nPulsa START para salir.\n", msg);
    while (aptMainLoop()) { hidScanInput(); if (hidKeysDown() & KEY_START) break; gspWaitForVBlank(); }
    return 1;
}

int main(void) {
    osSetSpeedupEnable(true);   // New 3DS: CPU a 804 MHz (en Old 3DS no hace nada)
    gfxInitDefault();
    if(R_FAILED(romfsInit())) { error_fatal("No se pudo montar ROMFS."); gfxExit(); return 1; }
    if(!brick_renderer_init()) { error_fatal("No se pudo iniciar Citro3D/shader."); romfsExit(); gfxExit(); return 1; }
    cubo_construir();

    // Lee el mapa (si no lo encuentra, usa uno chiquito incluido en el juego)
    int mapa_original = ciudad_cargar("romfs:/mapas/ciudad.txt");
    (void)mapa_original;

    Jugador jugador;
    Camara camara;
    jugador_iniciar(&jugador);
    ciudad_inicio(&jugador.x, &jugador.z); // empieza donde dice la P del mapa

    // --- Partida guardada (sdmc:/3ds/A-brick-cube/partida.dat) ---
    const char *guardar_msg = NULL; int guardar_t = 0, guardar_ok = 0;
    {
        Partida pg;
        if (guardado_leer(&pg)) {
            if (!ciudad_choca(pg.x, pg.z, 0.45f)) {
                jugador.x = pg.x; jugador.z = pg.z; jugador.angulo = pg.angulo;
                jugador.suelo = jugador.elev = pg.suelo;
            }
            reloj_poner_hora(pg.hora);
            mision_poner_dinero(pg.dinero);
            guardar_msg = "Partida cargada"; guardar_ok = 1; guardar_t = 180;
        }
    }
    ciudad_precalentar(jugador.x, jugador.z);
    camara_iniciar(&camara);
    camara.hombro = 0;
    camara.giro = 180.0f - jugador.angulo; // Cámara centrada detrás del personaje, como en Ocarina of Time 3D.
    camara.alto = 8.0f;
    vehiculos_iniciar(jugador.x, jugador.z);
    npcs_iniciar(jugador.x, jugador.z);
    int en_carro = -1;
    int hablando = -1;
    int oferta = 0;                        // 1 mientras un NPC ofrece una misión
    char dentro = 0;                       // letra del lugar donde estamos (0 = en la calle)
    int hablando_i = -1, fase_f = 0, fund = 0, pend_salir = 0, pend_puerta = 0;
    char pend_l = 0;
    float gx = 0, gz = 0, gsuelo = 0;      // dónde estaba en la calle
    Camara cam_guardada;
    unsigned prev = 0;
    int anim = 0, anim_n = -1, anim_sube = 0;     // animación de subir / bajar del carro
    float ax0 = 0, az0 = 0, ax1 = 0, az1 = 0;

    while (aptMainLoop()) {
        hidScanInput(); u32 down=hidKeysDown(), held=hidKeysHeld();
        if(down&KEY_START)break;
        touchPosition tp; hidTouchRead(&tp);
        int toca_boton = (held & KEY_TOUCH) && tp.px >= BOTON_X && tp.px < BOTON_X + BOTON_W && tp.py >= BOTON_Y && tp.py < BOTON_Y + BOTON_H;
        int pide_guardar = (down & KEY_SELECT) || ((down & KEY_TOUCH) && toca_boton);
        if (guardar_t > 0 && --guardar_t == 0) guardar_msg = NULL;
        circlePosition cp;hidCircleRead(&cp);
        SceCtrlData pad;pad.buttons=0;
        if(held&KEY_A)pad.buttons|=SCE_CTRL_CROSS;
        if(held&KEY_B)pad.buttons|=SCE_CTRL_CIRCLE;
        if(held&KEY_X)pad.buttons|=SCE_CTRL_TRIANGLE;
        if(held&KEY_Y)pad.buttons|=SCE_CTRL_SQUARE;
        if(held&KEY_R)pad.buttons|=SCE_CTRL_RTRIGGER;
        if(held&KEY_L)pad.buttons|=SCE_CTRL_LTRIGGER;
        float lx = 0.0f, ly = 0.0f;
        brick3ds_read_circle(&cp, &lx, &ly);
        float rx=((held&KEY_DRIGHT)?1.0f:0.0f)-((held&KEY_DLEFT)?1.0f:0.0f);
        float ry=((held&KEY_DUP)?1.0f:0.0f)-((held&KEY_DDOWN)?1.0f:0.0f);
        brick3ds_frame_begin();

        // --- Actualizar ---
        unsigned nuevos = pad.buttons & ~prev;
        prev = pad.buttons;
        /* Cruceta: control de cámara en Old 3DS, sin stick derecho. */
        // --- Transición (fundido a negro) ---
        if (fase_f == 1) {
            if (++fund >= 14) {
                fund = 14; fase_f = -1;
                if (pend_salir) {
                    interior_salir(); dentro = 0; pend_salir = 0; hablando_i = -1;
                    jugador.x = gx; jugador.z = gz; jugador.suelo = jugador.elev = gsuelo; jugador.vel = 0; camara = cam_guardada;
                } else if (pend_puerta) {
                    interior_cambiar(&jugador); pend_puerta = 0; hablando_i = -1;
                } else {
                    gx = jugador.x; gz = jugador.z; gsuelo = jugador.suelo; cam_guardada = camara;
                    interior_entrar(pend_l, &jugador); dentro = pend_l; hablando = -1;
                }
            }
        } else if (fase_f == -1) { if (--fund <= 0) { fund = 0; fase_f = 0; } }
        if (fase_f) { lx = ly = 0.0f; nuevos &= ~(SCE_CTRL_CROSS | SCE_CTRL_CIRCLE | SCE_CTRL_TRIANGLE); }

        // --- Dentro de un lugar: cámara fija desde arriba, como Pokémon ---
        if (dentro) {
            if (hablando_i >= 0) {
                interior_actualizar(&jugador, 0, 0, 0);
                if (dialogo_actualizar((nuevos & (SCE_CTRL_CROSS | SCE_CTRL_CIRCLE)) != 0)) hablando_i = -1;
            } else {
                { int rv = interior_actualizar(&jugador, lx, ly, (pad.buttons & SCE_CTRL_RTRIGGER) != 0);
                  if (rv == 1 && fase_f == 0) { pend_salir = 1; fase_f = 1; }
                  else if (rv == 2 && fase_f == 0) { pend_puerta = 1; fase_f = 1; } }
                if (nuevos & SCE_CTRL_CIRCLE) {
                    int q = interior_npc_cercano(&jugador, 1.7f);
                    if (q >= 0) { hablando_i = q; interior_hablar(q, &jugador); }
                }
            }
            camara.hombro = 0; camara.desvio = 0; camara.sin_choque = 1; camara.giro = 0.0f; camara.alto = 52.0f;
            camara.distancia = camara.real = camara.dist_meta = 11.0f; camara.py = 0.25f;
            glViewport(0, 0, 960, 544);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glFrustum(-0.544f, 0.544f, -0.3264f, 0.3264f, 0.6f, 330.0f);
            camara_aplicar(&camara, jugador.x, jugador.z);
            cubo_activar();
            glEnable(GL_DEPTH_TEST);
            interior_dibujar(jugador.tiempo);
            jugador_dibujar(&jugador);
            cubo_desactivar();
            ui_inicio();
            if (fund > 0) ui_rect(0, 0, 960, 544, (unsigned)(fund * 255 / 14));
            ui_fin();
            if (pide_guardar) { guardar_msg = "Sal a la calle para guardar."; guardar_ok = 0; guardar_t = 150; }
            {
                static char aviso_i[80];
                EstadoAbajo ea = {0};
                ea.px = jugador.x; ea.pz = jugador.z; ea.ang = jugador.angulo;
                ea.lugar = interior_titulo();
                if (hablando_i < 0) {
                    int q = interior_npc_cercano(&jugador, 1.7f);
                    if (q >= 0) { snprintf(aviso_i, sizeof(aviso_i), "B: Hablar con %s", interior_npc_nombre(q)); ea.aviso = aviso_i; }
                }
                ea.guardado_msg = guardar_msg; ea.guardado_ok = guardar_ok; ea.boton_apretado = toca_boton;
                dibujar_abajo(&ea);
            }
            brick3ds_present();
            continue;
        }

        // En el carro la cámara va fija detrás (como GTA): el stick derecho no la mueve
        camara_actualizar(&camara, en_carro >= 0 ? 0.0f : rx, en_carro >= 0 ? 0.0f : ry, en_carro >= 0 ? 0 : (nuevos & SCE_CTRL_LTRIGGER) != 0);
        npcs_actualizar(jugador.x, jugador.z);
        escombros_actualizar();
        reloj_actualizar();
        mision_actualizar(en_carro >= 0 ? jugador.x : jugador.x, jugador.z, en_carro >= 0);
        vehiculos_ia(jugador.x, jugador.z, en_carro);
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
            if (anim_n >= 0) vehiculo_manejar(anim_n, 0.0f, 0.0f, 0.0f, 1);   // freno de mano (antes se iba en reversa)
            fx = jugador.x; fz = jugador.z;
            if (anim == 0) {
                jugador.elev = 0.0f; jugador.vel = 0.0f;
                if (anim_sube) { en_carro = anim_n; camara.alto = 12.0f; }
            }
        } else if (en_carro < 0) {
            if (hablando >= 0) {
                jugador_actualizar(&jugador, 0, 0, camara.giro, 0, 0);
                if (dialogo_actualizar((nuevos & (SCE_CTRL_CROSS | SCE_CTRL_CIRCLE)) != 0)) {
                    if (npc_da_mision(hablando) && !mision_activa() && mision_preparar(jugador.x, jugador.z)) oferta = 1;
                    npc_soltar(hablando); hablando = -1;
                }
            } else if (oferta) {
                jugador_actualizar(&jugador, 0, 0, camara.giro, 0, 0);
                if (nuevos & SCE_CTRL_CIRCLE) { mision_aceptar(); oferta = 0; }
                else if (nuevos & SCE_CTRL_CROSS) oferta = 0;
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
                    anim = 24; anim_n = n; anim_sube = 1; vehiculo_tomar(n);
                } else {
                    char L = lugar_entrada_cerca(jugador.x, jugador.z, 2.5f);
                    if (L && interior_hay(L) && fase_f == 0) { pend_l = L; pend_salir = 0; fase_f = 1; }
                }
            }
            fx = jugador.x; fz = jugador.z;
            if (anim <= 0 && (fabsf(lx) + fabsf(ly) > 0.05f))
                camara_seguir(&camara, 180.0f - jugador.angulo, 0.12f);
            camara.dist_meta = 6.5f;
        } else {
            float ang, vel, vx, vz;
            // R = acelerar, L = freno / reversa, stick (arriba/abajo) hace lo mismo, cuadrado = freno de mano
            float gas = (pad.buttons & SCE_CTRL_RTRIGGER) ? 1.0f : (ly < 0.0f ? -ly : 0.0f);
            float frn = (pad.buttons & SCE_CTRL_LTRIGGER) ? 1.0f : (ly > 0.0f ? ly : 0.0f);
            vehiculo_manejar(en_carro, -lx, gas, frn, (pad.buttons & SCE_CTRL_SQUARE) != 0);
            vehiculo_pos(en_carro, &fx, &fz, &ang, &vel);
            vehiculo_velocidad(en_carro, &vx, &vz);
            // Cámara de persecución: va detrás del carro con un poco de retraso; el stick derecho mira alrededor
            camara.hombro = 0;
            camara.sin_choque = 1;
            camara.dist_meta = 8.0f;
            camara.giro = 180.0f - ang;                            // justo detrás del carro (la cámara se coloca opuesta a giro)
            camara.alto = 10.0f;
            jugador.x = fx; jugador.z = fz; jugador.vel = 0.0f;
            if (nuevos & SCE_CTRL_TRIANGLE) {
                // se baja por el lado derecho (o el primer lado libre)
                float r = ang * 3.14159265f / 180.0f;
                int ok = 0;
                static const float RADIOS[4] = { 1.9f, 2.6f, 3.4f, 4.6f };
                for (int rr = 0; rr < 4 && !ok; rr++)                 // prueba derecha, izquierda, atrás, adelante y las diagonales, cada vez más lejos
                    for (int k = 0; k < 8 && !ok; k++) {
                        static const float GIROS[8] = { 1.5708f, -1.5708f, 3.1416f, 0.0f, 0.7854f, -0.7854f, 2.3562f, -2.3562f };
                        float a2 = r + GIROS[k];
                        float ex = fx + sinf(a2) * RADIOS[rr], ez = fz + cosf(a2) * RADIOS[rr];
                        if (!ciudad_choca(ex, ez, 0.45f) && !vehiculos_choca(ex, ez, 0.45f, -1)) { jugador.x = ex; jugador.z = ez; ok = 1; }
                    }
                if (!ok) { jugador.x = fx + sinf(r + 1.5708f) * 1.9f; jugador.z = fz + cosf(r + 1.5708f) * 1.9f; ok = 1; }   // último recurso: siempre se puede bajar
                if (ok) {
                    ax0 = fx; az0 = fz; ax1 = jugador.x; az1 = jugador.z;
                    jugador.x = ax0; jugador.z = az0;
                    anim = 20; anim_n = en_carro; anim_sube = 0; en_carro = -1;
                }
            }
        }

        // --- Dibujar ---
        glViewport(0, 0, 960, 544);
        { float sr, sg, sb; reloj_cielo(&sr, &sg, &sb); glClearColor(sr, sg, sb, 1.0f); }
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
        escombros_dibujar(fx, fz);
        mision_dibujar_3d(fx, fz);
        if (en_carro < 0) jugador_dibujar(&jugador);
        else { float cx2, cz2, a2, v2; vehiculo_pos(en_carro, &cx2, &cz2, &a2, &v2); float rl, pt, al; vehiculo_inclinacion(en_carro, &rl, &pt, &al); jugador_dibujar_en_carro(&jugador, cx2, cz2, a2, rl, pt, al); }
        cubo_desactivar();

        // 3) Arriba: solo el filtro de noche y el fundido
        ui_inicio();
        reloj_filtro();                                       // noche / atardecer
        if (fund > 0) ui_rect(0, 0, 960, 544, (unsigned)(fund * 255 / 14));
        ui_fin();

        // 4) Guardar partida (solo a pie, en la calle y sin conversación)
        if (pide_guardar) {
            if (en_carro >= 0 || anim > 0) { guardar_msg = "Bájate del carro para guardar."; guardar_ok = 0; }
            else if (hablando >= 0 || oferta || fase_f) { guardar_msg = "Termina de hablar primero."; guardar_ok = 0; }
            else {
                Partida pg;
                pg.x = jugador.x; pg.z = jugador.z; pg.angulo = jugador.angulo; pg.suelo = jugador.suelo;
                pg.hora = reloj_hora(); pg.dinero = mision_dinero();
                guardar_ok = guardado_escribir(&pg);
                guardar_msg = guardar_ok ? "¡Partida guardada!" : "No se pudo guardar. ¿Hay tarjeta SD?";
            }
            guardar_t = 150;
        }

        // 5) Pantalla de abajo: hora, dinero, minimapa, misión y textos
        {
            static char aviso_c[80];
            EstadoAbajo ea = {0};
            ea.px = fx; ea.pz = fz; ea.ang = en_carro >= 0 ? 0.0f : jugador.angulo;
            ea.en_carro = en_carro >= 0;
            if (oferta) ea.oferta = mision_oferta();
            if (hablando < 0 && !oferta && en_carro < 0 && anim <= 0) {
                int q = npc_cercano(jugador.x, jugador.z, 2.4f);
                if (q >= 0) { snprintf(aviso_c, sizeof(aviso_c), "B: Hablar con %s", npc_nombre(q)); ea.aviso = aviso_c; }
                else if (vehiculo_cercano(jugador.x, jugador.z, 3.5f) >= 0) ea.aviso = "X: Subir al carro";
                else {
                    char L = lugar_entrada_cerca(jugador.x, jugador.z, 2.5f);
                    if (L && interior_hay(L)) { snprintf(aviso_c, sizeof(aviso_c), "X: Entrar a %s", interior_nombre(L)); ea.aviso = aviso_c; }
                }
            }
            ea.guardado_msg = guardar_msg; ea.guardado_ok = guardar_ok; ea.boton_apretado = toca_boton;
            dibujar_abajo(&ea);
        }

        brick3ds_present();
    }

    brick_renderer_exit(); romfsExit(); gfxExit(); return 0;
}
