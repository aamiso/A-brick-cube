#include <3ds.h>
#include <citro3d.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vshader_shbin.h"

#define CLEAR_COLOR 0x82B9E8FF
#define DISPLAY_TRANSFER_FLAGS (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))
#define MAX_LINE 256
#define MAX_FILE 600000
#define MODEL_SCALE 0.16f

typedef struct {
    int w, d, h;
    unsigned char *voxels;
    unsigned int palette[256];
    unsigned char has_color[256];
} VoxelModel;

static DVLB_s *vshader_dvlb;
static shaderProgram_s program;
static C3D_RenderTarget *target;
static C3D_Mtx projection;
static int u_projection, u_modelView;
static float angle_x = 0.25f, angle_y = 0.45f;
static VoxelModel house;

static const int face_neighbor[6][3] = {
    {0,1,0}, {0,0,1}, {0,0,-1}, {1,0,0}, {-1,0,0}, {0,-1,0}
};
static const float face_brightness[6] = {1.0f, 0.90f, 0.68f, 0.78f, 0.84f, 0.60f};
static const float face_corner[6][4][3] = {
    {{0,1,0},{1,1,0},{1,1,1},{0,1,1}},
    {{0,0,1},{1,0,1},{1,1,1},{0,1,1}},
    {{0,0,0},{1,0,0},{1,1,0},{0,1,0}},
    {{1,0,0},{1,0,1},{1,1,1},{1,1,0}},
    {{0,0,0},{0,0,1},{0,1,1},{0,1,0}},
    {{0,0,0},{0,0,1},{1,0,1},{1,0,0}}
};

static unsigned char *voxel_at(const VoxelModel *m, int x, int y, int z)
{
    if (x < 0 || y < 0 || z < 0 || x >= m->w || y >= m->h || z >= m->d)
        return NULL;
    return &m->voxels[((size_t)y * m->d + z) * m->w + x];
}

static int hex_digit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int load_voxel_model(const char *path, VoxelModel *m)
{
    FILE *file = fopen(path, "rb");
    if (!file) return 0;
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (size <= 0 || size > MAX_FILE) { fclose(file); return 0; }

    char *buffer = (char *)malloc((size_t)size + 1);
    if (!buffer) { fclose(file); return 0; }
    size_t read_size = fread(buffer, 1, (size_t)size, file);
    fclose(file);
    buffer[read_size] = '\0';

    int w = 0, d = 0, h = 0, row = 0, in_layer = 0;
    char *dimension_buffer = strdup(buffer);
    if (!dimension_buffer) { free(buffer); return 0; }
    char *save = NULL;
    for (char *line = strtok_r(dimension_buffer, "\n", &save); line; line = strtok_r(NULL, "\n", &save)) {
        size_t len = strlen(line);
        while (len && (line[len-1] == '\r' || line[len-1] == ' ' || line[len-1] == '\t')) line[--len] = 0;
        char *p = line;
        while (*p == ' ' || *p == '\t') ++p;
        if (!*p || (p[0] == '/' && p[1] == '/')) continue;
        if (!strncmp(p, "capa", 4)) { ++h; row = 0; in_layer = 1; continue; }
        if (in_layer) {
            int n = (int)strlen(p);
            if (n > w) w = n;
            if (++row > d) d = row;
        }
    }

    free(dimension_buffer);
    if (w <= 0 || d <= 0 || h <= 0 || w > 128 || d > 128 || h > 200) {
        free(buffer); return 0;
    }
    memset(m, 0, sizeof(*m));
    m->w = w; m->d = d; m->h = h;
    m->voxels = (unsigned char *)calloc((size_t)w * d * h, 1);
    if (!m->voxels) { free(buffer); return 0; }

    save = NULL; h = 0; row = 0; in_layer = 0;
    for (char *line = strtok_r(buffer, "\n", &save); line; line = strtok_r(NULL, "\n", &save)) {
        size_t len = strlen(line);
        while (len && (line[len-1] == '\r' || line[len-1] == ' ' || line[len-1] == '\t')) line[--len] = 0;
        char *p = line;
        while (*p == ' ' || *p == '\t') ++p;
        if (!*p || (p[0] == '/' && p[1] == '/')) continue;
        if (!strncmp(p, "color", 5) && (p[5] == ' ' || p[5] == '\t')) {
            char letter = 0, hex[16] = {0};
            if (sscanf(p + 5, " %c %15s", &letter, hex) == 2) {
                const char *q = hex[0] == '#' ? hex + 1 : hex;
                unsigned int rgb = 0; int ok = 1;
                for (int k = 0; k < 6; ++k) {
                    int v = hex_digit(q[k]);
                    if (v < 0) { ok = 0; break; }
                    rgb = (rgb << 4) | (unsigned int)v;
                }
                if (ok) { m->palette[(unsigned char)letter] = rgb; m->has_color[(unsigned char)letter] = 1; }
            }
            continue;
        }
        if (!strncmp(p, "capa", 4)) { ++h; row = 0; in_layer = 1; continue; }
        if (!in_layer || h > m->h || row >= m->d) continue;
        for (int x = 0; p[x] && x < m->w; ++x) {
            if (p[x] != '.' && p[x] != ' ')
                *voxel_at(m, x, h - 1, row) = (unsigned char)p[x];
        }
        ++row;
    }
    free(buffer);
    return 1;
}

