#include "vitaGL.h"
#include <3ds.h>
#include <citro3d.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Adaptador de la API fija (estilo vitaGL) usada por el juego a Citro3D.
   - Las matrices se combinan en CPU (una sola MVP por dibujo) y la GPU de la
     3DS transforma los vértices (antes se hacía vértice por vértice en CPU).
   - La MVP incluye el giro de 90° de la pantalla superior de 3DS y el rango
     de profundidad de PICA200 ([-1,0] en lugar del [-1,1] de OpenGL).
   - Los buffers (glBufferData) viven en memoria lineal, que la GPU lee directo.
   - Los datos que no están en memoria lineal se copian a un buffer por cuadro. */

extern void brick3ds_set_mvp(const C3D_Mtx *m);

#define STACK_MAX 32
#define BUFFER_MAX 4096
#define RING_BYTES (3u * 1024u * 1024u)
#define RING_MAX_BYTES (12u * 1024u * 1024u)

typedef struct { float m[16]; } Mat4;            /* columna mayor, como OpenGL */
typedef struct { void *data; size_t size; int used; int linear; } Buffer;
typedef struct Pending { void *data; int linear; struct Pending *next; } Pending;

static Mat4 projection, modelview, *current = &modelview;
static Mat4 stack[STACK_MAX];
static int stack_top;
static Buffer buffers[BUFFER_MAX];
static unsigned next_hint = 1, bound_buffer = 0;

static const unsigned char *vertex_ptr, *color_ptr;
static int vertex_lin = 0, color_lin = 0;
static int vertex_size = 3, color_size = 4, vertex_stride = 0, color_stride = 0;
static unsigned vertex_type = GL_FLOAT, color_type = GL_UNSIGNED_BYTE;
static int vertex_enabled = 0, color_enabled = 0;
static float current_color[4] = {1, 1, 1, 1};

static unsigned char *ring;
static size_t ring_used;
static Pending *pending_head;
static unsigned pending_leaks;

/* ---------- matrices ---------- */
static void identity(Mat4 *m){memset(m,0,sizeof(*m));m->m[0]=m->m[5]=m->m[10]=m->m[15]=1.0f;}
static void multiply(Mat4 *out,const Mat4 *a,const Mat4 *b){
    Mat4 t;
    for(int c=0;c<4;c++)for(int r=0;r<4;r++){
        float s=0;for(int k=0;k<4;k++)s+=a->m[k*4+r]*b->m[c*4+k];
        t.m[c*4+r]=s;
    }
    *out=t;
}
static void multiply_current(const Mat4 *rhs){Mat4 t;multiply(&t,current,rhs);*current=t;}

void glMatrixMode(unsigned int mode){current=(mode==GL_PROJECTION)?&projection:&modelview;}
void glLoadIdentity(void){identity(current);}
void glPushMatrix(void){if(stack_top<STACK_MAX)stack[stack_top++]=*current;}
void glPopMatrix(void){if(stack_top>0)*current=stack[--stack_top];}
void glTranslatef(float x,float y,float z){Mat4 t;identity(&t);t.m[12]=x;t.m[13]=y;t.m[14]=z;multiply_current(&t);}
void glScalef(float x,float y,float z){Mat4 t;identity(&t);t.m[0]=x;t.m[5]=y;t.m[10]=z;multiply_current(&t);}
void glRotatef(float angle,float x,float y,float z){
    float len=sqrtf(x*x+y*y+z*z);if(len<1e-8f)return;x/=len;y/=len;z/=len;
    float a=angle*0.01745329251994329577f,c=cosf(a),s=sinf(a),q=1-c;Mat4 t;identity(&t);
    t.m[0]=x*x*q+c;t.m[4]=x*y*q-z*s;t.m[8]=x*z*q+y*s;
    t.m[1]=y*x*q+z*s;t.m[5]=y*y*q+c;t.m[9]=y*z*q-x*s;
    t.m[2]=z*x*q-y*s;t.m[6]=z*y*q+x*s;t.m[10]=z*z*q+c;
    multiply_current(&t);
}
void glFrustum(double l,double r,double b,double t,double n,double f){
    Mat4 m;memset(&m,0,sizeof(m));
    if(n<=0||f<=n||r==l||t==b)return;
    m.m[0]=(float)(2*n/(r-l));m.m[5]=(float)(2*n/(t-b));
    m.m[8]=(float)((r+l)/(r-l));m.m[9]=(float)((t+b)/(t-b));
    m.m[10]=(float)(-(f+n)/(f-n));m.m[11]=-1;
    m.m[14]=(float)(-2*f*n/(f-n));multiply_current(&m);
}
void glOrtho(double l,double r,double b,double t,double n,double f){
    Mat4 m;identity(&m);if(r==l||t==b||f==n)return;
    m.m[0]=(float)(2/(r-l));m.m[5]=(float)(2/(t-b));m.m[10]=(float)(-2/(f-n));
    m.m[12]=(float)(-(r+l)/(r-l));m.m[13]=(float)(-(t+b)/(t-b));m.m[14]=(float)(-(f+n)/(f-n));multiply_current(&m);
}

