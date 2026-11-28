#include "vitaGL.h"
#include <3ds.h>
#include <citro3d.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Adaptador de la API fija usada por el juego Vita. Las matrices se calculan
   en CPU y los vértices ya transformados se envían al shader PICA de 3DS. */
extern void brick3ds_set_identity_uniforms(void);

#define STACK_MAX 24
#define BUFFER_MAX 2048
typedef struct { float m[16]; } Mat4;
typedef struct { void *data; size_t size; } Buffer;
static Mat4 projection, modelview, *current;
static Mat4 stack[STACK_MAX];
static int stack_top;
static Buffer buffers[BUFFER_MAX];
static unsigned next_buffer=1, bound_buffer=0;
static const float *vertex_ptr;
static const unsigned char *color_ptr;
static int vertex_size=3, color_size=4, vertex_stride=0, color_stride=0;
static unsigned vertex_type=GL_FLOAT, color_type=GL_UNSIGNED_BYTE;
static int vertex_enabled=0, color_enabled=0, depth_enabled=1;
static float current_color[4]={1,1,1,1};
static int batch_active=0, batch_vertices=0;
#define BATCH_VERTEX_LIMIT 4096
static void flush_batch(void){if(batch_active){C3D_ImmDrawEnd();batch_active=0;batch_vertices=0;}}