static void send_vertex(float x, float y, float z, float r, float g, float b)
{
    C3D_ImmSendAttrib(x, y, z, 1.0f);
    C3D_ImmSendAttrib(r, g, b, 1.0f);
}

static void draw_voxel_model(const VoxelModel *m)
{
    C3D_Mtx model;
    Mtx_Identity(&model);
    Mtx_RotateX(&model, angle_x, true);
    Mtx_RotateY(&model, angle_y, true);
    Mtx_Translate(&model, 0.0f, -0.95f, 6.4f, true);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, u_projection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, u_modelView, &model);

    C3D_ImmDrawBegin(GPU_TRIANGLES);
    for (int y = 0; y < m->h; ++y) {
        for (int z = 0; z < m->d; ++z) {
            for (int x = 0; x < m->w; ++x) {
                unsigned char letter = *voxel_at(m, x, y, z);
                if (!letter) continue;
                unsigned int rgb = m->has_color[letter] ? m->palette[letter] : 0xff00ff;
                float cr = ((rgb >> 16) & 255) / 255.0f;
                float cg = ((rgb >> 8) & 255) / 255.0f;
                float cb = (rgb & 255) / 255.0f;

                for (int f = 0; f < 6; ++f) {
                    unsigned char *neighbor = voxel_at(m, x + face_neighbor[f][0],
                        y + face_neighbor[f][1], z + face_neighbor[f][2]);
                    if (neighbor && *neighbor) continue;
                    float shade = face_brightness[f];
                    const int order[6] = {0,1,2,0,2,3};
                    for (int v = 0; v < 6; ++v) {
                        const float *c = face_corner[f][order[v]];
                        float px = (x + c[0] - m->w * 0.5f) * MODEL_SCALE;
                        float py = (y + c[1]) * MODEL_SCALE - 0.75f;
                        float pz = (z + c[2] - m->d * 0.5f) * MODEL_SCALE;
                        send_vertex(px, py, pz, cr * shade, cg * shade, cb * shade);
                    }
                }
            }
        }
    }
    C3D_ImmDrawEnd();
}

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

int main(int argc, char **argv)
{
    (void)argc; (void)argv;
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);
    if (R_FAILED(romfsInit())) {
        printf("No se pudo montar ROMFS.\n");
        gfxExit(); return 1;
    }
    if (!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) {
        printf("No se pudo iniciar Citro3D.\n");
        romfsExit(); gfxExit(); return 1;
    }

    target = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    if (!target) {
        printf("No se pudo crear el render target.\n");
        C3D_Fini(); romfsExit(); gfxExit(); return 1;
    }
    C3D_RenderTargetSetOutput(target, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    if (!scene_init()) {
        printf("No se pudo cargar el shader PICA200.\n");
        C3D_RenderTargetDelete(target); C3D_Fini(); romfsExit(); gfxExit(); return 1;
    }
    if (!load_voxel_model("romfs:/modelos/casa_roja.txt", &house)) {
        printf("No se pudo leer casa_roja.txt desde ROMFS.\n");
        printf("Revisa que data/modelos/casa_roja.txt exista.\n");
        shaderProgramFree(&program); DVLB_Free(vshader_dvlb);
        C3D_RenderTargetDelete(target); C3D_Fini(); romfsExit(); gfxExit(); return 1;
    }

    printf("A BRICK CUBE - PORT 3DS\n");
    printf("Primer modelo voxel original\n");
    printf("Casa roja: data/modelos/casa_roja.txt\n");
    printf("Cruceta: girar modelo\n");
    printf("START: salir\n");

    while (aptMainLoop()) {
        hidScanInput();
        u32 down = hidKeysDown();
        if (down & KEY_START) break;
        if (down & KEY_LEFT) angle_y -= 0.10f;
        if (down & KEY_RIGHT) angle_y += 0.10f;
        if (down & KEY_UP) angle_x -= 0.10f;
        if (down & KEY_DOWN) angle_x += 0.10f;

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C3D_RenderTargetClear(target, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
        C3D_FrameDrawOn(target);
        draw_voxel_model(&house);
        C3D_FrameEnd(0);
    }

    free(house.voxels);
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
    C3D_RenderTargetDelete(target);
    C3D_Fini();
    romfsExit();
    gfxExit();
    return 0;
}
