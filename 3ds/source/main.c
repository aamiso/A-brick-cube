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
#define CELL_SCALE 0.16f

static DVLB_s *vshader_dvlb;
static shaderProgram_s program;
static C3D_RenderTarget *target;
static C3D_Mtx projection;
static int u_projection, u_modelView;
static char map_grid[MAP_MAX][MAP_MAX];
static int map_width, map_height, player_x, player_y;
static float angle_x = 0.22f, angle_y = 0.0f;
static float player_fx, player_fy;
static float player_vx, player_vy;
static float player_angle, walk_phase, anim_time, player_anim_speed;

/* Malla estatica combinada: se construye al iniciar y solo se regenera
   cuando el jugador cambia de tile. El personaje sigue siendo dinamico. */
typedef struct { float x, y, z, r, g, b, a; } WorldVertex;
static WorldVertex *build_vertices;
static size_t build_count, build_capacity;
static void *world_positions, *world_colors;
static int world_vertex_count;
static int building_world_mesh;
static int mesh_origin_x, mesh_origin_y;
static const float MESH_RECENTER_DISTANCE = 8.0f;
static const float PLAYER_SPEED = 0.052f;
static const float PLAYER_RUN_SPEED = 0.082f;
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

static float angle_lerp(float current, float target, float amount)
{
    float d = atan2f(sinf(target-current), cosf(target-current));
    return current + d * amount;
}

static void update_player(float dx, float dy, int run)
{
    float input_len = sqrtf(dx*dx + dy*dy);
    if (input_len > 1.0f) { dx /= input_len; dy /= input_len; input_len = 1.0f; }

    float speed = run ? PLAYER_RUN_SPEED : PLAYER_SPEED;
    float target_x = dx * speed;
    float target_y = dy * speed;
    /* Aceleración y frenado progresivos, como en el juego original. */
    player_vx += (target_x - player_vx) * (input_len > 0.01f ? 0.22f : 0.16f);
    player_vy += (target_y - player_vy) * (input_len > 0.01f ? 0.22f : 0.16f);

    if (player_can_stand(player_fx + player_vx, player_fy)) player_fx += player_vx;
    else player_vx = 0.0f;
    if (player_can_stand(player_fx, player_fy + player_vy)) player_fy += player_vy;
    else player_vy = 0.0f;

    float actual = sqrtf(player_vx*player_vx + player_vy*player_vy);
    player_anim_speed = actual / PLAYER_SPEED;
    if (player_anim_speed > 1.7f) player_anim_speed = 1.7f;

    if (actual > 0.0015f) {
        float target_angle = atan2f(player_vx, player_vy);
        player_angle = angle_lerp(player_angle, target_angle, 0.20f);
        walk_phase += 0.30f * (actual / PLAYER_SPEED);
    } else {
        player_anim_speed = 0.0f;
        walk_phase *= 0.82f;
        if (fabsf(player_vx) < 0.0005f) player_vx = 0.0f;
        if (fabsf(player_vy) < 0.0005f) player_vy = 0.0f;
    }
    player_x = (int)floorf(player_fx + 0.5f);
    player_y = (int)floorf(player_fy + 0.5f);
}



static void character_vertex(float x, float y, float z, float r, float g, float b)
{
    vertex(x, y, z, r, g, b);
}

/* Geometría del personaje original: piezas voxel con pivotes en cadera,
   hombros y cuello. La transformación rota la pieza alrededor del pivote,
   no alrededor del centro de cada cubo. */
static void character_box(float ox, float oy, float oz,
                          float px, float py, float pz,
                          float sx, float sy, float sz,
                          float rot_x, float rot_y, float rot_z,
                          float r, float g, float b)
{
    float cx=cosf(rot_x), sxr=sinf(rot_x);
    float cy=cosf(rot_y), syr=sinf(rot_y);
    float cz=cosf(rot_z), szr=sinf(rot_z);
    for (int f=0; f<6; ++f) {
        for (int k=0; k<6; ++k) {
            int vi=(int[]){0,1,2,0,2,3}[k];
            float x=px+(cube_faces[f][vi][0]-.5f)*sx;
            float y=py+(cube_faces[f][vi][1]-.5f)*sy;
            float z=pz+(cube_faces[f][vi][2]-.5f)*sz;
            float y1=y*cx-z*sxr, z1=y*sxr+z*cx;
            float x2=x*cz-y1*szr, y2=x*szr+y1*cz;
            float wx=x2*cy+z1*syr;
            float wz=-x2*syr+z1*cy;
            float shade=shade_face[f];
            character_vertex(ox+wx,oy+y2,oz+wz,r*shade,g*shade,b*shade);
        }
    }
}

