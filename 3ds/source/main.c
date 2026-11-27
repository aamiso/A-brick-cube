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
#define CHUNK_SIZE 16
#define RENDER_DISTANCE 26.0f
#define CELL_SCALE 0.16f

static DVLB_s *vshader_dvlb;
static shaderProgram_s program;
static C3D_RenderTarget *target;
static C3D_Mtx projection;
static int u_projection, u_modelView;
static char map_grid[MAP_MAX][MAP_MAX];
static int map_width, map_height, player_x, player_y;
static float angle_x = 0.82f, angle_y = 0.35f;
static float camera_target_yaw = 0.35f;
static float player_fx, player_fy;
static float player_angle, walk_phase, anim_time;

/* Malla estatica combinada: se construye al iniciar y solo se regenera
   cuando el jugador cambia de tile. El personaje sigue siendo dinamico. */
typedef struct { float x, y, z, r, g, b, a; } WorldVertex;
static WorldVertex *build_vertices;
static size_t build_count, build_capacity;
static void *world_positions, *world_colors;
static int world_vertex_count;
static int building_world_mesh;
static const float PLAYER_SPEED = 0.075f;
static const float PLAYER_RADIUS = 0.28f;

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
    if (building_world_mesh) {
        if (build_count >= build_capacity) {
            size_t next = build_capacity ? build_capacity * 2 : 16384;
            WorldVertex *grown = (WorldVertex *)realloc(build_vertices, next * sizeof(WorldVertex));
            if (!grown) return;
            build_vertices = grown;
            build_capacity = next;
        }
        WorldVertex *v = &build_vertices[build_count++];
        v->x=x; v->y=y; v->z=z; v->r=r; v->g=g; v->b=b; v->a=1.0f;
        return;
    }
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

static int solid_tile(char t)
{
    return t == 'T' || t == 'W' ||
           (strchr("CGBA LFNDEHJKXO IQZsRcqYMVU", t) != NULL && t != ' ');
}

static int player_can_stand(float x, float y)
{
    const float offsets[5][2] = {
        {0.0f, 0.0f}, {-PLAYER_RADIUS, 0.0f}, {PLAYER_RADIUS, 0.0f},
        {0.0f, -PLAYER_RADIUS}, {0.0f, PLAYER_RADIUS}
    };
    for (int i = 0; i < 5; ++i) {
        int tx = (int)floorf(x + offsets[i][0] + 0.5f);
        int ty = (int)floorf(y + offsets[i][1] + 0.5f);
        if (tx < 0 || ty < 0 || tx >= map_width || ty >= map_height) return 0;
        if (solid_tile(map_at(tx, ty))) return 0;
    }
    return 1;
}

static void update_player(float dx, float dy)
{
    float length = sqrtf(dx * dx + dy * dy);
    if (length > 1.0f) { dx /= length; dy /= length; }
    dx *= PLAYER_SPEED;
    dy *= PLAYER_SPEED;
    if (player_can_stand(player_fx + dx, player_fy)) player_fx += dx;
    if (player_can_stand(player_fx, player_fy + dy)) player_fy += dy;
    player_x = (int)floorf(player_fx + 0.5f);
    player_y = (int)floorf(player_fy + 0.5f);
}


static void character_vertex(float x, float y, float z, float r, float g, float b)
{
    vertex(x, y, z, r, g, b);
}

// Cuboide del personaje: conserva la forma de bloques del modelo original.
// Las piezas giran alrededor de su pivote y luego siguen la orientacion del jugador.
static void character_box(float ox, float oy, float oz,
                          float px, float py, float pz,
                          float sx, float sy, float sz,
                          float rot_x, float rot_y,
                          float r, float g, float b)
{
    float cx = cosf(rot_x), sxr = sinf(rot_x);
    float cy = cosf(rot_y), syr = sinf(rot_y);
    for (int f = 0; f < 6; ++f) {
        for (int k = 0; k < 6; ++k) {
            int vi = (int[]){0,1,2,0,2,3}[k];
            float lx = (cube_faces[f][vi][0] - 0.5f) * sx;
            float ly = (cube_faces[f][vi][1] - 0.5f) * sy;
            float lz = (cube_faces[f][vi][2] - 0.5f) * sz;
            float ry = ly * cx - lz * sxr;
            float rz = ly * sxr + lz * cx;
            float rx = lx * cy + rz * syr;
            float rz2 = -lx * syr + rz * cy;
            float wx = ox + px + rx;
            float wy = oy + py + ry;
            float wz = oz + pz + rz2;
            float shade = shade_face[f];
            character_vertex(wx, wy, wz, r * shade, g * shade, b * shade);
        }
    }
}