static void identity(Mat4 *m){memset(m,0,sizeof(*m));m->m[0]=m->m[5]=m->m[10]=m->m[15]=1.0f;}
static void multiply(Mat4 *out,const Mat4 *a,const Mat4 *b){
    Mat4 t;
    for(int c=0;c<4;c++)for(int r=0;r<4;r++){
        t.m[c*4+r]=0;
        for(int k=0;k<4;k++)t.m[c*4+r]+=a->m[k*4+r]*b->m[c*4+k];
    }
    *out=t;
}
static void apply(const Mat4 *m,float x,float y,float z,float w,float *o){
    o[0]=m->m[0]*x+m->m[4]*y+m->m[8]*z+m->m[12]*w;
    o[1]=m->m[1]*x+m->m[5]*y+m->m[9]*z+m->m[13]*w;
    o[2]=m->m[2]*x+m->m[6]*y+m->m[10]*z+m->m[14]*w;
    o[3]=m->m[3]*x+m->m[7]*y+m->m[11]*z+m->m[15]*w;
}
static void multiply_current(const Mat4 *rhs){Mat4 t;multiply(&t,current,rhs);*current=t;}
static const void *resolve_pointer(const void *p){
    if(!bound_buffer)return p;
    uintptr_t off=(uintptr_t)p;
    Buffer *b=&buffers[bound_buffer%BUFFER_MAX];
    if(!b->data || off>=b->size)return NULL;
    return (const unsigned char*)b->data+off;
}
void glMatrixMode(unsigned int mode){current=(mode==GL_PROJECTION)?&projection:&modelview;}
void glLoadIdentity(void){if(!current){current=&modelview;}identity(current);}
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
void glViewport(int x,int y,int w,int h){(void)x;(void)y;(void)w;(void)h;}
void glEnable(unsigned int cap){if(cap==GL_DEPTH_TEST){flush_batch();depth_enabled=1;C3D_DepthTest(true,GPU_GEQUAL,GPU_WRITE_ALL);}}
void glDisable(unsigned int cap){if(cap==GL_DEPTH_TEST){flush_batch();depth_enabled=0;C3D_DepthTest(false,GPU_ALWAYS,GPU_WRITE_ALL);}}
void glEnableClientState(unsigned int cap){if(cap==GL_VERTEX_ARRAY)vertex_enabled=1;if(cap==GL_COLOR_ARRAY)color_enabled=1;}
void glDisableClientState(unsigned int cap){if(cap==GL_VERTEX_ARRAY)vertex_enabled=0;if(cap==GL_COLOR_ARRAY)color_enabled=0;}
void glVertexPointer(int size,unsigned int type,int stride,const void *ptr){vertex_size=size;vertex_type=type;vertex_stride=stride;vertex_ptr=(const float*)resolve_pointer(ptr);}
void glColorPointer(int size,unsigned int type,int stride,const void *ptr){color_size=size;color_type=type;color_stride=stride;color_ptr=(const unsigned char*)resolve_pointer(ptr);}
void glColor3f(float r,float g,float b){current_color[0]=r;current_color[1]=g;current_color[2]=b;current_color[3]=1;}
void glGenBuffers(int n,unsigned int *ids){for(int i=0;i<n;i++){unsigned id=next_buffer++;if(id>=BUFFER_MAX)id=(id% (BUFFER_MAX-1))+1;ids[i]=id;}}
void glBindBuffer(unsigned int target,unsigned int id){if(target==GL_ARRAY_BUFFER)bound_buffer=id;}
void glBufferData(unsigned int target,long size,const void *data,unsigned int usage){
    (void)usage;if(target!=GL_ARRAY_BUFFER||!bound_buffer||bound_buffer>=BUFFER_MAX||size<0)return;
    Buffer *b=&buffers[bound_buffer];free(b->data);b->data=size?malloc((size_t)size):NULL;b->size=b->data?(size_t)size:0;if(b->data&&data)memcpy(b->data,data,(size_t)size);
}
void glDeleteBuffers(int n,const unsigned int *ids){for(int i=0;i<n;i++){unsigned id=ids[i];if(id<BUFFER_MAX){free(buffers[id].data);buffers[id].data=NULL;buffers[id].size=0;}if(bound_buffer==id)bound_buffer=0;}}
void glDrawArrays(unsigned int mode,int first,int count){
    if(mode!=GL_TRIANGLES||!vertex_enabled||!vertex_ptr||count<=0)return;
    Mat4 mv,mvp;mv=modelview;multiply(&mvp,&projection,&mv);
    int vs=vertex_stride?vertex_stride:vertex_size*(int)sizeof(float);
    int cs=color_stride?color_stride:(color_type==GL_UNSIGNED_BYTE?color_size:(int)(color_size*sizeof(float)));
    for(int i=0;i<count;i++){
        if(!batch_active){brick3ds_set_identity_uniforms();C3D_ImmDrawBegin(GPU_TRIANGLES);batch_active=1;batch_vertices=0;}
        const unsigned char *vp=(const unsigned char*)vertex_ptr+(size_t)(first+i)*vs;
        float x=((const float*)vp)[0],y=((const float*)vp)[1],z=vertex_size>2?((const float*)vp)[2]:0.0f;
        float eye[4],clip[4];apply(&mv,x,y,z,1,eye);apply(&projection,eye[0],eye[1],eye[2],eye[3],clip);
        float rr=current_color[0],gg=current_color[1],bb=current_color[2],aa=current_color[3];
        if(color_enabled&&color_ptr){
            const unsigned char *cp=color_ptr+(size_t)(first+i)*cs;
            if(color_type==GL_UNSIGNED_BYTE){rr=cp[0]/255.0f;gg=cp[1]/255.0f;bb=cp[2]/255.0f;aa=color_size>3?cp[3]/255.0f:1.0f;}
            else {const float *cf=(const float*)cp;rr=cf[0];gg=cf[1];bb=cf[2];aa=color_size>3?cf[3]:1.0f;}
        }
        C3D_ImmSendAttrib(clip[0],clip[1],clip[2],clip[3]);
        C3D_ImmSendAttrib(rr,gg,bb,aa);
        if(++batch_vertices>=BATCH_VERTEX_LIMIT)flush_batch();
    }
}
static float clear_color[4]={0,0,0,1};
extern void brick3ds_clear(float r,float g,float b,float a,unsigned mask);
void glClearColor(float r,float g,float b,float a){clear_color[0]=r;clear_color[1]=g;clear_color[2]=b;clear_color[3]=a;}
void glClear(unsigned int mask){flush_batch();brick3ds_clear(clear_color[0],clear_color[1],clear_color[2],clear_color[3],mask);(void)depth_enabled;}

void brick3ds_flush(void){flush_batch();}