static void draw_original_character(void)
{
    const float s=CELL_SCALE;
    const float bx=(player_fx-mesh_origin_x)*s;
    const float bz=(player_fy-mesh_origin_y)*s;

    /* Paleta exacta de personaje_crear() en src/player/personaje.c */
    const float shirt[3]={0xd9/255.0f,0x38/255.0f,0x2c/255.0f};
    const float pants[3]={0x2f/255.0f,0x62/255.0f,0xc9/255.0f};
    const float skin[3] ={0xf2/255.0f,0xc2/255.0f,0x30/255.0f};
    const float hair[3] ={0x5a/255.0f,0x3a/255.0f,0x22/255.0f};
    const float belt[3] ={0x3a/255.0f,0x2c/255.0f,0x22/255.0f};
    const float shoe[3] ={0x24/255.0f,0x24/255.0f,0x2b/255.0f};
    const float eye[3]  ={0x1b/255.0f,0x1b/255.0f,0x22/255.0f};
    const float mouth[3]={0xb8/255.0f,0x48/255.0f,0x4a/255.0f};

    float vel=player_anim_speed;
    float correr=vel>1.0f?(vel-1.0f)/0.7f:0.0f;
    if(correr>1.0f)correr=1.0f;
    if(vel>1.0f)vel=1.0f;
    float step=sinf(walk_phase);
    float quieto=(1.0f-vel);
    float balanceo=step*(48.0f+24.0f*correr)*vel*(3.14159265f/180.0f);
    float rebote=fabsf(step)*(0.07f+0.06f*correr)*vel*s;
    float respira=sinf(anim_time*2.2f)*0.012f*quieto*s;
    float inclina=((5.0f+11.0f*correr)*vel)*(3.14159265f/180.0f);
    float giro_torso=step*7.0f*vel*(3.14159265f/180.0f);
    float mira=sinf(anim_time*0.6f)*14.0f*quieto*(3.14159265f/180.0f);
    float abre=(4.0f+sinf(anim_time*2.2f)*1.5f*quieto)*(3.14159265f/180.0f);
    float root_y=rebote+respira;

    /* Piernas: cada una gira desde la cadera, igual que la malla Vita. */
    character_box(bx,root_y+.60f*s,bz,-.17f*s,-.30f*s,0,
        .30f*s,.50f*s,.30f*s,balanceo,player_angle,0,
        pants[0],pants[1],pants[2]);
    character_box(bx,root_y+.60f*s,bz,.17f*s,-.30f*s,0,
        .30f*s,.50f*s,.30f*s,-balanceo,player_angle,0,
        pants[0],pants[1],pants[2]);
    character_box(bx,root_y+.60f*s,bz,-.17f*s,-.55f*s,.015f*s,
        .30f*s,.10f*s,.34f*s,balanceo,player_angle,0,
        shoe[0],shoe[1],shoe[2]);
    character_box(bx,root_y+.60f*s,bz,.17f*s,-.55f*s,.015f*s,
        .30f*s,.10f*s,.34f*s,-balanceo,player_angle,0,
        shoe[0],shoe[1],shoe[2]);

    /* Torso, cabeza y brazos comparten el movimiento del tronco. */
    float tx=bx, ty=root_y+.60f*s, tz=bz;
    character_box(tx,ty,tz,0,-.30f*s,0,.80f*s,.60f*s,.40f*s,
        inclina,player_angle,giro_torso,shirt[0],shirt[1],shirt[2]);
    character_box(tx,ty,tz,0,-.55f*s,0,.80f*s,.10f*s,.40f*s,
        inclina,player_angle,giro_torso,belt[0],belt[1],belt[2]);
    character_box(tx,ty,tz,0,-.015f*s,.12f*s,.20f*s,.10f*s,.16f*s,
        inclina,player_angle,giro_torso,skin[0],skin[1],skin[2]);

    /* Brazos articulados: manga arriba y mano amarilla abajo. */
    character_box(tx,ty,tz,-.50f*s,-.20f*s,0,.20f*s,.40f*s,.20f*s,
        inclina-balanceo*.9f,player_angle,giro_torso+abre,
        shirt[0],shirt[1],shirt[2]);
    character_box(tx,ty,tz,-.50f*s,-.50f*s,0,.20f*s,.20f*s,.20f*s,
        inclina-balanceo*.9f,player_angle,giro_torso+abre,
        skin[0],skin[1],skin[2]);
    character_box(tx,ty,tz,.50f*s,-.20f*s,0,.20f*s,.40f*s,.20f*s,
        inclina+balanceo*.9f,player_angle,giro_torso-abre,
        shirt[0],shirt[1],shirt[2]);
    character_box(tx,ty,tz,.50f*s,-.50f*s,0,.20f*s,.20f*s,.20f*s,
        inclina+balanceo*.9f,player_angle,giro_torso-abre,
        skin[0],skin[1],skin[2]);

    /* Cabeza de 6x6x6 voxels, flequillo, nuca y patillas originales. */
    character_box(tx,ty,tz,0,.30f*s,0,.60f*s,.60f*s,.60f*s,
        inclina,player_angle,mira,skin[0],skin[1],skin[2]);
    character_box(tx,ty,tz,0,.55f*s,0,.60f*s,.10f*s,.60f*s,
        inclina,player_angle,mira,hair[0],hair[1],hair[2]);
    character_box(tx,ty,tz,0,.45f*s,-.20f*s,.60f*s,.10f*s,.20f*s,
        inclina,player_angle,mira,hair[0],hair[1],hair[2]);
    character_box(tx,ty,tz,-.25f*s,.45f*s,-.05f*s,.10f*s,.10f*s,.50f*s,
        inclina,player_angle,mira,hair[0],hair[1],hair[2]);
    character_box(tx,ty,tz,.25f*s,.45f*s,-.05f*s,.10f*s,.10f*s,.50f*s,
        inclina,player_angle,mira,hair[0],hair[1],hair[2]);
    character_box(tx,ty,tz,0,.35f*s,-.25f*s,.60f*s,.10f*s,.10f*s,
        inclina,player_angle,mira,hair[0],hair[1],hair[2]);
    character_box(tx,ty,tz,0,.45f*s,.25f*s,.60f*s,.10f*s,.10f*s,
        inclina,player_angle,mira,hair[0],hair[1],hair[2]);

    /* Ojos y boca en la cara frontal (+Z). */
    character_box(tx,ty,tz,-.20f*s,.35f*s,.302f*s,.10f*s,.10f*s,.025f*s,
        inclina,player_angle,mira,eye[0],eye[1],eye[2]);
    character_box(tx,ty,tz,.20f*s,.35f*s,.302f*s,.10f*s,.10f*s,.025f*s,
        inclina,player_angle,mira,eye[0],eye[1],eye[2]);
    character_box(tx,ty,tz,0,.15f*s,.302f*s,.20f*s,.10f*s,.025f*s,
        inclina,player_angle,mira,mouth[0],mouth[1],mouth[2]);
}