static void draw_original_character(void)
{
    const float s = CELL_SCALE;
    const float base_x = (player_fx - player_x) * s;
    const float base_z = (player_fy - player_y) * s;
    const float skin_r=.95f, skin_g=.76f, skin_b=.60f;
    const float shirt_r=.85f, shirt_g=.22f, shirt_b=.17f;
    const float pants_r=.18f, pants_g=.38f, pants_b=.79f;
    const float hair_r=.35f, hair_g=.22f, hair_b=.13f;
    const float shoe_r=.14f, shoe_g=.14f, shoe_b=.17f;
    const float step = sinf(walk_phase) * .48f;

    // Piernas: 0.6 unidades de alto, pivote en la cadera.
    character_box(base_x, 0, base_z,
        -.017f*s, .60f*s, 0, .30f*s, .60f*s, .30f*s,
        step, player_angle, pants_r,pants_g,pants_b);
    character_box(base_x, 0, base_z,
        .17f*s, .60f*s, 0, .30f*s, .60f*s, .30f*s,
        -step, player_angle, pants_r,pants_g,pants_b);
    // Zapatos negros en la base de cada pierna.
    character_box(base_x, 0, base_z,
        -.017f*s, .04f*s, .035f*s, .34f*s, .12f*s, .42f*s,
        step, player_angle, shoe_r,shoe_g,shoe_b);
    character_box(base_x, 0, base_z,
        .17f*s, .04f*s, .035f*s, .34f*s, .12f*s, .42f*s,
        -step, player_angle, shoe_r,shoe_g,shoe_b);

    // Torso y cinturón.
    character_box(base_x, 0, base_z,
        0, .90f*s, 0, .80f*s, .60f*s, .40f*s,
        0, player_angle, shirt_r,shirt_g,shirt_b);
    character_box(base_x, 0, base_z,
        0, .65f*s, 0, .80f*s, .10f*s, .40f*s,
        0, player_angle, hair_r, hair_g, hair_b);

    // Brazos articulados: camisa arriba y manos color piel abajo.
    character_box(base_x, 0, base_z,
        -.50f*s, .90f*s, 0, .20f*s, .40f*s, .20f*s,
        -step*.85f, player_angle, shirt_r,shirt_g,shirt_b);
    character_box(base_x, 0, base_z,
        -.50f*s, .60f*s, 0, .20f*s, .20f*s, .20f*s,
        -step*.85f, player_angle, skin_r,skin_g,skin_b);
    character_box(base_x, 0, base_z,
        .50f*s, .90f*s, 0, .20f*s, .40f*s, .20f*s,
        step*.85f, player_angle, shirt_r,shirt_g,shirt_b);
    character_box(base_x, 0, base_z,
        .50f*s, .60f*s, 0, .20f*s, .20f*s, .20f*s,
        step*.85f, player_angle, skin_r,skin_g,skin_b);

    // Cabeza, pelo superior y flequillo frontal (+Z).
    character_box(base_x, 0, base_z,
        0, 1.50f*s, 0, .60f*s, .60f*s, .60f*s,
        0, player_angle, skin_r,skin_g,skin_b);
    character_box(base_x, 0, base_z,
        0, 1.95f*s, 0, .60f*s, .10f*s, .60f*s,
        0, player_angle, hair_r,hair_g,hair_b);
    character_box(base_x, 0, base_z,
        0, 1.85f*s, .25f*s, .60f*s, .10f*s, .10f*s,
        0, player_angle, hair_r,hair_g,hair_b);
    // Ojos y boca del rostro.
    character_box(base_x, 0, base_z,
        -.15f*s, 1.68f*s, .305f*s, .10f*s, .10f*s, .025f*s,
        0, player_angle, .10f,.10f,.13f);
    character_box(base_x, 0, base_z,
        .15f*s, 1.68f*s, .305f*s, .10f*s, .10f*s, .025f*s,
        0, player_angle, .10f,.10f,.13f);
    character_box(base_x, 0, base_z,
        0, 1.53f*s, .305f*s, .20f*s, .04f*s, .025f*s,
        0, player_angle, .72f,.28f,.29f);
}

