
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


    Main

\************************************************************************/

#ifndef GLMAIN_H_INCLUDED
#define GLMAIN_H_INCLUDED

/*****************************************************************************************************/

#include "../../include/tinygl.h"

/*****************************************************************************************************/

#define X 0
#define Y 1
#define Z 2
#define W 3

#define R 0
#define G 1
#define B 2
#define A 3

#define GL_MAX_CONTEXT 64
#define GL_MAX_USER_PLANES 8
#define GL_MAX_USER_LIGHTS 32

#define GL_MAX_POLY_VERTEX 200

#define GL_MATRIX_MODELVIEW 0
#define GL_MATRIX_PROJECTION 1
#define GL_MATRIX_TEXTURE 2

/*****************************************************************************************************/

typedef unsigned char GLU8;
typedef unsigned short GLU16;
typedef unsigned int GLU32;

typedef signed char GLI8;
typedef signed short GLI16;
typedef int GLI32;

/*****************************************************************************************************/

// Rendering functions and vars

typedef struct tag_GLRENDERFLAGS {
    GLboolean Alpha;
    GLboolean Blend;
    GLboolean CullFace;
    GLboolean Depth;
    GLboolean Dither;
    GLboolean EdgeFlag;
    GLboolean Fog;
    GLboolean Lighting;
    GLboolean LineSmooth;
    GLboolean LineStipple;
    GLboolean LogicOp;
    GLboolean Map1Color4;
    GLboolean Map1Index;
    GLboolean Normalize;
    GLboolean PointSmooth;
    GLboolean PolygonSmooth;
    GLboolean PolygonStipple;
    GLboolean Scissor;
    GLboolean Stencil;
    GLboolean Texture1D;
    GLboolean Texture2D;
    GLboolean TextureGenQ;
    GLboolean TextureGenR;
    GLboolean TextureGenS;
    GLboolean TextureGenT;
} GLRENDERFLAGS, *LPGLRENDERFLAGS;

typedef struct tag_GLRENDERFUNCTIONS {
    GLenum RenderMode;

    GLenum BlendFuncSFactor;
    GLenum BlendFuncDFactor;

    GLenum AlphaFunc;
    GLclampf AlphaRef;

    GLboolean RMask;
    GLboolean GMask;
    GLboolean BMask;
    GLboolean AMask;

    GLboolean DepthMask;
    GLenum DepthFunc;
    GLclampd DepthRangeNear;
    GLclampd DepthRangeFar;
    GLdouble ClearDepth;

    GLenum FogMode;
    GLdouble FogDensity;
    GLdouble FogStart;
    GLdouble FogEnd;
    GLdouble FogIndex;
    GLdouble FogColor[4];
} GLRENDERFUNCTIONS, *LPGLRENDERFUNCTIONS;

/*****************************************************************************************************/

typedef struct tag_GLCLIPPLANE {
    GLboolean On;
    GLdouble Position[4];
    GLdouble Normal[4];
    GLdouble Distance;
} GLCLIPPLANE, *LPGLCLIPPLANE;

/*****************************************************************************************************/

typedef struct tag_GLLIGHT {
    GLboolean On;
    GLdouble Ambient[4];
    GLdouble Diffuse[4];
    GLdouble Specular[4];
    GLdouble Position[4];
    GLdouble Direction[3];
    GLdouble SpotExponent;
    GLdouble SpotCutOff;
    GLdouble ConstantAtten;
    GLdouble LinearAtten;
    GLdouble QuadraticAtten;
} GLLIGHT, *LPGLLIGHT;

/*****************************************************************************************************/

typedef struct tag_GLXFORMDATA {
    GLenum MatrixMode;
    GLdouble NMatrix[3][4][4];
    GLdouble IMatrix[3][4][4];
    GLCLIPPLANE Plane[6];
} GLXFORMDATA, *LPGLXFORMDATA;

/*****************************************************************************************************/

typedef struct tag_GLVERTEX {
    GLdouble World[4];  // World coordinates (XYZW)
    GLdouble Eye[4];    // Eye coordinates (XYZW)
    GLdouble Screen[4]; // Screen coordinates (XYZW)
    GLdouble Color[4];  // Color (RGBA)
    GLdouble Normal[4]; // Normal vector
    GLdouble Tex[2];    // Texture offsets over W
} GLVERTEX, *LPGLVERTEX;