static void draw_city(void)
{
    /* Toda la ciudad se construye de una vez. Los suelos y edificios
       contiguos del mismo tipo se fusionan en rectángulos para reducir
       drásticamente vértices y llamadas de dibujo. */
    static unsigned char used[MAP_MAX][MAP_MAX];
    memset(used, 0, sizeof(used));
    if (!building_world_mesh) C3D_ImmDrawBegin(GPU_TRIANGLES);

    for (int my = 0; my < map_height; ++my) {
        for (int mx = 0; mx < map_width; ++mx) {
            char t = map_at(mx, my);
            if (used[my][mx]) continue;

            float wx = (mx - mesh_origin_x) * CELL_SCALE;
            float wz = (my - mesh_origin_y) * CELL_SCALE;
            float half = CELL_SCALE * .5f;

            if (t == 'T') {
                used[my][mx] = 1;
                box(wx-half*.24f, 0, wz-half*.24f,
                    wx+half*.24f, CELL_SCALE, wz+half*.24f,
                    .38f,.22f,.10f,-1,-1,'#');
                box(wx-half, CELL_SCALE*.85f, wz-half,
                    wx+half, CELL_SCALE*2.7f, wz+half,
                    .12f,.48f,.16f,-1,-1,'#');
                continue;
            }

            int w = 1, h = 1;
            while (mx+w < map_width && !used[my][mx+w] &&
                   map_at(mx+w,my) == t && t != 'P') w++;
            for (;;) {
                if (my+h >= map_height || t == 'P') break;
                int same = 1;
                for (int k=0;k<w;k++)
                    if (used[my+h][mx+k] || map_at(mx+k,my+h) != t) { same=0; break; }
                if (!same) break;
                h++;
            }
            for (int yy=0; yy<h; ++yy)
                for (int xx=0; xx<w; ++xx) used[my+yy][mx+xx] = 1;

            float xlo = wx-half;
            float zlo = wz-half;
            float xhi = wx + (w-.5f)*CELL_SCALE;
            float zhi = wz + (h-.5f)*CELL_SCALE;

            float gr=.39f, gg=.70f, gb=.29f;
            if (t=='=') { gr=.29f; gg=.30f; gb=.34f; }
            else if (t=='S') { gr=.70f; gg=.69f; gb=.65f; }
            else if (t=='W') { gr=.14f; gg=.42f; gb=.78f; }

            if (!is_building(t))
                map_quad(xlo,-.02f,zlo,xhi,zhi,gr,gg,gb);

            if (is_building(t)) {
                float r,g,b,bh;
                building_color(t,&r,&g,&b,&bh);
                /* Un solo prisma por bloque contiguo, sin caras internas. */
                box(xlo,0,zlo,xhi,bh*CELL_SCALE,zhi,r,g,b,-1,-1,'#');
            }
        }
    }
    if (!building_world_mesh) C3D_ImmDrawEnd();
}