static void draw_city(void)
{
    /*
     * El mapa completo permanece cargado en map_grid. La ciudad se recorre
     * por sectores de 16x16: solo se generan vertices para los sectores
     * cercanos al jugador. Esto es distancia de dibujado, no un limite
     * de exploracion; el jugador puede recorrer los 252x252 tiles.
     */
    const float max_dist2 = RENDER_DISTANCE * RENDER_DISTANCE;
    const float chunk_radius = (float)CHUNK_SIZE * 0.7072f;
    const int chunks_x = (map_width + CHUNK_SIZE - 1) / CHUNK_SIZE;
    const int chunks_y = (map_height + CHUNK_SIZE - 1) / CHUNK_SIZE;

    if (!building_world_mesh) C3D_ImmDrawBegin(GPU_TRIANGLES);
    for (int cy = 0; cy < chunks_y; ++cy) {
        for (int cx = 0; cx < chunks_x; ++cx) {
            int bx = cx * CHUNK_SIZE;
            int by = cy * CHUNK_SIZE;
            int ex = bx + CHUNK_SIZE;
            int ey = by + CHUNK_SIZE;
            if (ex > map_width) ex = map_width;
            if (ey > map_height) ey = map_height;

            // Culling de sector usando el punto mas cercano de su rectangulo.
            float near_x = player_fx;
            float near_y = player_fy;
            if (near_x < (float)bx) near_x = (float)bx;
            if (near_x > (float)(ex - 1)) near_x = (float)(ex - 1);
            if (near_y < (float)by) near_y = (float)by;
            if (near_y > (float)(ey - 1)) near_y = (float)(ey - 1);
            float ndx = near_x - player_fx;
            float ndy = near_y - player_fy;
            if (ndx * ndx + ndy * ndy > max_dist2) continue;

            for (int my = by; my < ey; ++my) {
                for (int mx = bx; mx < ex; ++mx) {
                    float dx = (float)mx - player_fx;
                    float dy = (float)my - player_fy;
                    if (dx * dx + dy * dy > max_dist2) continue;

                    char t = map_at(mx, my);
                    float wx = (mx - player_x) * CELL_SCALE;
                    float wz = (my - player_y) * CELL_SCALE;
                    float xlo = wx - CELL_SCALE * .5f;
                    float xhi = wx + CELL_SCALE * .5f;
                    float zlo = wz - CELL_SCALE * .5f;
                    float zhi = wz + CELL_SCALE * .5f;

                    float gr = .39f, gg = .70f, gb = .29f;
                    if (t == '=') { gr = .29f; gg = .30f; gb = .34f; }
                    else if (t == 'S') { gr = .70f; gg = .69f; gb = .65f; }
                    else if (t == 'W') { gr = .14f; gg = .42f; gb = .78f; }

                    if (!is_building(t) && t != 'T')
                        map_quad(xlo, -.02f, zlo, xhi, zhi, gr, gg, gb);

                    if (is_building(t)) {
                        float r, g, b, h;
                        building_color(t, &r, &g, &b, &h);
                        box(xlo, 0.0f, zlo, xhi, h * CELL_SCALE, zhi,
                            r, g, b, mx, my, t);
                    } else if (t == 'T') {
                        box(wx - CELL_SCALE * .12f, 0, wz - CELL_SCALE * .12f,
                            wx + CELL_SCALE * .12f, CELL_SCALE, wz + CELL_SCALE * .12f,
                            .38f, .22f, .10f, mx, my, '#');
                        box(xlo + .012f, CELL_SCALE * .85f, zlo + .012f,
                            xhi - .012f, CELL_SCALE * 2.7f, zhi - .012f,
                            .12f, .48f, .16f, mx, my, '#');
                    }
                }
            }
        }
    }
    if (!building_world_mesh) {
        C3D_ImmDrawEnd();
    }
}

/* Convierte el escenario estatico en dos buffers GPU reutilizables.
   Posiciones y colores se separan para usar el formato de atributos PICA. */
static int rebuild_world_mesh(void)
{
    if (world_positions) { linearFree(world_positions); world_positions = NULL; }
    if (world_colors) { linearFree(world_colors); world_colors = NULL; }
    build_count = 0;
    building_world_mesh = 1;
    draw_city();
    building_world_mesh = 0;

    if (build_count == 0 || build_count > 700000) return 0;
    world_positions = linearAlloc(build_count * 3 * sizeof(float));
    world_colors = linearAlloc(build_count * 4 * sizeof(float));
    if (!world_positions || !world_colors) {
        if (world_positions) linearFree(world_positions);
        if (world_colors) linearFree(world_colors);
        world_positions = world_colors = NULL;
        return 0;
    }
    float *p = (float *)world_positions;
    float *col = (float *)world_colors;
    for (size_t i = 0; i < build_count; ++i) {
        const WorldVertex *v = &build_vertices[i];
        *p++=v->x; *p++=v->y; *p++=v->z;
        *col++=v->r; *col++=v->g; *col++=v->b; *col++=v->a;
    }
    world_vertex_count = (int)build_count;
    free(build_vertices);
    build_vertices = NULL;
    build_capacity = 0;
    return 1;
}