/*****************************************************************************************************/

typedef struct tag_GLMATERIAL {
    GLdouble Ambient[4];
    GLdouble Diffuse[4];
    GLdouble Specular[4];
    GLdouble Emission[4];
    GLdouble Transparency[4];
    GLdouble Shininess;
} GLMATERIAL, *LPGLMATERIAL;

/*****************************************************************************************************/

typedef struct tag_GLPOLYGON {
    GLI32 NumVertex;
    LPGLVERTEX Vertex;
    GLMATERIAL FrontFace;
    GLMATERIAL BackFace;
} GLPOLYGON, *LPGLPOLYGON;

/*****************************************************************************************************/

typedef struct tag_GLRENDERBLOCK {
    GLRENDERFLAGS RenderFlag;
    GLRENDERFUNCTIONS RenderFunc;
    GLXFORMDATA XForm;
    GLCLIPPLANE UserPlane[GL_MAX_USER_PLANES];
    GLLIGHT Light[GL_MAX_USER_LIGHTS];
    GLLIST PolygonList;
} GLRENDERBLOCK, *LPGLRENDERBLOCK;

/*****************************************************************************************************/

struct TGL_CONTEXT {
    GLU32 Id;

    GLI32 ViewPort[4];
    GLI32 Width;
    GLI32 Height;
    GLboolean HasDepthBuffer;
    GLboolean HasColorBuffer;
    TGL_SURFACE_MODE SurfaceMode;
    TGL_PIXEL_FORMAT SurfacePixelFormat;
    TGL_SURFACE_DESC Surface;
    TGL_SURFACE_DESC InternalSurface;
    TGL_BRIDGE_CALLBACKS BridgeCallbacks;

    GLU8 *ColorBuffer;
    GLU32 ColorBufferSize;

    GLU8 *DepthBuffer;
    GLU32 DepthBufferSize;

    GLU8 *AuxBuffer[4];

    // Current rendering material for this context
    GLRENDERBLOCK Block;
    GLPOLYGON IPolygon;
    GLVERTEX IVertex;

    // Pending rendering lists
    GLU32 NumLists;
    GLLIST RenderList;
};

typedef struct TGL_CONTEXT GLRENDERCONTEXT;
typedef GLRENDERCONTEXT *LPGLRENDERCONTEXT;

/*****************************************************************************************************/

// Macro functions

#define CLAMP(a, b, c)                                                         \
    {                                                                          \
        if (a < b)                                                             \
            a = b;                                                             \
        else if (a > c)                                                        \
            a = c;                                                             \
    }

#define RGB555_TO_RGB888_R(rgb) (((rgb & 0x7C00) >> 10) << 3)
#define RGB555_TO_RGB888_G(rgb) (((rgb & 0x03E0) >> 5) << 3)
#define RGB555_TO_RGB888_B(rgb) (((rgb & 0x001F) >> 0) << 3)

#define RGB888_TO_RGB555(r, g, b)                                              \
    ((((GLI32)r >> 3) << 10) | (((GLI32)g >> 3) << 5) | (((GLI32)b >> 3) << 0))

#define RGB_TO_RGB888(r, g, b)                                                 \
    (((GLI32)r << 16) | ((GLI32)g << 8) | ((GLI32)b << 0))

/*****************************************************************************************************/

extern LPGLRENDERCONTEXT GlRenderContext[GL_MAX_CONTEXT];
extern GLI32 GlNumContext;
extern LPGLRENDERCONTEXT GlCurrentContext;

/*****************************************************************************************************/

GLvoid *glMalloc(GLsizei Size);
void glFree(GLvoid *Pointer);

GLboolean glCreatePolygon(LPGLPOLYGON poly, GLenum numvert);
GLboolean glDestroyPolygon(LPGLPOLYGON poly);
LPGLPOLYGON glAllocatePolygon(GLenum numvert);
void glSetErrorCode(GLenum ErrorCode);
GLenum glGetErrorCode(void);
void glSetLibraryError(TGL_ERROR ErrorCode);
void glClearErrorCode(void);
void glClearLibraryError(void);

/*****************************************************************************************************/

#endif // GLMAIN_H_INCLUDED
