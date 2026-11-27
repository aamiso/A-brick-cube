#include <3ds.h>
#include <citro3d.h>
#include <math.h>
#include <stdio.h>
#include "vshader_shbin.h"

#define CLEAR_COLOR 0x82B9E8FF
#define DISPLAY_TRANSFER_FLAGS (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

static DVLB_s *vshader_dvlb;
static shaderProgram_s program;
static C3D_RenderTarget *target;
static C3D_Mtx projection;
static int u_projection, u_modelView;
static float angle_x = 0.45f;
static float angle_y = 0.55f;

static const float corners[8][3] = {
    {-1,-1,-1}, { 1,-1,-1}, { 1, 1,-1}, {-1, 1,-1},
    {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1}, {-1, 1, 1}
};
static const int faces[6][6] = {
    {4,5,6,4,6,7}, {1,0,3,1,3,2}, {0,4,7,0,7,3},
    {5,1,2,5,2,6}, {3,7,6,3,6,2}, {0,1,5,0,5,4}
};
static const float colors[6][4] = {
    {0.86f,0.20f,0.16f,1.0f}, {0.60f,0.12f,0.10f,1.0f},
    {0.20f,0.62f,0.24f,1.0f}, {0.16f,0.48f,0.18f,1.0f},
    {0.28f,0.72f,0.30f,1.0f}, {0.12f,0.38f,0.16f,1.0f}
};

static int scene_init(void)
{
    vshader_dvlb = DVLB_ParseFile((u32 *)vshader_shbin, vshader_shbin_size);
    if (!vshader_dvlb) return 0;

    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
    C3D_BindProgram(&program);

    u_projection = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
    u_modelView = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");

    C3D_AttrInfo *attrs = C3D_GetAttrInfo();
    AttrInfo_Init(attrs);
    AttrInfo_AddLoader(attrs, 0, GPU_FLOAT, 3);
    AttrInfo_AddLoader(attrs, 1, GPU_FLOAT, 4);

    Mtx_PerspTilt(&projection, C3D_AngleFromDegrees(55.0f), C3D_AspectRatioTop, 0.1f, 100.0f, true);

    C3D_TexEnv *env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_PRIMARY_COLOR, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);

    C3D_DepthMap(true, -1.0f, 0.0f);
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    C3D_CullFace(GPU_CULL_NONE);
    return 1;
}

static void draw_cube(void)
{
    C3D_Mtx model;
    Mtx_Identity(&model);
    Mtx_RotateX(&model, angle_x, true);
    Mtx_RotateY(&model, angle_y, true);
    Mtx_Translate(&model, 0.0f, 0.0f, 4.5f, true);

    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, u_projection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, u_modelView, &model);

    C3D_ImmDrawBegin(GPU_TRIANGLES);
    for (int f = 0; f < 6; ++f) {
        for (int v = 0; v < 6; ++v) {
            const float *p = corners[faces[f][v]];
            C3D_ImmSendAttrib(p[0], p[1], p[2], 1.0f);
            C3D_ImmSendAttrib(colors[f][0], colors[f][1], colors[f][2], colors[f][3]);
        }
    }
    C3D_ImmDrawEnd();
}

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);

    if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) {
        printf("No se pudo iniciar Citro3D.\n");
        gfxExit();
        return 1;
    }

    target = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    if (!target) {
        printf("No se pudo crear el render target.\n");
        C3D_Fini(); gfxExit();
        return 1;
    }
    C3D_RenderTargetSetOutput(target, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

    if (!scene_init()) {
        printf("No se pudo cargar el shader PICA200.\n");
        C3D_RenderTargetDelete(target);
        C3D_Fini(); gfxExit();
        return 1;
    }

    printf("A BRICK CUBE - PORT 3DS\n");
    printf("Prototipo de renderizado 3D\n");
    printf("Cruceta: girar el cubo\n");
    printf("START: salir\n");

    while (aptMainLoop()) {
        hidScanInput();
        u32 down = hidKeysDown();
        if (down & KEY_START) break;
        if (down & KEY_LEFT) angle_y -= 0.12f;
        if (down & KEY_RIGHT) angle_y += 0.12f;
        if (down & KEY_UP) angle_x -= 0.12f;
        if (down & KEY_DOWN) angle_x += 0.12f;

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C3D_RenderTargetClear(target, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
        C3D_FrameDrawOn(target);
        draw_cube();
        C3D_FrameEnd(0);
    }

    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
    C3D_RenderTargetDelete(target);
    C3D_Fini();
    gfxExit();
    return 0;
}