static void draw_world_mesh(void)
{
    C3D_BufInfo *buf = C3D_GetBufInfo();
    BufInfo_Init(buf);
    BufInfo_Add(buf, world_positions, 3 * sizeof(float), 1, 0x0);
    BufInfo_Add(buf, world_colors, 4 * sizeof(float), 1, 0x1);
    C3D_DrawArrays(GPU_TRIANGLES, 0, world_vertex_count);

    C3D_ImmDrawBegin(GPU_TRIANGLES);
    draw_original_character();
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
    Mtx_PerspTilt(&projection,C3D_AngleFromDegrees(55.0f),C3D_AspectRatioTop,.1f,100.0f,false);
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
    printf("Ciudad completa: %d x %d tiles\n", map_width, map_height);
    printf("Render por sectores: %d x %d tiles\n", CHUNK_SIZE, CHUNK_SIZE);
    player_fx = (float)player_x;
    player_fy = (float)player_y;
    if (!rebuild_world_mesh())
        printf("Aviso: no se pudo crear la malla estatica; se usara render de emergencia.\\n");
    printf("Stick: mover jugador\n");
    printf("Cruceta izq./der.: girar camara\n");
    printf("Cruceta arriba/abajo: inclinar\n");
    printf("Colisiones: edificios, arboles y agua\n");
    printf("START: salir\n");

    while(aptMainLoop()) {
        hidScanInput();
        u32 down=hidKeysDown();
        if(down&KEY_START)break;
        circlePosition circle;
        hidCircleRead(&circle);
        float move_x = (float)circle.dx / 156.0f;
        float move_y = -(float)circle.dy / 156.0f;
        int old_tile_x = player_x, old_tile_y = player_y;
        update_player(move_x, move_y);
        if (player_x != old_tile_x || player_y != old_tile_y) {
            if (!rebuild_world_mesh())
                printf("Aviso: no se pudo actualizar la malla del sector.\\n");
        }
        if (sqrtf(move_x * move_x + move_y * move_y) > 0.18f) {
            player_angle = atan2f(move_x, move_y);
            walk_phase += 0.30f;
        } else {
            walk_phase *= 0.82f;
        }
        anim_time += 1.0f / 60.0f;

        // La camara orbita alrededor del jugador y mira desde atras
        // hacia la direccion en la que se esta desplazando.
        float move_length = sqrtf(move_x * move_x + move_y * move_y);
        if (move_length > 0.18f)
            camera_target_yaw = atan2f(-move_x, move_y);

        if(down&KEY_LEFT)camera_target_yaw-=.18f;
        if(down&KEY_RIGHT)camera_target_yaw+=.18f;
        float yaw_delta = atan2f(sinf(camera_target_yaw-angle_y),
                                 cosf(camera_target_yaw-angle_y));
        angle_y += yaw_delta * .12f;
        if(down&KEY_UP)angle_x-=.05f;
        if(down&KEY_DOWN)angle_x+=.05f;
        if(angle_x<.25f)angle_x=.25f;
        if(angle_x>1.35f)angle_x=1.35f;

        C3D_Mtx model;
        Mtx_Identity(&model);
        Mtx_Translate(&model,0.0f,-.48f,-4.25f,true);
        Mtx_RotateX(&model,angle_x,true);
        Mtx_RotateY(&model,angle_y,true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER,u_projection,&projection);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER,u_modelView,&model);

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C3D_RenderTargetClear(target,C3D_CLEAR_ALL,CLEAR_COLOR,0);
        C3D_FrameDrawOn(target);
        if (world_positions && world_vertex_count > 0)
            draw_world_mesh();
        else {
            C3D_ImmDrawBegin(GPU_TRIANGLES);
            draw_city();
            C3D_ImmDrawEnd();
        }
        C3D_FrameEnd(0);
    }

    if (world_positions) linearFree(world_positions);
    if (world_colors) linearFree(world_colors);
    free(build_vertices);
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
    C3D_RenderTargetDelete(target);
    C3D_Fini();
    romfsExit();
    gfxExit();
    return 0;
}
