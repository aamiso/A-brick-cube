#include <3ds.h>
#include <citro3d.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vshader_shbin.h"

#define CLEAR_COLOR 0x82B9E8FF
#define DISPLAY_TRANSFER_FLAGS (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))
#define MAP_MAX 400
#define MAP_RADIUS 19
#define CELL_SCALE 0.16f

static DVLB_s *vshader_dvlb;
static shaderProgram_s program;
static C3D_RenderTarget *target;
static C3D_Mtx projection;
static int u_projection, u_modelView;
static char map_grid[MAP_MAX][MAP_MAX];
static int map_width, map_height, player_x, player_y;
static float angle_x = 0.82f, angle_y = 0.35f;

static const float cube_faces[6][4][3] = {
    {{0,1,0},{1,1,0},{1,1,1},{0,1,1}},
    {{0,0,1},{1,0,1},{1,1,1},{0,1,1}},
    {{0,0,0},{1,0,0},{1,1,0},{0,1,0}},
    {{1,0,0},{1,0,1},{1,1,1},{1,1,0}},
    {{0,0,0},{0,0,1},{0,1,1},{0,1,0}},
    {{0,0,0},{0,0,1},{1,0,1},{1,0,0}}
};
static const int neighbor[6][2] = {{0,-1},{0,1},{0,-1},{1,0},{-1,0},{0,1}};
static const float shade_face[6] = {1.0f,0.88f,0.68f,0.78f,0.84f,0.60f};

static char map_at(int x, int y)
{
    if (x < 0 || y < 0 || x >= map_width || y >= map_height) return ' ';
    return map_grid[y][x];
}

static int load_city_map(void)
{
    FILE *f = fopen("romfs:/mapas/ciudad.txt", "rb");
    if (!f) return 0;
    char line[MAP_MAX + 8];
    map_width = map_height = 0;
    player_x = player_y = -1;
    memset(map_grid, ' ', sizeof(map_grid));
    while (fgets(line, sizeof(line), f) && map_height < MAP_MAX) {
        if (line[0] == '#') continue;
        size_t n = strcspn(line, "\r\n");
        if (n == 0) continue;
        if (n > MAP_MAX) n = MAP_MAX;
        if ((int)n > map_width) map_width = (int)n;
        for (size_t x = 0; x < n; ++x) {
            map_grid[map_height][x] = line[x];
            if (line[x] == 'P') { player_x = (int)x; player_y = map_height; }
        }
        ++map_height;
    }
    fclose(f);
    return map_width > 0 && map_height > 0 && player_x >= 0 && player_y >= 0;
}

static void vertex(float x, float y, float z, float r, float g, float b)
{
    C3D_ImmSendAttrib(x, y, z, 1.0f);
    C3D_ImmSendAttrib(r, g, b, 1.0f);
}

static void quad(const float p[4][3], float r, float g, float b, float shade)
{
    static const int order[6] = {0,1,2,0,2,3};
    for (int i = 0; i < 6; ++i) {
        const float *v = p[order[i]];
        vertex(v[0],v[1],v[2],r*shade,g*shade,b*shade);
    }
}

static void map_quad(float x0,float y,float z0,float x1,float z1,float r,float g,float b)
{
    const float p[4][3]={{x0,y,z0},{x1,y,z0},{x1,y,z1},{x0,y,z1}};
    quad(p,r,g,b,1.0f);
}

static void box(float x0,float y0,float z0,float x1,float y1,float z1,
                float r,float g,float b,int map_x,int map_y,char type)
{
    for (int f=0;f<6;++f) {
        if (f==5) continue; // la cara inferior nunca se ve
        if (f==1 && map_at(map_x,map_y+1)==type) continue;
        if (f==2 && map_at(map_x,map_y-1)==type) continue;
        if (f==3 && map_at(map_x+1,map_y)==type) continue;
        if (f==4 && map_at(map_x-1,map_y)==type) continue;
        float p[4][3];
        for(int k=0;k<4;++k) {
            p[k][0]=x0+cube_faces[f][k][0]*(x1-x0);
            p[k][1]=y0+cube_faces[f][k][1]*(y1-y0);
            p[k][2]=z0+cube_faces[f][k][2]*(z1-z0);
        }
        quad(p,r,g,b,shade_face[f]);
    }
}

static int is_building(char t)
{
    return strchr("CGBA LFNDEHJKXO IQZsRcqYMVU", t) != NULL && t != ' ';
}