/* Convierte el escenario estatico en dos buffers GPU reutilizables.
   Posiciones y colores se separan para usar el formato de atributos PICA. */
static int rebuild_world_mesh(void)
{
    mesh_origin_x = player_x;
    mesh_origin_y = player_y;
    world_vertex_count = 0;
    if (world_positions) { linearFree(world_positions); world_positions = NULL; }
    if (world_colors) { linearFree(world_colors); world_colors = NULL; }
    build_count = 0;
    building_world_mesh = 1;
    draw_city();
    building_world_mesh = 0;

    if (build_count == 0 || build_count > 700000) {
        free(build_vertices);
        build_vertices = NULL;
        build_capacity = 0;
        return 0;
    }
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
        u32 held = hidKeysHeld();
        /* Movimiento relativo a la cámara: arriba siempre avanza hacia delante. */
        float stick_x = (float)circle.dx / 156.0f;
        float stick_y = -(float)circle.dy / 156.0f;
        if (held & KEY_RIGHT) stick_x += 1.0f;
        if (held & KEY_LEFT)  stick_x -= 1.0f;
        if (held & KEY_UP)    stick_y += 1.0f;
        if (held & KEY_DOWN)  stick_y -= 1.0f;
        float move_x = sinf(angle_y)*stick_y + cosf(angle_y)*stick_x;
        float move_y = cosf(angle_y)*stick_y - sinf(angle_y)*stick_x;
        update_player(move_x, move_y, (held & KEY_B) != 0);
        /* La malla se conserva entre varios tiles. Solo se reconstruye al
           alejarse 8 tiles del centro, evitando pausas de CPU cada paso. */
        float mdx = player_fx - (float)mesh_origin_x;
        float mdy = player_fy - (float)mesh_origin_y;
        if (mdx*mdx + mdy*mdy >= MESH_RECENTER_DISTANCE*MESH_RECENTER_DISTANCE) {
            if (!rebuild_world_mesh())
                printf("Aviso: no se pudo actualizar la malla del sector.\\n");
        }
        anim_time += 1.0f / 60.0f;

        /* La cámara permanece detrás de la espalda y gira suavemente con él. */
        angle_y = angle_lerp(angle_y, player_angle, 0.14f);

        C3D_Mtx model;
        Mtx_Identity(&model);
        Mtx_Translate(&model,-(player_fx-mesh_origin_x)*CELL_SCALE,-.10f,-.58f-(player_fy-mesh_origin_y)*CELL_SCALE,true);
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
            draw_city();
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
