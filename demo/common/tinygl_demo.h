#ifndef TINYGL_DEMO_H
#define TINYGL_DEMO_H

#include "../../include/tinygl.h"

/************************************************************************/

typedef enum TINYGL_DEMO_VIEW_MODE {
    TINYGL_DEMO_VIEW_MODE_UNLIT_CUBE = 1,
    TINYGL_DEMO_VIEW_MODE_LIT_CUBE = 2
} TINYGL_DEMO_VIEW_MODE;

/************************************************************************/

typedef struct TINYGL_DEMO_APP {
    TGLContext Context;
    TGL_SURFACE_DESC Surface;
    unsigned char *ColorBuffer;
    GLsizei Width;
    GLsizei Height;
    TINYGL_DEMO_VIEW_MODE ViewMode;
    GLfloat RotationX;
    GLfloat RotationY;
} TINYGL_DEMO_APP;

/************************************************************************/

int tinyglDemoInitialize(TINYGL_DEMO_APP *App, GLsizei Width, GLsizei Height);

/************************************************************************/

void tinyglDemoShutdown(TINYGL_DEMO_APP *App);

/************************************************************************/

int tinyglDemoResize(TINYGL_DEMO_APP *App, GLsizei Width, GLsizei Height);

/************************************************************************/

int tinyglDemoRenderFrame(TINYGL_DEMO_APP *App, GLfloat DeltaSeconds);

/************************************************************************/

void tinyglDemoSetViewMode(TINYGL_DEMO_APP *App, TINYGL_DEMO_VIEW_MODE ViewMode);

/************************************************************************/

TINYGL_DEMO_VIEW_MODE tinyglDemoGetViewMode(const TINYGL_DEMO_APP *App);

/************************************************************************/

const void *tinyglDemoGetPixels(const TINYGL_DEMO_APP *App);

/************************************************************************/

GLsizei tinyglDemoGetPitch(const TINYGL_DEMO_APP *App);

/************************************************************************/

#endif