static void building_color(char t,float *r,float *g,float *b,float *h)
{
    *h=2.8f;
    *r=0.72f; *g=0.55f; *b=0.38f;
    switch(t) {
        case 'C': *h=1.0f; *r=.95f; *g=.78f; *b=.20f; break;
        case 'G': *h=1.5f; *r=.95f; *g=.58f; *b=.22f; break;
        case 'B': *h=1.8f; *r=.65f; *g=.25f; *b=.20f; break;
        case 'A': *h=2.5f; *r=.28f; *g=.48f; *b=.85f; break;
        case 'L': *h=2.7f; *r=.60f; *g=.42f; *b=.30f; break;
        case 'F': *h=3.2f; *r=.60f; *g=.63f; *b=.67f; break;
        case 'N': *h=3.8f; *r=.58f; *g=.36f; *b=.76f; break;
        case 'D': *h=4.5f; *r=.88f; *g=.89f; *b=.92f; break;
        case 'E': *h=6.0f; *r=.30f; *g=.68f; *b=.80f; break;
        case 'H': *h=1.6f; *r=.80f; *g=.22f; *b=.18f; break;
        case 'J': *h=1.8f; *r=.22f; *g=.42f; *b=.82f; break;
        case 'K': *h=2.2f; *r=.78f; *g=.78f; *b=.72f; break;
        case 'O': *h=4.8f; *r=.50f; *g=.62f; *b=.75f; break;
        case 'I': *h=3.5f; *r=.25f; *g=.55f; *b=.70f; break;
        case 'U': *h=3.0f; *r=.78f; *g=.65f; *b=.42f; break;
        case 'X': *h=1.8f; *r=.25f; *g=.55f; *b=.35f; break;
        case 'Q': *h=1.4f; *r=.62f; *g=.32f; *b=.20f; break;
        case 'Z': *h=3.4f; *r=.85f; *g=.35f; *b=.55f; break;
        case 's': *h=2.4f; *r=.78f; *g=.72f; *b=.62f; break;
        case 'R': *h=2.6f; *r=.72f; *g=.20f; *b=.18f; break;
        case 'M': *h=1.6f; *r=.20f; *g=.68f; *b=.42f; break;
        case 'Y': *h=3.6f; *r=.90f; *g=.90f; *b=.82f; break;
        case 'V': *h=4.2f; *r=.88f; *g=.68f; *b=.22f; break;
        case 'c': case 'q': *h=2.2f; *r=.25f; *g=.35f; *b=.52f; break;
    }
}

static void draw_city(void)
{
    int x0=player_x-MAP_RADIUS, x1=player_x+MAP_RADIUS;
    int y0=player_y-MAP_RADIUS, y1=player_y+MAP_RADIUS;
    if(x0<0)x0=0; if(y0<0)y0=0;
    if(x1>=map_width)x1=map_width-1;
    if(y1>=map_height)y1=map_height-1;

    C3D_ImmDrawBegin(GPU_TRIANGLES);
    for(int my=y0;my<=y1;++my) {
        for(int mx=x0;mx<=x1;++mx) {
            char t=map_at(mx,my);
            float wx=(mx-player_x)*CELL_SCALE;
            float wz=(my-player_y)*CELL_SCALE;
            float xlo=wx-CELL_SCALE*.5f, xhi=wx+CELL_SCALE*.5f;
            float zlo=wz-CELL_SCALE*.5f, zhi=wz+CELL_SCALE*.5f;

            float gr=.39f,gg=.70f,gb=.29f;
            if(t=='='){gr=.29f;gg=.30f;gb=.34f;}
            else if(t=='S'){gr=.70f;gg=.69f;gb=.65f;}
            else if(t=='W'){gr=.14f;gg=.42f;gb=.78f;}
            map_quad(xlo,-.02f,zlo,xhi,zhi,gr,gg,gb);

            if(is_building(t)) {
                float r,g,b,h;
                building_color(t,&r,&g,&b,&h);
                box(xlo,0.0f,zlo,xhi,h*CELL_SCALE,zhi,r,g,b,mx,my,t);

            } else if(t=='T') {
                // Árbol low-poly: tronco y copa cúbica.
                box(wx-CELL_SCALE*.12f,0,wz-CELL_SCALE*.12f,
                    wx+CELL_SCALE*.12f,CELL_SCALE*1.0f,zhi,
                    .38f,.22f,.10f,mx,my,'#');
                box(xlo+.012f,CELL_SCALE*.85f,zlo+.012f,
                    xhi-.012f,CELL_SCALE*2.7f,zhi-.012f,
                    .12f,.48f,.16f,mx,my,'#');
            }
        }
    }
    C3D_ImmDrawEnd();
}