/* Corrección de 3DS aplicada después de la proyección:
   x' = y, y' = -x   (la pantalla superior está girada 90°)
   z' = (z - w) / 2  (OpenGL [-1,1] -> PICA [-1,0]; cerca = -1, lejos = 0) */
static void upload_mvp(void){
    Mat4 mvp,fix,out;
    multiply(&mvp,&projection,&modelview);
    memset(&fix,0,sizeof(fix));
    fix.m[1*4+0]=1.0f;     /* fila 0 toma y */
    fix.m[0*4+1]=-1.0f;    /* fila 1 toma -x */
    fix.m[2*4+2]=0.5f; fix.m[3*4+2]=-0.5f;
    fix.m[3*4+3]=1.0f;
    multiply(&out,&fix,&mvp);
    C3D_Mtx u;
    for(int r=0;r<4;r++){
        u.r[r].x=out.m[0*4+r]; u.r[r].y=out.m[1*4+r];
        u.r[r].z=out.m[2*4+r]; u.r[r].w=out.m[3*4+r];
    }
    brick3ds_set_mvp(&u);
}

/* ---------- estado ---------- */
void glViewport(int x,int y,int w,int h){(void)x;(void)y;(void)w;(void)h;}
void glEnable(unsigned int cap){if(cap==GL_DEPTH_TEST)C3D_DepthTest(true,GPU_GEQUAL,GPU_WRITE_ALL);}
void glDisable(unsigned int cap){if(cap==GL_DEPTH_TEST)C3D_DepthTest(false,GPU_ALWAYS,GPU_WRITE_COLOR);}
void glEnableClientState(unsigned int cap){if(cap==GL_VERTEX_ARRAY)vertex_enabled=1;if(cap==GL_COLOR_ARRAY)color_enabled=1;}
void glDisableClientState(unsigned int cap){if(cap==GL_VERTEX_ARRAY)vertex_enabled=0;if(cap==GL_COLOR_ARRAY)color_enabled=0;}
void glColor3f(float r,float g,float b){current_color[0]=r;current_color[1]=g;current_color[2]=b;current_color[3]=1;}

/* ---------- buffers ---------- */
static const unsigned char *resolve_pointer(const void *p,int *is_linear){
    *is_linear=0;
    if(!bound_buffer)return (const unsigned char*)p;
    uintptr_t off=(uintptr_t)p;
    Buffer *b=&buffers[bound_buffer];
    if(!b->used||!b->data||off>=b->size)return NULL;
    *is_linear=b->linear;
    return (const unsigned char*)b->data+off;
}
void glVertexPointer(int size,unsigned int type,int stride,const void *ptr){vertex_size=size;vertex_type=type;vertex_stride=stride;vertex_ptr=resolve_pointer(ptr,&vertex_lin);}
void glColorPointer(int size,unsigned int type,int stride,const void *ptr){color_size=size;color_type=type;color_stride=stride;color_ptr=resolve_pointer(ptr,&color_lin);}

