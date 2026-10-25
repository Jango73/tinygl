#ifndef TINYGL_DEMO_H
#define TINYGL_DEMO_H

#include "../../include/tinygl.h"

#define TINYGL_DEMO_MANY_CUBES_COUNT 200

/************************************************************************/

typedef enum TINYGL_DEMO_VIEW_MODE {
    TINYGL_DEMO_VIEW_MODE_UNLIT_CUBE = 1,
    TINYGL_DEMO_VIEW_MODE_LIT_CUBE = 2,
    TINYGL_DEMO_VIEW_MODE_MANY_CUBES = 3
} TINYGL_DEMO_VIEW_MODE;

/************************************************************************/

typedef struct TINYGL_DEMO_CUBE {
    GLfloat BasePositionX;
    GLfloat BasePositionY;
    GLfloat BasePositionZ;
    GLfloat NoisePhaseX;
    GLfloat NoisePhaseY;
    GLfloat NoisePhaseZ;
    GLfloat NoiseFrequencyX;
    GLfloat NoiseFrequencyY;
    GLfloat NoiseFrequencyZ;
    GLfloat NoiseAmplitudeX;
    GLfloat NoiseAmplitudeY;
    GLfloat NoiseAmplitudeZ;
    GLfloat RotationX;
    GLfloat RotationY;
    GLfloat SpeedX;
    GLfloat SpeedY;
    GLfloat Scale;
    GLfloat Red;
    GLfloat Green;
    GLfloat Blue;
} TINYGL_DEMO_CUBE;

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
    GLfloat ElapsedSeconds;
    GLfloat FpsAccumulator;
    int FpsFrameCount;
    int CurrentFps;
    TINYGL_DEMO_CUBE Cubes[TINYGL_DEMO_MANY_CUBES_COUNT];
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