static int scene_init(void)
{
    vshader_dvlb=DVLB_ParseFile((u32*)vshader_shbin,vshader_shbin_size);
    if(!vshader_dvlb)return 0;
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program,&vshader_dvlb->DVLE[0]);
    C3D_BindProgram(&program);
    u_projection=shaderInstanceGetUniformLocation(program.vertexShader,"projection");
    u_modelView=shaderInstanceGetUniformLocation(program.vertexShader,"modelView");
    C3D_AttrInfo *attrs=C3D_GetAttrInfo();
    AttrInfo_Init(attrs);
    AttrInfo_AddLoader(attrs,0,GPU_FLOAT,3);
    AttrInfo_AddLoader(attrs,1,GPU_FLOAT,4);
    Mtx_PerspTilt(&projection,C3D_AngleFromDegrees(55.0f),C3D_AspectRatioTop,.1f,100.0f,true);
    C3D_TexEnv *env=C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env,C3D_Both,GPU_PRIMARY_COLOR,0,0);
    C3D_TexEnvFunc(env,C3D_Both,GPU_REPLACE);
    C3D_DepthMap(true,-1.0f,0.0f);
    C3D_DepthTest(true,GPU_GEQUAL,GPU_WRITE_ALL);
    C3D_CullFace(GPU_CULL_NONE);
    return 1;
}

int main(int argc,char **argv)
{
    (void)argc;(void)argv;
    gfxInitDefault();
    consoleInit(GFX_BOTTOM,NULL);
    if(R_FAILED(romfsInit())) {
        printf("No se pudo montar ROMFS.\n");
        gfxExit(); return 1;
    }
    if(!load_city_map()) {
        printf("No se pudo leer mapas/ciudad.txt\n");
        romfsExit();gfxExit();return 1;
    }
    if(!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)) {
        printf("No se pudo iniciar Citro3D.\n");
        romfsExit();gfxExit();return 1;
    }
    target=C3D_RenderTargetCreate(240,400,GPU_RB_RGBA8,GPU_RB_DEPTH24_STENCIL8);
    if(!target) {
        C3D_Fini();romfsExit();gfxExit();return 1;
    }
    C3D_RenderTargetSetOutput(target,GFX_TOP,GFX_LEFT,DISPLAY_TRANSFER_FLAGS);
    if(!scene_init()) {
        printf("Error cargando shader.\n");
        C3D_RenderTargetDelete(target);C3D_Fini();romfsExit();gfxExit();return 1;
    }

    printf("A BRICK CUBE - CIUDAD 3DS\n");
    printf("Mapa original: %d x %d\n",map_width,map_height);
    printf("Inicio original: %d, %d\n",player_x,player_y);
    printf("Vista local: %d x %d celdas\n",MAP_RADIUS*2+1,MAP_RADIUS*2+1);
    printf("Izq./Der.: girar ciudad\n");
    printf("Arriba/Abajo: inclinar vista\n");
    printf("START: salir\n");

    while(aptMainLoop()) {
        hidScanInput();
        u32 down=hidKeysDown();
        if(down&KEY_START)break;
        if(down&KEY_LEFT)angle_y-=.08f;
        if(down&KEY_RIGHT)angle_y+=.08f;
        if(down&KEY_UP)angle_x-=.05f;
        if(down&KEY_DOWN)angle_x+=.05f;
        if(angle_x<.25f)angle_x=.25f;
        if(angle_x>1.35f)angle_x=1.35f;

        C3D_Mtx model;
        Mtx_Identity(&model);
        Mtx_RotateX(&model,angle_x,true);
        Mtx_RotateY(&model,angle_y,true);
        Mtx_Translate(&model,0.0f,-.75f,6.2f,true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER,u_projection,&projection);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER,u_modelView,&model);

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C3D_RenderTargetClear(target,C3D_CLEAR_ALL,CLEAR_COLOR,0);
        C3D_FrameDrawOn(target);
        draw_city();
        C3D_FrameEnd(0);
    }

    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
    C3D_RenderTargetDelete(target);
    C3D_Fini();
    romfsExit();
    gfxExit();
    return 0;
}