/* IDs reutilizables: la ciudad crea y borra mallas al caminar, así que los IDs
   nunca deben dar la vuelta y pisar un buffer que sigue vivo. */
void glGenBuffers(int n,unsigned int *ids){
    for(int i=0;i<n;i++){
        ids[i]=0;
        for(int k=0;k<BUFFER_MAX-1;k++){
            unsigned id=1+((next_hint-1+k)%(BUFFER_MAX-1));
            if(!buffers[id].used){buffers[id].used=1;buffers[id].data=NULL;buffers[id].size=0;ids[i]=id;next_hint=id+1;break;}
        }
    }
}
void glBindBuffer(unsigned int target,unsigned int id){if(target==GL_ARRAY_BUFFER)bound_buffer=(id<BUFFER_MAX)?id:0;}

/* La GPU puede seguir leyendo un buffer durante el cuadro actual: se libera
   al inicio del siguiente cuadro. */
static void release_later(void *data,int linear){
    if(!data)return;
    Pending *p=(Pending*)malloc(sizeof(Pending));
    if(!p){ pending_leaks++; return; } /* Ante duda, filtrar memoria es más seguro que liberar mientras la GPU la usa. */
    p->data=data;p->linear=linear;p->next=pending_head;pending_head=p;
}
void glBufferData(unsigned int target,long size,const void *data,unsigned int usage){
    (void)usage;
    if(target!=GL_ARRAY_BUFFER||!bound_buffer||size<0)return;
    Buffer *b=&buffers[bound_buffer];
    release_later(b->data,b->linear);
    b->data=NULL;b->size=0;b->linear=0;
    if(!size)return;
    b->data=linearAlloc((size_t)size);
    if(b->data)b->linear=1; else b->data=malloc((size_t)size);
    if(!b->data)return;
    b->size=(size_t)size;
    if(data)memcpy(b->data,data,(size_t)size);
    if(b->linear)GSPGPU_FlushDataCache(b->data,(u32)size);
}
int brick3ds_buffer_valid(unsigned int id,size_t minimum_size){
    return id>0&&id<BUFFER_MAX&&buffers[id].used&&buffers[id].data&&buffers[id].size>=minimum_size;
}

void glDeleteBuffers(int n,const unsigned int *ids){
    for(int i=0;i<n;i++){
        unsigned id=ids[i];
        if(id==0||id>=BUFFER_MAX)continue;
        release_later(buffers[id].data,buffers[id].linear);
        buffers[id].data=NULL;buffers[id].size=0;buffers[id].used=0;buffers[id].linear=0;
        if(bound_buffer==id)bound_buffer=0;
    }
}

/* ---------- buffer temporal por cuadro (memoria lineal) ---------- */
static size_t ring_capacity;
static void *ring_copy(const unsigned char *src,int stride,int elem,int count){
    if(!src||stride<=0||elem<=0||count<=0)return NULL;
    size_t need=(size_t)elem*(size_t)count;
    size_t start=(ring_used+15u)&~(size_t)15u;
    if(need>RING_MAX_BYTES||start>RING_MAX_BYTES-need)return NULL;
    if(!ring||start+need>ring_capacity){
        size_t wanted=start+need, capacity=ring_capacity?ring_capacity:RING_BYTES;
        while(capacity<wanted&&capacity<RING_MAX_BYTES)capacity*=2;
        if(capacity>RING_MAX_BYTES)capacity=RING_MAX_BYTES;
        unsigned char *larger=(unsigned char*)linearAlloc(capacity);
        if(!larger)return NULL;
        if(ring&&ring_used){
            GSPGPU_FlushDataCache(ring,(u32)ring_used);
            memcpy(larger,ring,ring_used);
            release_later(ring,1);
        }
        ring=larger;ring_capacity=capacity;
    }
    unsigned char *dst=ring+start;
    if(stride==elem)memcpy(dst,src,need);
    else for(int i=0;i<count;i++)memcpy(dst+(size_t)i*elem,src+(size_t)i*stride,(size_t)elem);
    ring_used=start+need;
    return dst;
}

