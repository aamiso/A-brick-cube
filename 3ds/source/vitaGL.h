#ifndef BRICK3DS_VITAGL_COMPAT_H
#define BRICK3DS_VITAGL_COMPAT_H

#include <stddef.h>

#define GL_FLOAT 0x1406
#define GL_UNSIGNED_BYTE 0x1401
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_VERTEX_ARRAY 0x8074
#define GL_COLOR_ARRAY 0x8076
#define GL_TRIANGLES 0x0004
#define GL_PROJECTION 0x1701
#define GL_MODELVIEW 0x1700
#define GL_DEPTH_TEST 0x0B71
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_FALSE 0
#define GL_TRUE 1

void glMatrixMode(unsigned int mode);
void glLoadIdentity(void);
void glPushMatrix(void);
void glPopMatrix(void);
void glTranslatef(float x,float y,float z);
void glRotatef(float angle,float x,float y,float z);
void glScalef(float x,float y,float z);
void glFrustum(double l,double r,double b,double t,double n,double f);
void glOrtho(double l,double r,double b,double t,double n,double f);
void glViewport(int x,int y,int w,int h);
void glEnable(unsigned int cap);
void glDisable(unsigned int cap);
void glEnableClientState(unsigned int cap);
void glDisableClientState(unsigned int cap);
void glVertexPointer(int size,unsigned int type,int stride,const void *ptr);
void glColorPointer(int size,unsigned int type,int stride,const void *ptr);
void glColor3f(float r,float g,float b);
void glDrawArrays(unsigned int mode,int first,int count);
void glGenBuffers(int n,unsigned int *buffers);
void glBindBuffer(unsigned int target,unsigned int buffer);
void glBufferData(unsigned int target,long size,const void *data,unsigned int usage);
void glDeleteBuffers(int n,const unsigned int *buffers);
void glClearColor(float r,float g,float b,float a);
void glClear(unsigned int mask);

#endif
