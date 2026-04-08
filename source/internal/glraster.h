
/************************************************************************\

    TinyGL - CPU implementation of OpenGL
    Copyright (c) 1998-2026 Jango73

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.


    Rasterizer

\************************************************************************/

#ifndef GLRASTER_H_INCLUDED
#define GLRASTER_H_INCLUDED

/**********************************************************************************************/

#include "glmain.h"

/**********************************************************************************************/

// Vertex structure used by rasterizers

typedef struct {
    GLdouble x; // X coordinate
    GLdouble y; // Y coordinate
    GLdouble z; // Z coordinate
    GLdouble r; // Red color component
    GLdouble g; // Green color component
    GLdouble b; // Blue color component
    GLdouble a; // Alpha component
    GLdouble u; // X texture coordinate
    GLdouble v; // Y texture coordinate
    GLdouble w; // 1/z
} GLrastervertex;

/**********************************************************************************************/

extern GLI32 GlClip[2][2];

/**********************************************************************************************/

void glRasterTriangle();

void glPolygonToClipping(LPGLPOLYGON, GLMATRIX4D);
void glPolygonToViewport(LPGLPOLYGON, GLI32[4]);
void glLightPolygon(LPGLPOLYGON, LPGLRENDERBLOCK);

GLboolean glRasterSetContextAndBlock(LPGLRENDERCONTEXT, LPGLRENDERBLOCK);
void glRasterizePolygon(LPGLRENDERCONTEXT, LPGLRENDERBLOCK, LPGLPOLYGON);

void glRasterizeBlock(LPGLRENDERCONTEXT Context, LPGLRENDERBLOCK Block);
GLboolean glRasterSetContext(LPGLRENDERCONTEXT Context);

/**********************************************************************************************/

EXPORT GLboolean APIENTRY glSetupSingleClipPlane(LPGLCLIPPLANE, GLdouble *,
                                                 GLdouble *);

EXPORT GLboolean APIENTRY glClipPolygonToSinglePlane(LPGLCLIPPLANE, GLI32 *,
                                                     LPGLVERTEX, GLI32 *,
                                                     LPGLVERTEX);

EXPORT GLboolean APIENTRY glClipPolygon(LPGLCLIPPLANE, GLI32, GLI32 *,
                                        LPGLVERTEX, GLI32 *, LPGLVERTEX);

/**********************************************************************************************/

#endif // GLRASTER_H_INCLUDED