void brick3ds_compat_init(void){
    ring=(unsigned char*)linearAlloc(RING_BYTES);
    ring_capacity=ring?RING_BYTES:0;
    identity(&projection);identity(&modelview);
}
void brick3ds_compat_frame_begin(void){
    /* C3D_FRAME_SYNCDRAW ya esperó a que la GPU terminara el cuadro anterior. */
    Pending *p=pending_head;
    while(p){Pending *next=p->next;if(p->linear)linearFree(p->data);else free(p->data);free(p);p=next;}
    pending_head=NULL;
    ring_used=0;
}
void brick3ds_flush(void){
    if(ring&&ring_used)GSPGPU_FlushDataCache(ring,(u32)ring_used);
}
void brick3ds_compat_exit(void){
    brick3ds_compat_frame_begin();
    for(int i=1;i<BUFFER_MAX;i++)if(buffers[i].data){if(buffers[i].linear)linearFree(buffers[i].data);else free(buffers[i].data);buffers[i].data=NULL;}
    if(ring){linearFree(ring);ring=NULL;ring_capacity=0;}
}

/* ---------- dibujo ---------- */
void glDrawArrays(unsigned int mode,int first,int count){
    if(mode!=GL_TRIANGLES||!vertex_enabled||!vertex_ptr||count<=0||first<0)return;
    if(vertex_type!=GL_FLOAT)return;

    int use_color=color_enabled&&color_ptr&&color_type==GL_UNSIGNED_BYTE;
    int vs=vertex_stride?vertex_stride:vertex_size*(int)sizeof(float);
    int cs=color_stride?color_stride:color_size;
    int vel=vertex_size*(int)sizeof(float);

    const void *vbuf,*cbuf=NULL; int draw_first;
    int direct=vertex_lin&&(!use_color||color_lin);
    if(direct){
        vbuf=vertex_ptr; cbuf=use_color?color_ptr:NULL; draw_first=first;
    } else {
        vbuf=ring_copy(vertex_ptr+(size_t)first*vs,vs,vel,count);
        if(use_color){cbuf=ring_copy(color_ptr+(size_t)first*cs,cs,color_size,count);vs=vel;cs=color_size;}
        else vs=vel;
        if(!vbuf||(use_color&&!cbuf))return;   /* reserva fallida: no enviar punteros inválidos a la GPU */
        draw_first=0;
    }

    C3D_AttrInfo *ai=C3D_GetAttrInfo(); AttrInfo_Init(ai);
    AttrInfo_AddLoader(ai,0,GPU_FLOAT,vertex_size);
    C3D_BufInfo *bi=C3D_GetBufInfo(); BufInfo_Init(bi);
    BufInfo_Add(bi,vbuf,vs,1,0x0);
    if(use_color){
        AttrInfo_AddLoader(ai,1,GPU_UNSIGNED_BYTE,color_size);
        BufInfo_Add(bi,cbuf,cs,1,0x1);
    } else {
        AttrInfo_AddFixed(ai,1);
        C3D_FVec *f=C3D_FixedAttribGetWritePtr(1);
        /* el shader divide el color entre 255 */
        f->x=current_color[0]*255.0f; f->y=current_color[1]*255.0f;
        f->z=current_color[2]*255.0f; f->w=current_color[3]*255.0f;
    }
    upload_mvp();
    C3D_DrawArrays(GPU_TRIANGLES,draw_first,count);
}

static float clear_color[4]={0,0,0,1};
extern void brick3ds_clear(float r,float g,float b,float a,unsigned mask);
void glClearColor(float r,float g,float b,float a){clear_color[0]=r;clear_color[1]=g;clear_color[2]=b;clear_color[3]=a;}
void glClear(unsigned int mask){brick3ds_clear(clear_color[0],clear_color[1],clear_color[2],clear_color[3],mask);}
