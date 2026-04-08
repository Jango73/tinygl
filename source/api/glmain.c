
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


    List

\************************************************************************/

#include "../../include/tinygl.h"

#include "../internal/glstd.h"
#include "../internal/gllist.h"
#include "../internal/glmain.h"
#include "../internal/glraster.h"
#include "../internal/glvector.h"

/**********************************************************************************************/

// Global variables

GLI32 GlNumContext;
LPGLRENDERCONTEXT GlRenderContext[GL_MAX_CONTEXT];
LPGLRENDERCONTEXT GlCurrentContext;

GLI32 GlNumAllocs;

/**********************************************************************************************/

GLboolean GlInputPrimitive;
GLenum GlPrimitiveType;
GLenum GlNumVertProcessed;

/**********************************************************************************************/

// Variables local to this module

static GLenum GlCurrentErrorCode;
static TGL_ERROR GlLastError;
static GLboolean GlLibraryInitialized;

/**********************************************************************************************/

/**
 * @brief Get the maximum depth of one matrix stack.
 * @param MatrixMode The internal matrix mode.
 * @return The maximum number of entries for that stack.
 */
static GLI32 glGetMatrixStackMaxDepth(GLenum MatrixMode) {
    switch (MatrixMode) {
    case GL_MATRIX_MODELVIEW:
        return GL_MODELVIEW_MATRIX_STACK_MAX;

    case GL_MATRIX_PROJECTION:
        return GL_PROJECTION_MATRIX_STACK_MAX;

    case GL_MATRIX_TEXTURE:
        return GL_TEXTURE_MATRIX_STACK_MAX;

    default:
        return 0;
    }
}

/**********************************************************************************************/

/**
 * @brief Copy one 4x4 matrix.
 * @param Destination The destination matrix.
 * @param Source The source matrix.
 */
static void glCopyMatrix4d(GLdouble Destination[4][4], GLdouble Source[4][4]) {
    memcpy(Destination, Source, sizeof(GLdouble) * 16);
}

/**********************************************************************************************/

/**
 * @brief Copy a linear double matrix into a 4x4 matrix.
 * @param Destination The destination matrix.
 * @param Source The source values in linear order.
 */
static void glCopyLinearToMatrix4d(GLdouble Destination[4][4],
                                   const GLdouble *Source) {
    memcpy(Destination, Source, sizeof(GLdouble) * 16);
}

/**********************************************************************************************/

/**
 * @brief Copy a linear float matrix into a 4x4 double matrix.
 * @param Destination The destination matrix.
 * @param Source The source values in linear order.
 */
static void glCopyLinearFloatToMatrix4d(GLdouble Destination[4][4],
                                        const GLfloat *Source) {
    GLI32 Index;
    GLdouble *DestinationValue;

    DestinationValue = &(Destination[0][0]);

    for (Index = 0; Index < 16; Index++)
        DestinationValue[Index] = Source[Index];
}

/**********************************************************************************************/

/**
 * @brief Return the active normal matrix.
 * @param Context The render context.
 * @return A pointer to the active matrix.
 */
static GLMATRIX4D *glGetContextCurrentNMatrix(LPGLRENDERCONTEXT Context) {
    return &(Context->Block.XForm.NMatrix[Context->Block.XForm.MatrixMode]);
}

/**********************************************************************************************/

/**
 * @brief Return the active inverse matrix.
 * @param Context The render context.
 * @return A pointer to the active inverse matrix.
 */
static GLMATRIX4D *glGetContextCurrentIMatrix(LPGLRENDERCONTEXT Context) {
    return &(Context->Block.XForm.IMatrix[Context->Block.XForm.MatrixMode]);
}

/**********************************************************************************************/

/**
 * @brief Synchronize the active matrix into the current stack entry.
 * @param Context The render context.
 */
static void glSyncCurrentMatrixStackEntry(LPGLRENDERCONTEXT Context) {
    GLenum MatrixMode;
    GLI32 StackIndex;

    MatrixMode = Context->Block.XForm.MatrixMode;
    StackIndex = Context->Block.XForm.MatrixStackDepth[MatrixMode] - 1;

    if (StackIndex < 0)
        return;

    glCopyMatrix4d(Context->Block.XForm.NMatrixStack[MatrixMode][StackIndex],
                   Context->Block.XForm.NMatrix[MatrixMode]);
    glCopyMatrix4d(Context->Block.XForm.IMatrixStack[MatrixMode][StackIndex],
                   Context->Block.XForm.IMatrix[MatrixMode]);
}

/**********************************************************************************************/

static GLboolean glCheckState() {
    if (GlCurrentContext == NULL)
        return FALSE;
    if (GlInputPrimitive == TRUE) {
        glSetErrorCode(GL_INVALID_OPERATION);
        return FALSE;
    }
    return TRUE;
}

/**********************************************************************************************/

static TGL_ERROR glMapErrorCodeToLibraryError(GLenum ErrorCode) {
    switch (ErrorCode) {
    case GL_NO_ERROR:
        return TGL_ERROR_NONE;
    case GL_INVALID_VALUE:
        return TGL_ERROR_INVALID_ARGUMENT;
    case GL_INVALID_OPERATION:
        return TGL_ERROR_INVALID_CONTEXT;
    case GL_OUT_OF_MEMORY:
        return TGL_ERROR_OUT_OF_MEMORY;
    case GL_INVALID_ENUM:
    default:
        return TGL_ERROR_INTERNAL;
    }
}

/**********************************************************************************************/

static void glNotifyLibraryError(TGL_ERROR ErrorCode) {
    if (GlCurrentContext == NULL)
        return;

    if (GlCurrentContext->BridgeCallbacks.NotifyError == NULL)
        return;

    GlCurrentContext->BridgeCallbacks.NotifyError(
        GlCurrentContext->BridgeCallbacks.UserData, ErrorCode);
}

/**********************************************************************************************/

void glSetErrorCode(GLenum ErrorCode) {
    GlCurrentErrorCode = ErrorCode;
    GlLastError = glMapErrorCodeToLibraryError(ErrorCode);
    glNotifyLibraryError(GlLastError);
}

/**********************************************************************************************/

GLenum glGetErrorCode(void) { return GlCurrentErrorCode; }

/**********************************************************************************************/

void glSetLibraryError(TGL_ERROR ErrorCode) {
    GlLastError = ErrorCode;
    glNotifyLibraryError(ErrorCode);
}

/**********************************************************************************************/

void glClearErrorCode(void) { GlCurrentErrorCode = GL_NO_ERROR; }

/**********************************************************************************************/

void glClearLibraryError(void) { GlLastError = TGL_ERROR_NONE; }

/**********************************************************************************************/

GLvoid *glMalloc(GLsizei Size) {
    GLvoid *Pointer;
    Pointer = (GLvoid *)malloc(Size);
    if (Pointer)
        GlNumAllocs++;
    return Pointer;
}

/**********************************************************************************************/

void glFree(GLvoid *Pointer) {
    if (Pointer) {
        free(Pointer);
        GlNumAllocs--;
    }
}

/**********************************************************************************************/

void glDestroyPolygonListItem(GLvoid *Data) {
    LPGLPOLYGON Poly = (LPGLPOLYGON)Data;
    glFree(Poly->Vertex);
    glFree(Poly);
}

/**********************************************************************************************/

void glDestroyRenderListItem(GLvoid *Data) {
    LPGLRENDERBLOCK Block = (LPGLRENDERBLOCK)Data;
    glList_Destroy(&(Block->PolygonList));
    glFree(Block);
}

/**********************************************************************************************/

GLboolean glCreatePolygon(LPGLPOLYGON poly, GLenum numvert) {
    LPGLVERTEX v;

    if (poly == NULL)
        return FALSE;

    poly->NumVertex = 0;
    poly->Vertex = NULL;

    v = (LPGLVERTEX)glMalloc(sizeof(GLVERTEX) * numvert);
    if (v == NULL)
        return FALSE;

    poly->NumVertex = numvert;
    poly->Vertex = v;

    return TRUE;
}

/**********************************************************************************************/

GLboolean glDestroyPolygon(LPGLPOLYGON poly) {
    if (poly == NULL)
        return FALSE;

    poly->NumVertex = 0;
    glFree(poly->Vertex);
    poly->Vertex = NULL;

    return TRUE;
}

/**********************************************************************************************/

LPGLPOLYGON glAllocatePolygon(GLenum numvert) {
    LPGLPOLYGON poly;

    poly = (LPGLPOLYGON)glMalloc(sizeof(GLPOLYGON));
    if (poly == NULL)
        return NULL;

    if (glCreatePolygon(poly, numvert) == FALSE) {
        glFree(poly);
        return NULL;
    }

    return poly;
}

/**********************************************************************************************/

void glComputeFrustum(LPGLRENDERCONTEXT Context, LPGLRENDERBLOCK Block) {
    GLdouble Pos[4];
    GLdouble Nor[4];
    GLdouble Box[8][4];
    GLint Idx[6][4];
    GLI32 c;

    GLdouble MINX = -0.8;
    GLdouble MAXX = 0.8;
    GLdouble MINY = -0.8;
    GLdouble MAXY = 0.8;
    GLdouble MINZ = 0.8;
    GLdouble MAXZ = 0.8;

    MINX *= (GLdouble)Context->ViewPort[2] / 2.0;
    MAXX *= (GLdouble)Context->ViewPort[2] / 2.0;

    MINY *= (GLdouble)Context->ViewPort[3] / 2.0;
    MAXY *= (GLdouble)Context->ViewPort[3] / 2.0;

    Box[0][X] = MINX;
    Box[0][Y] = MINY;
    Box[0][Z] = MINZ;
    Box[0][W] = 1.0;
    Box[1][X] = MAXX;
    Box[1][Y] = MINY;
    Box[1][Z] = MINZ;
    Box[1][W] = 1.0;
    Box[2][X] = MINX;
    Box[2][Y] = MINY;
    Box[2][Z] = MAXZ;
    Box[2][W] = 1.0;
    Box[3][X] = MAXX;
    Box[3][Y] = MINY;
    Box[3][Z] = MAXZ;
    Box[3][W] = 1.0;
    Box[4][X] = MINX;
    Box[4][Y] = MAXY;
    Box[4][Z] = MINZ;
    Box[4][W] = 1.0;
    Box[5][X] = MAXX;
    Box[5][Y] = MAXY;
    Box[5][Z] = MINZ;
    Box[5][W] = 1.0;
    Box[6][X] = MINX;
    Box[6][Y] = MAXY;
    Box[6][Z] = MAXZ;
    Box[6][W] = 1.0;
    Box[7][X] = MAXX;
    Box[7][Y] = MAXY;
    Box[7][Z] = MAXZ;
    Box[7][W] = 1.0;

    // Left plane
    Idx[0][0] = 0;
    Idx[0][1] = 2;
    Idx[0][2] = 6;
    Idx[0][3] = 4;
    // Idx[0][0]=4; Idx[0][1]=6; Idx[0][2]=2; Idx[0][3]=0;

    // Right plane
    Idx[1][0] = 3;
    Idx[1][1] = 1;
    Idx[1][2] = 5;
    Idx[1][3] = 7;
    // Idx[1][0]=7; Idx[1][1]=5; Idx[1][2]=1; Idx[1][3]=3;

    // Bottom plane
    Idx[2][0] = 1;
    Idx[2][1] = 3;
    Idx[2][2] = 2;
    Idx[2][3] = 0;
    // Idx[2][0]=0; Idx[2][1]=2; Idx[2][2]=3; Idx[2][3]=1;

    // Top plane
    Idx[3][0] = 4;
    Idx[3][1] = 6;
    Idx[3][2] = 7;
    Idx[3][3] = 5;
    // Idx[3][0]=5; Idx[3][1]=7; Idx[3][2]=6; Idx[3][3]=4;

    // Near plane
    Idx[4][0] = 0;
    Idx[4][1] = 4;
    Idx[4][2] = 5;
    Idx[4][3] = 1;
    // Idx[4][0]=1; Idx[4][1]=5; Idx[4][2]=4; Idx[4][3]=0;

    // Far plane
    Idx[5][0] = 3;
    Idx[5][1] = 7;
    Idx[5][2] = 6;
    Idx[5][3] = 2;
    // Idx[5][0]=2; Idx[5][1]=6; Idx[5][2]=7; Idx[5][3]=3;

    /*
    for (c=0; c<8; c++)
    {
      glMatrix4dTransVector4d(Box[c], Box[c],
    Block->XForm.NMatrix[GL_MATRIX_PROJECTION]);

      glVector4dToVector3d(Box[c]);

      if (Box[c][X]!=0.0) Box[c][X]=1.0/Box[c][X];
      if (Box[c][Y]!=0.0) Box[c][Y]=1.0/Box[c][Y];
      if (Box[c][Z]!=0.0) Box[c][Z]=1.0/Box[c][Z];
      if (Box[c][W]!=0.0) Box[c][W]=1.0/Box[c][W];

      // Box[c][X]*=Box[c][W];
      // Box[c][Y]*=Box[c][W];
      // Box[c][Z]*=Box[c][W];

      // Box[c][X]*=(GLdouble)Context->ViewPort[2]/2.0;
      // Box[c][Y]*=(GLdouble)Context->ViewPort[3]/2.0;
    }
    */

    for (c = 0; c < 6; c++) {
        Pos[X] = (Box[Idx[c][0]][X] + Box[Idx[c][1]][X] + Box[Idx[c][2]][X] +
                  Box[Idx[c][3]][X]) /
                 4.0;
        Pos[Y] = (Box[Idx[c][0]][Y] + Box[Idx[c][1]][Y] + Box[Idx[c][2]][Y] +
                  Box[Idx[c][3]][Y]) /
                 4.0;
        Pos[Z] = (Box[Idx[c][0]][Z] + Box[Idx[c][1]][Z] + Box[Idx[c][2]][Z] +
                  Box[Idx[c][3]][Z]) /
                 4.0;
        Pos[W] = (Box[Idx[c][0]][W] + Box[Idx[c][1]][W] + Box[Idx[c][2]][W] +
                  Box[Idx[c][3]][W]) /
                 4.0;

        glVector3dTriangleNormal(Nor, Box[Idx[c][0]], Box[Idx[c][1]],
                                 Box[Idx[c][2]]);

        glSetupSingleClipPlane(&(Block->XForm.Plane[c]), Pos, Nor);
    }
}

/**********************************************************************************************/

static GLboolean glInitLibrary(void) {
    GLU32 c;

    GlNumAllocs = 0;
    GlNumContext = 0;
    GlCurrentContext = NULL;
    GlInputPrimitive = FALSE;
    GlCurrentErrorCode = GL_NO_ERROR;
    GlLastError = TGL_ERROR_NONE;
    GlLibraryInitialized = TRUE;

    for (c = 0; c < GL_MAX_CONTEXT; c++)
        GlRenderContext[c] = NULL;

    return TRUE;
}

/**********************************************************************************************/

static GLboolean glSupportsSurfaceFormat(TGL_PIXEL_FORMAT PixelFormat) {
    switch (PixelFormat) {
    case TGL_PIXEL_FORMAT_XRGB8888:
    case TGL_PIXEL_FORMAT_ARGB8888:
        return TRUE;
    default:
        return FALSE;
    }
}

/**********************************************************************************************/

static GLboolean glValidateSurfaceDesc(const TGL_SURFACE_DESC *Surface) {
    if (Surface == NULL || Surface->Pixels == NULL)
        return FALSE;
    if (Surface->Width <= 0 || Surface->Height <= 0)
        return FALSE;
    if (!glSupportsSurfaceFormat(Surface->PixelFormat))
        return FALSE;
    if (Surface->Pitch < Surface->Width * 4)
        return FALSE;
    return TRUE;
}

/**********************************************************************************************/

static void glUpdateContextSurfaceState(LPGLRENDERCONTEXT Context,
                                        const TGL_SURFACE_DESC *Surface,
                                        TGL_SURFACE_MODE SurfaceMode) {
    Context->Surface = *Surface;
    Context->SurfaceMode = SurfaceMode;
    Context->SurfacePixelFormat = Surface->PixelFormat;
    Context->ColorBuffer = (GLU8 *)Surface->Pixels;
    Context->ColorBufferSize = Surface->Pitch * Surface->Height;
    Context->Width = Surface->Width;
    Context->Height = Surface->Height;
    Context->ViewPort[0] = 0;
    Context->ViewPort[1] = 0;
    Context->ViewPort[2] = Surface->Width;
    Context->ViewPort[3] = Surface->Height;
}

/**********************************************************************************************/

static void glDetachContextSurface(LPGLRENDERCONTEXT Context) {
    memset(&(Context->Surface), 0, sizeof(Context->Surface));
    Context->SurfaceMode = TGL_SURFACE_MODE_NONE;
    Context->SurfacePixelFormat = TGL_PIXEL_FORMAT_INVALID;
    Context->ColorBuffer = NULL;
    Context->ColorBufferSize = 0;
}

/**********************************************************************************************/

static GLboolean glSetContextSurface(LPGLRENDERCONTEXT Context,
                                     const TGL_SURFACE_DESC *Surface,
                                     TGL_SURFACE_MODE SurfaceMode) {
    if (!glValidateSurfaceDesc(Surface))
        return FALSE;

    glUpdateContextSurfaceState(Context, Surface, SurfaceMode);

    if (Context->BridgeCallbacks.NotifyResize != NULL) {
        Context->BridgeCallbacks.NotifyResize(Context->BridgeCallbacks.UserData,
                                              &(Context->Surface));
    }

    return TRUE;
}

/**********************************************************************************************/

static void glResizeContext(LPGLRENDERCONTEXT rc) {
    if (rc == NULL)
        return;

    if (rc->SurfaceMode == TGL_SURFACE_MODE_HOST_PROVIDED) {
        if (rc->Surface.Width > 0 && rc->Width > rc->Surface.Width)
            rc->Width = rc->Surface.Width;
        if (rc->Surface.Height > 0 && rc->Height > rc->Surface.Height)
            rc->Height = rc->Surface.Height;
    } else if (rc->SurfaceMode == TGL_SURFACE_MODE_INTERNAL &&
               rc->Surface.Pixels != NULL) {
        TGL_SURFACE_DESC Surface;
        GLU8 *Pixels;
        GLsizei Pitch;

        Pitch = rc->Width * 4;
        Pixels = (GLU8 *)glMalloc(Pitch * rc->Height);
        if (Pixels == NULL) {
            glSetLibraryError(TGL_ERROR_OUT_OF_MEMORY);
            return;
        }

        glFree(rc->InternalSurface.Pixels);

        Surface.Pixels = Pixels;
        Surface.Width = rc->Width;
        Surface.Height = rc->Height;
        Surface.Pitch = Pitch;
        Surface.PixelFormat = rc->SurfacePixelFormat;

        rc->InternalSurface = Surface;
        glUpdateContextSurfaceState(rc, &(rc->InternalSurface),
                                    TGL_SURFACE_MODE_INTERNAL);
    }

    if (!rc->HasDepthBuffer)
        return;

    // Free the current depth buffer
    glFree(rc->DepthBuffer);
    rc->DepthBuffer = NULL;

    // Compute the size in bytes of the depth buffer
    rc->DepthBufferSize = rc->Width * rc->Height;

    // Allocate memory for the depth buffer
    rc->DepthBuffer = (GLU8 *)glMalloc(rc->DepthBufferSize * sizeof(GLfloat));

    // Did the allocation fail ?
    if (rc->DepthBuffer == NULL) {
        glSetErrorCode(GL_OUT_OF_MEMORY);
    }
}

/**********************************************************************************************/

void glResetVertex(LPGLVERTEX Vertex) {
    Vertex->World[X] = 0.0;
    Vertex->World[Y] = 0.0;
    Vertex->World[Z] = 0.0;
    Vertex->World[W] = 1.0;

    Vertex->Color[R] = 1.0;
    Vertex->Color[G] = 1.0;
    Vertex->Color[B] = 1.0;
    Vertex->Color[A] = 1.0;

    Vertex->Normal[X] = 0.0;
    Vertex->Normal[Y] = 0.0;
    Vertex->Normal[Z] = 1.0;
    Vertex->Normal[W] = 0.0;

    Vertex->Tex[X] = 0.0;
    Vertex->Tex[Y] = 0.0;
}

/**********************************************************************************************/

void glResetMaterial(LPGLMATERIAL Material) {
    Material->Ambient[R] = 0.2;
    Material->Ambient[G] = 0.2;
    Material->Ambient[B] = 0.2;
    Material->Ambient[A] = 1.0;

    Material->Diffuse[R] = 0.8;
    Material->Diffuse[G] = 0.8;
    Material->Diffuse[B] = 0.8;
    Material->Diffuse[A] = 1.0;

    Material->Specular[R] = 0.0;
    Material->Specular[G] = 0.0;
    Material->Specular[B] = 0.0;
    Material->Specular[A] = 1.0;

    Material->Emission[R] = 0.0;
    Material->Emission[G] = 0.0;
    Material->Emission[B] = 0.0;
    Material->Emission[A] = 1.0;

    Material->Shininess = 0.0;
}

/**********************************************************************************************/

void glResetPolygon(LPGLPOLYGON Polygon) {
    Polygon->NumVertex = 0;
    glResetMaterial(&(Polygon->FrontFace));
    glResetMaterial(&(Polygon->BackFace));
}

/**********************************************************************************************/

static void glInitRenderContext(LPGLRENDERCONTEXT rc) {
    GLint c;
    GLint d;

    rc->Block.RenderFunc.RenderMode = 1;

    rc->Block.RenderFlag.Alpha = FALSE;
    rc->Block.RenderFlag.Blend = FALSE;
    rc->Block.RenderFlag.Depth = FALSE;
    rc->Block.RenderFlag.Dither = FALSE;
    rc->Block.RenderFlag.EdgeFlag = FALSE;
    rc->Block.RenderFlag.Fog = FALSE;

    rc->Block.RenderFunc.AlphaFunc = GL_ALWAYS;

    rc->Block.RenderFunc.BlendFuncSFactor = GL_SRC_ALPHA;
    rc->Block.RenderFunc.BlendFuncDFactor = GL_ONE_MINUS_SRC_ALPHA;

    rc->Block.RenderFunc.RMask = TRUE;
    rc->Block.RenderFunc.GMask = TRUE;
    rc->Block.RenderFunc.BMask = TRUE;
    rc->Block.RenderFunc.AMask = TRUE;
    rc->Block.RenderFunc.ClearColor[R] = 0.0;
    rc->Block.RenderFunc.ClearColor[G] = 0.0;
    rc->Block.RenderFunc.ClearColor[B] = 0.0;
    rc->Block.RenderFunc.ClearColor[A] = 1.0;

    rc->Block.RenderFunc.DepthMask = TRUE;
    rc->Block.RenderFunc.DepthFunc = GL_LESS;
    rc->Block.RenderFunc.DepthRangeNear = 0.0;
    rc->Block.RenderFunc.DepthRangeFar = 1.0;
    rc->Block.RenderFunc.ClearDepth = 1.0;

    rc->Block.RenderFunc.FogMode = GL_EXP;
    rc->Block.RenderFunc.FogDensity = 1.0;
    rc->Block.RenderFunc.FogStart = 0.0;
    rc->Block.RenderFunc.FogEnd = 1.0;
    rc->Block.RenderFunc.FogIndex = 0.0;
    rc->Block.RenderFunc.FogColor[R] = 0.0;
    rc->Block.RenderFunc.FogColor[G] = 0.0;
    rc->Block.RenderFunc.FogColor[B] = 0.0;
    rc->Block.RenderFunc.FogColor[A] = 0.0;

    rc->Block.XForm.MatrixMode = GL_MATRIX_MODELVIEW;

    for (c = 0; c < 3; c++) {
        glMatrix4dIdentity(rc->Block.XForm.NMatrix[c]);
        glMatrix4dInverse(rc->Block.XForm.IMatrix[c],
                          rc->Block.XForm.NMatrix[c]);
        rc->Block.XForm.MatrixStackDepth[c] = 1;

        for (d = 0; d < GL_MODELVIEW_MATRIX_STACK_MAX; d++) {
            glMatrix4dIdentity(rc->Block.XForm.NMatrixStack[c][d]);
            glMatrix4dIdentity(rc->Block.XForm.IMatrixStack[c][d]);
        }

        glCopyMatrix4d(rc->Block.XForm.NMatrixStack[c][0],
                       rc->Block.XForm.NMatrix[c]);
        glCopyMatrix4d(rc->Block.XForm.IMatrixStack[c][0],
                       rc->Block.XForm.IMatrix[c]);
    }

    glComputeFrustum(rc, &(rc->Block));

    /*
    for (c=0; c<6; c++)
    {
      rc->Block.XForm.Plane[c].Normal[X]          =0.0;
      rc->Block.XForm.Plane[c].Normal[Y]          =0.0;
      rc->Block.XForm.Plane[c].Normal[Z]          =1.0;
      rc->Block.XForm.Plane[c].Normal[W]          =1.0;
      rc->Block.XForm.Plane[c].Distance           =0.0;
    }
    */

    for (c = 0; c < GL_MAX_USER_PLANES; c++) {
        rc->Block.UserPlane[c].On = FALSE;
        rc->Block.UserPlane[c].Normal[X] = 0.0;
        rc->Block.UserPlane[c].Normal[Y] = 0.0;
        rc->Block.UserPlane[c].Normal[Z] = 1.0;
        rc->Block.UserPlane[c].Normal[W] = 1.0;
        rc->Block.UserPlane[c].Distance = 0.0;
    }

    for (c = 0; c < GL_MAX_USER_LIGHTS; c++) {
        rc->Block.Light[c].On = FALSE;

        rc->Block.Light[c].Ambient[R] = 0.0;
        rc->Block.Light[c].Ambient[G] = 0.0;
        rc->Block.Light[c].Ambient[B] = 0.0;
        rc->Block.Light[c].Ambient[A] = 1.0;

        rc->Block.Light[c].Diffuse[R] = 1.0;
        rc->Block.Light[c].Diffuse[G] = 1.0;
        rc->Block.Light[c].Diffuse[B] = 1.0;
        rc->Block.Light[c].Diffuse[A] = 1.0;

        rc->Block.Light[c].Specular[R] = 1.0;
        rc->Block.Light[c].Specular[G] = 1.0;
        rc->Block.Light[c].Specular[B] = 1.0;
        rc->Block.Light[c].Specular[A] = 1.0;

        rc->Block.Light[c].Position[X] = 0.0;
        rc->Block.Light[c].Position[Y] = 0.0;
        rc->Block.Light[c].Position[Z] = 1.0;
        rc->Block.Light[c].Position[W] = 0.0;

        rc->Block.Light[c].Direction[X] = 0.0;
        rc->Block.Light[c].Direction[Y] = 0.0;
        rc->Block.Light[c].Direction[Z] = -1.0;

        rc->Block.Light[c].SpotExponent = 0.0;
        rc->Block.Light[c].SpotCutOff = 180.0;

        rc->Block.Light[c].ConstantAtten = 1.0;
        rc->Block.Light[c].LinearAtten = 0.0;
        rc->Block.Light[c].QuadraticAtten = 0.0;
    }

    // Allocate vertices for input polygon
    glCreatePolygon(&(rc->IPolygon), GL_MAX_POLY_VERTEX);

    glResetPolygon(&(rc->IPolygon));
    glResetVertex(&(rc->IVertex));

    /*
    // Initialize rendering list
    rc->NumLists=0;
    if (!glList_Init(&(rc->RenderList), glDestroyRenderListItem))
    {
      SetLastError(USERERR|GL_INTERNAL_ERROR);
      return;
    }
    */
}

/**********************************************************************************************/

static void glDestroySingleContext(LPGLRENDERCONTEXT Context) {
    if (Context == NULL)
        return;

    if (Context == GlCurrentContext)
        GlCurrentContext = NULL;

    glFree(Context->DepthBuffer);
    Context->DepthBuffer = NULL;

    if (Context->SurfaceMode == TGL_SURFACE_MODE_INTERNAL) {
        glFree(Context->InternalSurface.Pixels);
        memset(&(Context->InternalSurface), 0, sizeof(Context->InternalSurface));
    }

    glDetachContextSurface(Context);
    glFree(Context->IPolygon.Vertex);
    glFree(Context);
}

/**********************************************************************************************/

static void glEnableDisable(GLenum cap, GLboolean val) {
    LPGLRENDERCONTEXT rc = GlCurrentContext;

    // Check if this is a light name
    if (cap >= GL_LIGHT0 && cap < GL_LIGHT0 + GL_MAX_USER_LIGHTS) {
        cap -= GL_LIGHT0;
        rc->Block.Light[cap].On = val;
        return;
    }

    // Check if this is a clipping plane name
    if (cap >= GL_CLIP_PLANE0 && cap < GL_CLIP_PLANE0 + GL_MAX_USER_PLANES) {
        cap -= GL_CLIP_PLANE0;
        rc->Block.UserPlane[cap].On = val;
        return;
    }

    switch (cap) {
    case GL_ALPHA_TEST:
        rc->Block.RenderFlag.Alpha = val;
        return;
    case GL_AUTO_NORMAL:
        return;
    case GL_BLEND:
        rc->Block.RenderFlag.Blend = val;
        return;
    case GL_COLOR_MATERIAL:
        return;
    case GL_CULL_FACE:
        rc->Block.RenderFlag.CullFace = val;
        return;
    case GL_DEPTH_TEST:
        rc->Block.RenderFlag.Depth = val;
        return;
    case GL_DITHER:
        rc->Block.RenderFlag.Dither = val;
        return;
    case GL_FOG:
        rc->Block.RenderFlag.Fog = val;
        return;
    case GL_LIGHTING:
        rc->Block.RenderFlag.Lighting = val;
        return;
    case GL_LINE_SMOOTH:
        rc->Block.RenderFlag.LineSmooth = val;
        return;
    case GL_LINE_STIPPLE:
        rc->Block.RenderFlag.LineStipple = val;
        return;
    case GL_LOGIC_OP:
        rc->Block.RenderFlag.LogicOp = val;
        return;
    case GL_MAP1_COLOR_4:
        rc->Block.RenderFlag.Map1Color4 = val;
        return;
    case GL_MAP1_INDEX:
        rc->Block.RenderFlag.Map1Index = val;
        return;
    case GL_NORMALIZE:
        rc->Block.RenderFlag.Normalize = val;
        return;
    case GL_POINT_SMOOTH:
        rc->Block.RenderFlag.PointSmooth = val;
        return;
    case GL_POLYGON_SMOOTH:
        rc->Block.RenderFlag.PolygonSmooth = val;
        return;
    case GL_POLYGON_STIPPLE:
        rc->Block.RenderFlag.PolygonStipple = val;
        return;
    case GL_SCISSOR_TEST:
        rc->Block.RenderFlag.Scissor = val;
        return;
    case GL_STENCIL_TEST:
        rc->Block.RenderFlag.Stencil = val;
        return;
    case GL_TEXTURE_1D:
        rc->Block.RenderFlag.Texture1D = val;
        return;
    case GL_TEXTURE_2D:
        rc->Block.RenderFlag.Texture2D = val;
        return;
    case GL_TEXTURE_GEN_Q:
        rc->Block.RenderFlag.TextureGenQ = val;
        return;
    case GL_TEXTURE_GEN_R:
        rc->Block.RenderFlag.TextureGenR = val;
        return;
    case GL_TEXTURE_GEN_S:
        rc->Block.RenderFlag.TextureGenS = val;
        return;
    case GL_TEXTURE_GEN_T:
        rc->Block.RenderFlag.TextureGenT = val;
        return;
    }

    glSetErrorCode(GL_INVALID_ENUM);
}

/**********************************************************************************************/

void glAddInputPolygonToRenderList(LPGLRENDERCONTEXT rc) {
    if (rc->IPolygon.NumVertex == 0)
        return;

    /*
    Poly=glAllocatePolygon(GlCurrentPrimitive.NumVertex);

    if (Poly)
    {
      Block=(LPGLRENDERBLOCK)glList_Tail(&(GlCurrentContext->RenderList));
      if (Block)
      {
        if (!glList_Add(&(Block->PolygonList), (GLvoid*)Poly))
        {
          glFree(Poly);
          SetLastError(USERERR|GL_INTERNAL_ERROR);
        }
      }
    }
    else
    {
      SetLastError(USERERR|GL_OUT_OF_MEMORY);
    }
    */

    // If we are not making a render list, render the polygon right now
    // glRasterSetContext(rc);
    glRasterizePolygon(rc, &(rc->Block), &(rc->IPolygon));

    rc->IPolygon.NumVertex = 0;
}

/**********************************************************************************************/

void glCheckInputPolygon(LPGLRENDERCONTEXT rc) {
    switch (GlPrimitiveType) {
    case GL_POINTS:
        if (rc->IPolygon.NumVertex == 1)
            glAddInputPolygonToRenderList(rc);
        break;
    case GL_LINES:
        if (rc->IPolygon.NumVertex == 2)
            glAddInputPolygonToRenderList(rc);
        break;
    case GL_LINE_STRIP:
        break;
    case GL_LINE_LOOP:
        break;
    case GL_TRIANGLES:
        if (rc->IPolygon.NumVertex == 3)
            glAddInputPolygonToRenderList(rc);
        break;
    case GL_TRIANGLE_STRIP:
        break;
    case GL_TRIANGLE_FAN:
        break;
    case GL_QUADS:
        if (rc->IPolygon.NumVertex == 4)
            glAddInputPolygonToRenderList(rc);
        break;
    case GL_QUAD_STRIP:
        break;
    case GL_POLYGON:
        break;
    }
}

/**********************************************************************************************/

void glAddInputVertexToInputPolygon(LPGLRENDERCONTEXT rc) {
    GLMATRIX4D *CNMatrix;
    GLenum nv = rc->IPolygon.NumVertex;

    if (nv < GL_MAX_POLY_VERTEX) {
        // Copy the vertex to the destination polygon
        memcpy(&(rc->IPolygon.Vertex[nv]), &(rc->IVertex), sizeof(GLVERTEX));

        // Get the current modelview matrix
        CNMatrix = &(rc->Block.XForm.NMatrix[GL_MATRIX_MODELVIEW]);

        // Transform the vertex with the modelview matrix
        glMatrix4dTransVector4d(rc->IPolygon.Vertex[nv].Eye,
                                rc->IPolygon.Vertex[nv].World, *CNMatrix);

        // Transform the vertex normal with the modelview matrix
        glMatrix4dTransVector4d(rc->IPolygon.Vertex[nv].Normal,
                                rc->IPolygon.Vertex[nv].Normal, *CNMatrix);

        rc->IPolygon.NumVertex++;
    }

    glCheckInputPolygon(rc);
}

/**********************************************************************************************/

EXPORT TGL_RESULT APIENTRY
tinyglCreateContext(const TGL_CONTEXT_DESC *ContextDesc, TGLContext *OutContext) {
    LPGLRENDERCONTEXT Context;
    GLsizei DefaultWidth;
    GLsizei DefaultHeight;
    GLU32 ContextIndex;

    if (OutContext == NULL) {
        glSetLibraryError(TGL_ERROR_INVALID_ARGUMENT);
        return TGL_RESULT_INVALID_ARGUMENT;
    }

    *OutContext = NULL;

    if (GlInputPrimitive || GlNumContext >= GL_MAX_CONTEXT) {
        glSetLibraryError(TGL_ERROR_INVALID_CONTEXT);
        return TGL_RESULT_INVALID_OPERATION;
    }

    if (!GlLibraryInitialized)
        glInitLibrary();

    Context = (LPGLRENDERCONTEXT)glMalloc(sizeof(GLRENDERCONTEXT));
    if (Context == NULL) {
        glSetLibraryError(TGL_ERROR_OUT_OF_MEMORY);
        return TGL_RESULT_OUT_OF_MEMORY;
    }

    memset(Context, 0, sizeof(GLRENDERCONTEXT));
    Context->Id = 0x0873;

    DefaultWidth = 100;
    DefaultHeight = 100;

    if (ContextDesc != NULL) {
        if (ContextDesc->MaxWidth > 0)
            DefaultWidth = ContextDesc->MaxWidth;
        if (ContextDesc->MaxHeight > 0)
            DefaultHeight = ContextDesc->MaxHeight;

        Context->HasDepthBuffer = ContextDesc->HasDepthBuffer;
        Context->HasColorBuffer = ContextDesc->HasColorBuffer;
    } else {
        Context->HasDepthBuffer = TRUE;
        Context->HasColorBuffer = TRUE;
    }

    Context->SurfacePixelFormat = TGL_PIXEL_FORMAT_XRGB8888;
    Context->ViewPort[0] = 0;
    Context->ViewPort[1] = 0;
    Context->ViewPort[2] = DefaultWidth;
    Context->ViewPort[3] = DefaultHeight;
    Context->Width = DefaultWidth;
    Context->Height = DefaultHeight;

    glInitRenderContext(Context);
    glResizeContext(Context);

    if (Context->HasDepthBuffer && Context->DepthBuffer == NULL) {
        glDestroySingleContext(Context);
        glSetLibraryError(TGL_ERROR_OUT_OF_MEMORY);
        return TGL_RESULT_OUT_OF_MEMORY;
    }

    for (ContextIndex = 0; ContextIndex < GL_MAX_CONTEXT; ContextIndex++) {
        if (GlRenderContext[ContextIndex] == NULL) {
            GlRenderContext[ContextIndex] = Context;
            GlNumContext++;
            *OutContext = (TGLContext)Context;
            glClearLibraryError();
            return TGL_RESULT_OK;
        }
    }

    glDestroySingleContext(Context);
    glSetLibraryError(TGL_ERROR_INTERNAL);
    return TGL_RESULT_INTERNAL_ERROR;
}

/**********************************************************************************************/

/**
 * @brief Return the TinyGL library version.
 * @param Version Receives the semantic version components.
 * @return TGL_RESULT_OK on success, otherwise an error code.
 */
EXPORT TGL_RESULT APIENTRY tinyglGetVersion(TGL_VERSION *Version) {
    if (Version == NULL) {
        glSetLibraryError(TGL_ERROR_INVALID_ARGUMENT);
        return TGL_RESULT_INVALID_ARGUMENT;
    }

    Version->Major = TINYGL_VERSION_MAJOR;
    Version->Minor = TINYGL_VERSION_MINOR;
    Version->Patch = TINYGL_VERSION_PATCH;

    glClearLibraryError();
    return TGL_RESULT_OK;
}

/**********************************************************************************************/

EXPORT TGL_RESULT APIENTRY tinyglDestroyContext(TGLContext Context) {
    GLU32 ContextIndex;

    if (Context == NULL) {
        glSetLibraryError(TGL_ERROR_INVALID_ARGUMENT);
        return TGL_RESULT_INVALID_ARGUMENT;
    }

    if (GlInputPrimitive) {
        glSetLibraryError(TGL_ERROR_INVALID_CONTEXT);
        return TGL_RESULT_INVALID_OPERATION;
    }

    for (ContextIndex = 0; ContextIndex < GL_MAX_CONTEXT; ContextIndex++) {
        if (GlRenderContext[ContextIndex] == (LPGLRENDERCONTEXT)Context) {
            glDestroySingleContext(GlRenderContext[ContextIndex]);
            GlRenderContext[ContextIndex] = NULL;
            GlNumContext--;
            glClearLibraryError();
            return TGL_RESULT_OK;
        }
    }

    glSetLibraryError(TGL_ERROR_INVALID_CONTEXT);
    return TGL_RESULT_INVALID_ARGUMENT;
}

/**********************************************************************************************/

EXPORT TGL_RESULT APIENTRY tinyglMakeCurrent(TGLContext Context) {
    if (GlInputPrimitive) {
        glSetLibraryError(TGL_ERROR_INVALID_CONTEXT);
        return TGL_RESULT_INVALID_OPERATION;
    }

    if (Context == NULL) {
        GlCurrentContext = NULL;
        glClearLibraryError();
        return TGL_RESULT_OK;
    }

    if (((LPGLRENDERCONTEXT)Context)->Id != 0x0873) {
        glSetLibraryError(TGL_ERROR_INVALID_CONTEXT);
        return TGL_RESULT_INVALID_ARGUMENT;
    }

    GlCurrentContext = (LPGLRENDERCONTEXT)Context;
    glClearLibraryError();
    return TGL_RESULT_OK;
}

/**********************************************************************************************/

EXPORT TGLContext APIENTRY tinyglGetCurrentContext(void) {
    return (TGLContext)GlCurrentContext;
}

/**********************************************************************************************/

EXPORT TGL_RESULT APIENTRY tinyglSetSurface(TGLContext Context,
                                            const TGL_SURFACE_DESC *Surface) {
    LPGLRENDERCONTEXT RenderContext;

    if (Context == NULL || Surface == NULL) {
        glSetLibraryError(TGL_ERROR_INVALID_ARGUMENT);
        return TGL_RESULT_INVALID_ARGUMENT;
    }

    RenderContext = (LPGLRENDERCONTEXT)Context;
    if (!glSetContextSurface(RenderContext, Surface,
                             TGL_SURFACE_MODE_HOST_PROVIDED)) {
        glSetLibraryError(TGL_ERROR_INVALID_SURFACE);
        return TGL_RESULT_INVALID_ARGUMENT;
    }

    glResizeContext(RenderContext);
    glClearLibraryError();
    return TGL_RESULT_OK;
}

/**********************************************************************************************/

EXPORT TGL_RESULT APIENTRY tinyglCreateInternalSurface(
    TGLContext Context, GLsizei Width, GLsizei Height,
    TGL_PIXEL_FORMAT PixelFormat) {
    LPGLRENDERCONTEXT RenderContext;
    TGL_SURFACE_DESC Surface;
    GLU8 *Pixels;
    GLsizei Pitch;

    if (Context == NULL || Width <= 0 || Height <= 0) {
        glSetLibraryError(TGL_ERROR_INVALID_ARGUMENT);
        return TGL_RESULT_INVALID_ARGUMENT;
    }

    if (!glSupportsSurfaceFormat(PixelFormat)) {
        glSetLibraryError(TGL_ERROR_INVALID_SURFACE);
        return TGL_RESULT_UNSUPPORTED;
    }

    RenderContext = (LPGLRENDERCONTEXT)Context;
    Pitch = Width * 4;
    Pixels = (GLU8 *)glMalloc(Pitch * Height);
    if (Pixels == NULL) {
        glSetLibraryError(TGL_ERROR_OUT_OF_MEMORY);
        return TGL_RESULT_OUT_OF_MEMORY;
    }

    if (RenderContext->SurfaceMode == TGL_SURFACE_MODE_INTERNAL) {
        glFree(RenderContext->InternalSurface.Pixels);
        memset(&(RenderContext->InternalSurface), 0,
               sizeof(RenderContext->InternalSurface));
    }

    Surface.Pixels = Pixels;
    Surface.Width = Width;
    Surface.Height = Height;
    Surface.Pitch = Pitch;
    Surface.PixelFormat = PixelFormat;

    RenderContext->InternalSurface = Surface;

    if (!glSetContextSurface(RenderContext, &(RenderContext->InternalSurface),
                             TGL_SURFACE_MODE_INTERNAL)) {
        glFree(Pixels);
        memset(&(RenderContext->InternalSurface), 0,
               sizeof(RenderContext->InternalSurface));
        glSetLibraryError(TGL_ERROR_INVALID_SURFACE);
        return TGL_RESULT_INVALID_ARGUMENT;
    }

    glResizeContext(RenderContext);
    glClearLibraryError();
    return TGL_RESULT_OK;
}

/**********************************************************************************************/

EXPORT TGL_RESULT APIENTRY tinyglDestroyInternalSurface(TGLContext Context) {
    LPGLRENDERCONTEXT RenderContext;

    if (Context == NULL) {
        glSetLibraryError(TGL_ERROR_INVALID_ARGUMENT);
        return TGL_RESULT_INVALID_ARGUMENT;
    }

    RenderContext = (LPGLRENDERCONTEXT)Context;
    if (RenderContext->SurfaceMode != TGL_SURFACE_MODE_INTERNAL) {
        glSetLibraryError(TGL_ERROR_INVALID_SURFACE);
        return TGL_RESULT_INVALID_OPERATION;
    }

    glFree(RenderContext->InternalSurface.Pixels);
    memset(&(RenderContext->InternalSurface), 0,
           sizeof(RenderContext->InternalSurface));
    glDetachContextSurface(RenderContext);
    glClearLibraryError();
    return TGL_RESULT_OK;
}

/**********************************************************************************************/

EXPORT TGL_RESULT APIENTRY tinyglGetSurface(TGLContext Context,
                                            TGL_SURFACE_DESC *Surface) {
    if (Context == NULL || Surface == NULL) {
        glSetLibraryError(TGL_ERROR_INVALID_ARGUMENT);
        return TGL_RESULT_INVALID_ARGUMENT;
    }

    *Surface = ((LPGLRENDERCONTEXT)Context)->Surface;
    glClearLibraryError();
    return TGL_RESULT_OK;
}

/**********************************************************************************************/

EXPORT TGL_SURFACE_MODE APIENTRY tinyglGetSurfaceMode(TGLContext Context) {
    if (Context == NULL) {
        glSetLibraryError(TGL_ERROR_INVALID_ARGUMENT);
        return TGL_SURFACE_MODE_NONE;
    }

    glClearLibraryError();
    return ((LPGLRENDERCONTEXT)Context)->SurfaceMode;
}

/**********************************************************************************************/

EXPORT TGL_RESULT APIENTRY
tinyglSetBridgeCallbacks(TGLContext Context,
                         const TGL_BRIDGE_CALLBACKS *Callbacks) {
    if (Context == NULL || Callbacks == NULL) {
        glSetLibraryError(TGL_ERROR_INVALID_ARGUMENT);
        return TGL_RESULT_INVALID_ARGUMENT;
    }

    ((LPGLRENDERCONTEXT)Context)->BridgeCallbacks = *Callbacks;
    glClearLibraryError();
    return TGL_RESULT_OK;
}

/**********************************************************************************************/

EXPORT TGL_PRESENT_RESULT APIENTRY tinyglPresent(TGLContext Context) {
    LPGLRENDERCONTEXT RenderContext;

    if (Context == NULL) {
        glSetLibraryError(TGL_ERROR_INVALID_ARGUMENT);
        return TGL_PRESENT_RESULT_FAILED;
    }

    RenderContext = (LPGLRENDERCONTEXT)Context;
    if (RenderContext->Surface.Pixels == NULL) {
        glSetLibraryError(TGL_ERROR_INVALID_SURFACE);
        return TGL_PRESENT_RESULT_NO_SURFACE;
    }

    if (RenderContext->BridgeCallbacks.Present != NULL) {
        return RenderContext->BridgeCallbacks.Present(
            RenderContext->BridgeCallbacks.UserData, &(RenderContext->Surface));
    }

    glClearLibraryError();
    return TGL_PRESENT_RESULT_OK;
}

/**********************************************************************************************/

EXPORT TGL_ERROR APIENTRY tinyglGetLastError(void) { return GlLastError; }

/**********************************************************************************************/

EXPORT void APIENTRY tinyglClearLastError(void) { glClearLibraryError(); }

/**********************************************************************************************/

EXPORT void *APIENTRY tinyglGetProcAddress(const char *Name) {
    if (Name == NULL) {
        glSetLibraryError(TGL_ERROR_INVALID_ARGUMENT);
        return NULL;
    }

    glClearLibraryError();
    return NULL;
}

/**********************************************************************************************/

EXPORT void APIENTRY glBegin(GLenum mode) {
    if (!glCheckState())
        return;

    switch (mode) {
    case GL_POINTS:
        GlPrimitiveType = GL_POINTS;
        break;
    case GL_LINES:
        GlPrimitiveType = GL_LINES;
        break;
    case GL_LINE_STRIP:
        GlPrimitiveType = GL_LINE_STRIP;
        break;
    case GL_LINE_LOOP:
        GlPrimitiveType = GL_LINE_LOOP;
        break;
    case GL_TRIANGLES:
        GlPrimitiveType = GL_TRIANGLES;
        break;
    case GL_TRIANGLE_STRIP:
        GlPrimitiveType = GL_TRIANGLE_STRIP;
        break;
    case GL_TRIANGLE_FAN:
        GlPrimitiveType = GL_TRIANGLE_FAN;
        break;
    case GL_QUADS:
        GlPrimitiveType = GL_QUADS;
        break;
    case GL_QUAD_STRIP:
        GlPrimitiveType = GL_QUAD_STRIP;
        break;
    case GL_POLYGON:
        GlPrimitiveType = GL_POLYGON;
        break;
    default:
        glSetErrorCode(GL_INVALID_ENUM);
        return;
    }

    /*
    // Create a new rendering block
    lpBlock=(LPGLRENDERBLOCK)glMalloc(sizeof(GLRENDERBLOCK));

    if (Block)
    {
      if (!glList_Add(&(GlCurrentContext->RenderList), (GLvoid*)lpBlock))
      {
        SetLastError(USERERR|GL_INTERNAL_ERROR);
        return;
      }
      if (!glList_Init(&(lpBlock->PolygonList), glDestroyPolygonListItem))
      {
        SetLastError(USERERR|GL_INTERNAL_ERROR);
        return;
      }
    }
    else
    {
      SetLastError(USERERR|GL_OUT_OF_MEMORY);
      return;
    }
    */

    // Initialize input polygon and vertex
    // glResetPolygon(&(GlCurrentContext->IPolygon));
    // glResetVertex(&(GlCurrentContext->IVertex));

    // Set input flag to TRUE
    GlInputPrimitive = TRUE;
}

/**********************************************************************************************/

EXPORT void APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor) {
    if (!glCheckState())
        return;

    if (sfactor != GL_ZERO && sfactor != GL_ONE && sfactor != GL_DST_COLOR &&
        sfactor != GL_ONE_MINUS_DST_COLOR && sfactor != GL_SRC_ALPHA &&
        sfactor != GL_ONE_MINUS_SRC_ALPHA && sfactor != GL_DST_ALPHA &&
        sfactor != GL_ONE_MINUS_DST_ALPHA && sfactor != GL_SRC_ALPHA_SATURATE) {
        glSetErrorCode(GL_INVALID_ENUM);
    }

    if (dfactor != GL_ZERO && dfactor != GL_ONE && dfactor != GL_SRC_COLOR &&
        dfactor != GL_ONE_MINUS_SRC_COLOR && dfactor != GL_SRC_ALPHA &&
        dfactor != GL_ONE_MINUS_SRC_ALPHA && dfactor != GL_DST_ALPHA &&
        dfactor != GL_ONE_MINUS_DST_ALPHA) {
        glSetErrorCode(GL_INVALID_ENUM);
    }

    GlCurrentContext->Block.RenderFunc.BlendFuncSFactor = sfactor;
    GlCurrentContext->Block.RenderFunc.BlendFuncDFactor = dfactor;
}

/**********************************************************************************************/

EXPORT void APIENTRY glClear(GLbitfield mask) {
    GLU32 *ColorBuffer;
    GLU32 ClearColor;
    GLfloat *DepthBuffer;
    GLfloat ClearDepth;
    GLU32 c;

    if (!glCheckState())
        return;

    if (mask & GL_COLOR_BUFFER_BIT) {
        if (GlCurrentContext->ColorBuffer) {
            ColorBuffer = (GLU32 *)GlCurrentContext->ColorBuffer;

            switch (GlCurrentContext->SurfacePixelFormat) {
            case TGL_PIXEL_FORMAT_ARGB8888:
                ClearColor =
                    (((GLU32)(GlCurrentContext->Block.RenderFunc.ClearColor[A] *
                              255.0))
                     << 24) |
                    (((GLU32)(GlCurrentContext->Block.RenderFunc.ClearColor[R] *
                              255.0))
                     << 16) |
                    (((GLU32)(GlCurrentContext->Block.RenderFunc.ClearColor[G] *
                              255.0))
                     << 8) |
                    ((GLU32)(GlCurrentContext->Block.RenderFunc.ClearColor[B] *
                             255.0));
                break;

            case TGL_PIXEL_FORMAT_XRGB8888:
            default:
                ClearColor =
                    (((GLU32)(GlCurrentContext->Block.RenderFunc.ClearColor[R] *
                              255.0))
                     << 16) |
                    (((GLU32)(GlCurrentContext->Block.RenderFunc.ClearColor[G] *
                              255.0))
                     << 8) |
                    ((GLU32)(GlCurrentContext->Block.RenderFunc.ClearColor[B] *
                             255.0));
                break;
            }

            for (c = 0; c < (GlCurrentContext->ColorBufferSize / 4); c++) {
                ColorBuffer[c] = ClearColor;
            }
        }
    }

    if (mask & GL_DEPTH_BUFFER_BIT) {
        if (GlCurrentContext->DepthBuffer) {
            DepthBuffer = (GLfloat *)GlCurrentContext->DepthBuffer;
            ClearDepth = GlCurrentContext->Block.RenderFunc.ClearDepth;
            for (c = 0; c < GlCurrentContext->DepthBufferSize; c++) {
                DepthBuffer[c] = ClearDepth;
            }
        }
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glClearAccum(GLfloat red, GLfloat green, GLfloat blue,
                                  GLfloat alpha) {
    GL_UNUSED(red);
    GL_UNUSED(green);
    GL_UNUSED(blue);
    GL_UNUSED(alpha);
}

/**********************************************************************************************/

EXPORT void APIENTRY glClearColor(GLclampf red, GLclampf green, GLclampf blue,
                                  GLclampf alpha) {
    if (GlCurrentContext == NULL)
        return;

    CLAMP(red, 0.0, 1.0);
    CLAMP(green, 0.0, 1.0);
    CLAMP(blue, 0.0, 1.0);
    CLAMP(alpha, 0.0, 1.0);

    GlCurrentContext->Block.RenderFunc.ClearColor[R] = red;
    GlCurrentContext->Block.RenderFunc.ClearColor[G] = green;
    GlCurrentContext->Block.RenderFunc.ClearColor[B] = blue;
    GlCurrentContext->Block.RenderFunc.ClearColor[A] = alpha;
}

/**********************************************************************************************/

EXPORT void APIENTRY glClearDepth(GLclampd depth) {
    if (!glCheckState())
        return;

    GlCurrentContext->Block.RenderFunc.ClearDepth = depth;
}

/**********************************************************************************************/

EXPORT void APIENTRY glClearIndex(GLfloat c) { GL_UNUSED(c); }

/**********************************************************************************************/

EXPORT void APIENTRY glClearStencil(GLint s) { GL_UNUSED(s); }

/**********************************************************************************************/

EXPORT void APIENTRY glClipPlane(GLenum plane, const GLdouble *equation) {
    GL_UNUSED(plane);
    GL_UNUSED(equation);
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3b(GLbyte red, GLbyte green, GLbyte blue) {
    GL_UNUSED(red);
    GL_UNUSED(green);
    GL_UNUSED(blue);
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3bv(const GLbyte *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glColor3d(GLdouble red, GLdouble green, GLdouble blue) {
    if (GlCurrentContext) {
        GlCurrentContext->IVertex.Color[R] = red;
        GlCurrentContext->IVertex.Color[G] = green;
        GlCurrentContext->IVertex.Color[B] = blue;
        GlCurrentContext->IVertex.Color[A] = 1.0;
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3dv(const GLdouble *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glColor3f(GLfloat red, GLfloat green, GLfloat blue) {
    if (GlCurrentContext) {
        GlCurrentContext->IVertex.Color[R] = red;
        GlCurrentContext->IVertex.Color[G] = green;
        GlCurrentContext->IVertex.Color[B] = blue;
        GlCurrentContext->IVertex.Color[A] = 1.0;
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3fv(const GLfloat *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glColor3i(GLint red, GLint green, GLint blue) {
    GL_UNUSED(red);
    GL_UNUSED(green);
    GL_UNUSED(blue);
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3iv(const GLint *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glColor3s(GLshort red, GLshort green, GLshort blue) {
    GL_UNUSED(red);
    GL_UNUSED(green);
    GL_UNUSED(blue);
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3sv(const GLshort *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glColor3ub(GLubyte red, GLubyte green, GLubyte blue) {
    GL_UNUSED(red);
    GL_UNUSED(green);
    GL_UNUSED(blue);
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3ubv(const GLubyte *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glColor3ui(GLuint red, GLuint green, GLuint blue) {
    GL_UNUSED(red);
    GL_UNUSED(green);
    GL_UNUSED(blue);
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3uiv(const GLuint *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glColor3us(GLushort red, GLushort green, GLushort blue) {
    GL_UNUSED(red);
    GL_UNUSED(green);
    GL_UNUSED(blue);
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3usv(const GLushort *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glDisable(GLenum cap) {
    if (!glCheckState())
        return;
    glEnableDisable(cap, FALSE);
}

/**********************************************************************************************/

EXPORT void APIENTRY glDrawBuffer(GLenum mode) { GL_UNUSED(mode); }

/**********************************************************************************************/

EXPORT void APIENTRY glEnable(GLenum cap) {
    if (!glCheckState())
        return;
    glEnableDisable(cap, TRUE);
}

/**********************************************************************************************/

EXPORT void APIENTRY glEnd() {
    if (GlInputPrimitive == FALSE) {
        glSetErrorCode(GL_INVALID_OPERATION);
        return;
    }

    glAddInputPolygonToRenderList(GlCurrentContext);

    GlInputPrimitive = FALSE;

    /*
    if (!glCheckState()) return;

    Block=glList_Tail(&(GlCurrentContext->RenderList));

    if (Block)
    {
      memcpy(Block, &(GlCurrentContext->Block),
    sizeof(GLRENDERBLOCK)-sizeof(GLLIST));
    }
    else
    {
      SetLastError(USERERR|GL_INTERNAL_ERROR);
    }
    */
}

/**********************************************************************************************/

EXPORT void APIENTRY glFlush() {
    if (!glCheckState())
        return;

    /*
    List=&(GlCurrentContext->RenderList);
    Block=(LPGLRENDERBLOCK)glList_GetPointer(List, Count++);

    while (Block)
    {
      glRasterizeBlock(GlCurrentContext, Block);
      Block=(LPGLRENDERBLOCK)glList_GetPointer(List, Count++);
    }

    glList_Destroy(List);
    */
}

/**********************************************************************************************/

EXPORT void APIENTRY glFrustum(GLdouble left, GLdouble right, GLdouble bottom,
                               GLdouble top, GLdouble znear, GLdouble zfar) {
    GLdouble tmp[4][4];
    GLMATRIX4D *CNMatrix;
    GLMATRIX4D *CIMatrix;

    if (!glCheckState())
        return;

    if (0.0 == (right - left))
        right += 0.001;
    if (0.0 == (top - bottom))
        top += 0.001;
    if (0.0 == (zfar - znear))
        zfar += 0.001;

    tmp[0][0] = (2.0 * znear) / (right - left);
    tmp[0][1] = 0.0;
    tmp[0][2] = (right + left) / (right - left);
    tmp[0][3] = 0.0;

    tmp[1][0] = 0.0;
    tmp[1][1] = (2.0 * znear) / (top - bottom);
    tmp[1][2] = (top + bottom) / (top - bottom);
    tmp[1][3] = 0.0;

    tmp[2][0] = 0.0;
    tmp[2][1] = 0.0;
    tmp[2][2] = 0.0 - ((zfar + znear) / (zfar - znear));
    tmp[2][3] = 0.0 - ((2.0 * zfar * znear) / (zfar - znear));

    tmp[3][0] = 0.0;
    tmp[3][1] = 0.0;
    tmp[3][2] = -1.0;
    tmp[3][3] = 0.0;

    CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
    CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

    glMatrix4dConcat(*CNMatrix, *CNMatrix, tmp, GL_MATOP_POSTCONCATENATE);
    glMatrix4dInverse(*CIMatrix, *CNMatrix);
    glSyncCurrentMatrixStackEntry(GlCurrentContext);

    if (GlCurrentContext->Block.XForm.MatrixMode == GL_MATRIX_PROJECTION) {
        glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glGetBooleanv(GLenum pname, GLboolean *params) {
    GL_UNUSED(pname);
    GL_UNUSED(params);

    if (!glCheckState())
        return;
}

/**********************************************************************************************/

EXPORT void APIENTRY glGetClipPlane(GLenum plane, GLdouble *equation) {
    GL_UNUSED(plane);
    GL_UNUSED(equation);

    if (!glCheckState())
        return;
}

/**********************************************************************************************/

EXPORT void APIENTRY glGetDoublev(GLenum pname, GLdouble *params) {
    LPGLRENDERCONTEXT rc;
    GLint c;

    if (!glCheckState())
        return;

    rc = GlCurrentContext;

    if (params == NULL) {
        glSetErrorCode(GL_INVALID_OPERATION);
        return;
    }

    // Check if this is a light name
    if (pname >= GL_LIGHT0 && pname < GL_LIGHT0 + GL_MAX_USER_LIGHTS) {
        params[0] = rc->Block.Light[pname - GL_LIGHT0].On ? 1.0 : 0.0;
        return;
    }

    switch (pname) {
    case GL_MODELVIEW_MATRIX: {
        for (c = 0; c < 16; c++)
            params[c] = rc->Block.XForm.NMatrix[GL_MATRIX_MODELVIEW][0][c];
        return;
    }

    case GL_NORMALIZE: {
        params[0] = rc->Block.RenderFlag.Normalize ? 1.0 : 0.0;
        return;
    }

    case GL_PROJECTION_MATRIX: {
        for (c = 0; c < 16; c++)
            params[c] = rc->Block.XForm.NMatrix[GL_MATRIX_PROJECTION][0][c];
        return;
    }
    }

    glSetErrorCode(GL_INVALID_ENUM);
}

/**********************************************************************************************/

EXPORT GLenum APIENTRY glGetError() {
    if (!glCheckState()) {
        glSetErrorCode(GL_INVALID_OPERATION);
        return 0;
    }

    {
        GLenum ErrorCode;

        ErrorCode = glGetErrorCode();
        glClearErrorCode();
        return ErrorCode;
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glGetFloatv(GLenum pname, GLfloat *params) {
    LPGLRENDERCONTEXT rc;
    GLint c;

    if (!glCheckState())
        return;

    rc = GlCurrentContext;

    if (params == NULL) {
        glSetErrorCode(GL_INVALID_OPERATION);
        return;
    }

    // Check if this is a light name
    if (pname >= GL_LIGHT0 && pname < GL_LIGHT0 + GL_MAX_USER_LIGHTS) {
        params[0] = rc->Block.Light[pname - GL_LIGHT0].On ? 1.0 : 0.0;
        return;
    }

    switch (pname) {
    case GL_MODELVIEW_MATRIX: {
        for (c = 0; c < 16; c++)
            params[c] =
                (GLfloat)rc->Block.XForm.NMatrix[GL_MATRIX_MODELVIEW][0][c];
        return;
    }

    case GL_NORMALIZE: {
        params[0] = rc->Block.RenderFlag.Normalize ? 1.0 : 0.0;
        return;
    }

    case GL_PROJECTION_MATRIX: {
        for (c = 0; c < 16; c++)
            params[c] =
                (GLfloat)rc->Block.XForm.NMatrix[GL_MATRIX_PROJECTION][0][c];
        return;
    }

    case GL_VIEWPORT: {
        params[0] = (GLfloat)rc->ViewPort[0];
        params[1] = (GLfloat)rc->ViewPort[1];
        params[2] = (GLfloat)rc->ViewPort[2];
        params[3] = (GLfloat)rc->ViewPort[3];
        return;
    }
    }

    glSetErrorCode(GL_INVALID_ENUM);
}

/**********************************************************************************************/

EXPORT void APIENTRY glGetIntegerv(GLenum pname, GLint *params) {
    LPGLRENDERCONTEXT lpRC;

    if (!glCheckState())
        return;

    if (params == NULL) {
        glSetErrorCode(GL_INVALID_OPERATION);
        return;
    }

    lpRC = GlCurrentContext;

    // Check if this is a light name
    if (pname >= GL_LIGHT0 && pname < GL_LIGHT0 + GL_MAX_USER_LIGHTS) {
        params[0] = lpRC->Block.Light[pname - GL_LIGHT0].On ? 1 : 0;
        return;
    }

    switch (pname) {
    case GL_DEPTH_FUNC: {
        params[0] = lpRC->Block.RenderFunc.DepthFunc;
        return;
    }

    case GL_DEPTH_TEST: {
        params[0] = lpRC->Block.RenderFlag.Depth ? 1 : 0;
        return;
    }

    case GL_MAX_LIGHTS: {
        params[0] = GL_MAX_USER_LIGHTS;
        return;
    }

    case GL_NORMALIZE: {
        params[0] = lpRC->Block.RenderFlag.Normalize ? 1 : 0;
        return;
    }

    case GL_VIEWPORT: {
        params[0] = lpRC->ViewPort[0];
        params[1] = lpRC->ViewPort[1];
        params[2] = lpRC->ViewPort[2];
        params[3] = lpRC->ViewPort[3];
        return;
    }
    }

    glSetErrorCode(GL_INVALID_ENUM);
}

/**********************************************************************************************/

EXPORT void APIENTRY glLightf(GLenum light, GLenum pname, GLfloat param) {
    if (!glCheckState())
        return;

    light -= GL_LIGHT0;

    if (light > GL_MAX_USER_LIGHTS) {
        glSetErrorCode(GL_INVALID_ENUM);
        return;
    }

    switch (pname) {
    case GL_SPOT_EXPONENT: {
        GlCurrentContext->Block.Light[light].SpotExponent = param;
    } break;

    case GL_SPOT_CUTOFF: {
        GlCurrentContext->Block.Light[light].SpotCutOff = param;
    } break;

    case GL_CONSTANT_ATTENUATION: {
        GlCurrentContext->Block.Light[light].ConstantAtten = param;
    } break;

    case GL_LINEAR_ATTENUATION: {
        GlCurrentContext->Block.Light[light].LinearAtten = param;
    } break;

    case GL_QUADRATIC_ATTENUATION: {
        GlCurrentContext->Block.Light[light].QuadraticAtten = param;
    } break;

    default:
        glSetErrorCode(GL_INVALID_ENUM);
        break;
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glLightfv(GLenum light, GLenum pname,
                               const GLfloat *params) {
    GLMATRIX4D *mat;
    GLdouble *vec;

    if (!glCheckState() || params == NULL)
        return;

    light -= GL_LIGHT0;

    if (light >= GL_MAX_USER_LIGHTS) {
        glSetErrorCode(GL_INVALID_ENUM);
        return;
    }

    switch (pname) {
    case GL_AMBIENT: {
        GlCurrentContext->Block.Light[light].Ambient[R] = (GLdouble)params[0];
        GlCurrentContext->Block.Light[light].Ambient[G] = (GLdouble)params[1];
        GlCurrentContext->Block.Light[light].Ambient[B] = (GLdouble)params[2];
        GlCurrentContext->Block.Light[light].Ambient[A] = (GLdouble)params[3];
    } break;

    case GL_DIFFUSE: {
        GlCurrentContext->Block.Light[light].Diffuse[R] = (GLdouble)params[0];
        GlCurrentContext->Block.Light[light].Diffuse[G] = (GLdouble)params[1];
        GlCurrentContext->Block.Light[light].Diffuse[B] = (GLdouble)params[2];
        GlCurrentContext->Block.Light[light].Diffuse[A] = (GLdouble)params[3];
    } break;

    case GL_SPECULAR: {
        GlCurrentContext->Block.Light[light].Specular[R] = (GLdouble)params[0];
        GlCurrentContext->Block.Light[light].Specular[G] = (GLdouble)params[1];
        GlCurrentContext->Block.Light[light].Specular[B] = (GLdouble)params[2];
        GlCurrentContext->Block.Light[light].Specular[A] = (GLdouble)params[3];
    } break;

    case GL_POSITION: {
        GlCurrentContext->Block.Light[light].Position[X] = (GLdouble)params[0];
        GlCurrentContext->Block.Light[light].Position[Y] = (GLdouble)params[1];
        GlCurrentContext->Block.Light[light].Position[Z] = (GLdouble)params[2];
        GlCurrentContext->Block.Light[light].Position[W] = (GLdouble)params[3];

        vec = (GLdouble *)&(GlCurrentContext->Block.Light[light].Position);
        mat = &(GlCurrentContext->Block.XForm.NMatrix[GL_MATRIX_MODELVIEW]);

        glMatrix4dTransVector4d(vec, vec, *mat);
    } break;

    default:
        glSetErrorCode(GL_INVALID_ENUM);
        break;
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glLighti(GLenum light, GLenum pname, GLint param) {
    GL_UNUSED(light);
    GL_UNUSED(pname);
    GL_UNUSED(param);
}

/**********************************************************************************************/

EXPORT void APIENTRY glLightiv(GLenum light, GLenum pname,
                               const GLint *params) {
    GL_UNUSED(light);
    GL_UNUSED(pname);
    GL_UNUSED(params);
}

/**********************************************************************************************/

EXPORT void APIENTRY glLineStipple(GLint factor, GLushort pattern) {
    GL_UNUSED(factor);
    GL_UNUSED(pattern);
}

/**********************************************************************************************/

EXPORT void APIENTRY glLineWidth(GLfloat width) { GL_UNUSED(width); }

/**********************************************************************************************/

EXPORT void APIENTRY glListBase(GLuint base) {
    GL_UNUSED(base);

    if (!glCheckState())
        return;
}

/**********************************************************************************************/

EXPORT void APIENTRY glLoadIdentity() {
    GLMATRIX4D *mat1;
    GLMATRIX4D *mat2;

    if (!glCheckState())
        return;

    mat1 = glGetContextCurrentNMatrix(GlCurrentContext);
    mat2 = glGetContextCurrentIMatrix(GlCurrentContext);

    glMatrix4dIdentity(*mat1);
    glMatrix4dInverse(*mat2, *mat1);
    glSyncCurrentMatrixStackEntry(GlCurrentContext);

    if (GlCurrentContext->Block.XForm.MatrixMode == GL_MATRIX_PROJECTION) {
        glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glLoadMatrixd(const GLdouble *Matrix) {
    GLMATRIX4D *CNMatrix;
    GLMATRIX4D *CIMatrix;

    if (!glCheckState() || Matrix == NULL)
        return;

    CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
    CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

    glCopyLinearToMatrix4d(*CNMatrix, Matrix);
    glMatrix4dInverse(*CIMatrix, *CNMatrix);
    glSyncCurrentMatrixStackEntry(GlCurrentContext);

    if (GlCurrentContext->Block.XForm.MatrixMode == GL_MATRIX_PROJECTION) {
        glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glLoadMatrixf(const GLfloat *Matrix) {
    GLMATRIX4D *CNMatrix;
    GLMATRIX4D *CIMatrix;

    if (!glCheckState() || Matrix == NULL)
        return;

    CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
    CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

    glCopyLinearFloatToMatrix4d(*CNMatrix, Matrix);
    glMatrix4dInverse(*CIMatrix, *CNMatrix);
    glSyncCurrentMatrixStackEntry(GlCurrentContext);

    if (GlCurrentContext->Block.XForm.MatrixMode == GL_MATRIX_PROJECTION) {
        glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param) {
    GL_UNUSED(face);
    GL_UNUSED(pname);
    GL_UNUSED(param);
}

/**********************************************************************************************/

EXPORT void APIENTRY glMaterialfv(GLenum face, GLenum pname,
                                  const GLfloat *params) {
    LPGLRENDERCONTEXT rc = GlCurrentContext;

    if (rc == NULL)
        return;

    switch (pname) {
    case GL_AMBIENT: {
        if (face == GL_FRONT || face == GL_FRONT_AND_BACK) {
            rc->IPolygon.FrontFace.Ambient[R] = params[R];
            rc->IPolygon.FrontFace.Ambient[G] = params[G];
            rc->IPolygon.FrontFace.Ambient[B] = params[B];
            rc->IPolygon.FrontFace.Ambient[A] = params[A];
        }
        if (face == GL_BACK || face == GL_FRONT_AND_BACK) {
            rc->IPolygon.BackFace.Ambient[R] = params[R];
            rc->IPolygon.BackFace.Ambient[G] = params[G];
            rc->IPolygon.BackFace.Ambient[B] = params[B];
            rc->IPolygon.BackFace.Ambient[A] = params[A];
        }
    } break;

    case GL_DIFFUSE: {
        if (face == GL_FRONT || face == GL_FRONT_AND_BACK) {
            rc->IPolygon.FrontFace.Diffuse[R] = params[R];
            rc->IPolygon.FrontFace.Diffuse[G] = params[G];
            rc->IPolygon.FrontFace.Diffuse[B] = params[B];
            rc->IPolygon.FrontFace.Diffuse[A] = params[A];
        }
        if (face == GL_BACK || face == GL_FRONT_AND_BACK) {
            rc->IPolygon.BackFace.Diffuse[R] = params[R];
            rc->IPolygon.BackFace.Diffuse[G] = params[G];
            rc->IPolygon.BackFace.Diffuse[B] = params[B];
            rc->IPolygon.BackFace.Diffuse[A] = params[A];
        }
    } break;

    case GL_SPECULAR: {
        if (face == GL_FRONT || face == GL_FRONT_AND_BACK) {
            rc->IPolygon.FrontFace.Specular[R] = params[R];
            rc->IPolygon.FrontFace.Specular[G] = params[G];
            rc->IPolygon.FrontFace.Specular[B] = params[B];
            rc->IPolygon.FrontFace.Specular[A] = params[A];
        }
        if (face == GL_BACK || face == GL_FRONT_AND_BACK) {
            rc->IPolygon.BackFace.Specular[R] = params[R];
            rc->IPolygon.BackFace.Specular[G] = params[G];
            rc->IPolygon.BackFace.Specular[B] = params[B];
            rc->IPolygon.BackFace.Specular[A] = params[A];
        }
    } break;

    case GL_EMISSION: {
        if (face == GL_FRONT || face == GL_FRONT_AND_BACK) {
            rc->IPolygon.FrontFace.Emission[R] = params[R];
            rc->IPolygon.FrontFace.Emission[G] = params[G];
            rc->IPolygon.FrontFace.Emission[B] = params[B];
            rc->IPolygon.FrontFace.Emission[A] = params[A];
        }
        if (face == GL_BACK || face == GL_FRONT_AND_BACK) {
            rc->IPolygon.BackFace.Emission[R] = params[R];
            rc->IPolygon.BackFace.Emission[G] = params[G];
            rc->IPolygon.BackFace.Emission[B] = params[B];
            rc->IPolygon.BackFace.Emission[A] = params[A];
        }
    } break;

    case GL_SHININESS: {
        if (face == GL_FRONT || face == GL_FRONT_AND_BACK) {
            rc->IPolygon.FrontFace.Shininess = params[0];
        }
        if (face == GL_BACK || face == GL_FRONT_AND_BACK) {
            rc->IPolygon.BackFace.Shininess = params[0];
        }
    } break;

    default:
        glSetErrorCode(GL_INVALID_ENUM);
        break;
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glMateriali(GLenum face, GLenum pname, GLint param) {
    GL_UNUSED(face);
    GL_UNUSED(pname);
    GL_UNUSED(param);
}

/**********************************************************************************************/

EXPORT void APIENTRY glMaterialiv(GLenum face, GLenum pname,
                                  const GLint *params) {
    GL_UNUSED(face);
    GL_UNUSED(pname);
    GL_UNUSED(params);
}

/**********************************************************************************************/

EXPORT void APIENTRY glMatrixMode(GLenum mode) {
    if (!glCheckState())
        return;

    switch (mode) {
    case GL_MODELVIEW: {
        GlCurrentContext->Block.XForm.MatrixMode = GL_MATRIX_MODELVIEW;
    } break;

    case GL_PROJECTION: {
        GlCurrentContext->Block.XForm.MatrixMode = GL_MATRIX_PROJECTION;
    } break;

    case GL_TEXTURE: {
        GlCurrentContext->Block.XForm.MatrixMode = GL_MATRIX_TEXTURE;
    } break;

    default:
        glSetErrorCode(GL_INVALID_ENUM);
        break;
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glMultMatrixd(const GLdouble *Matrix) {
    GLMATRIX4D *CNMatrix;
    GLMATRIX4D *CIMatrix;
    GLMATRIX4D Temp;

    if (!glCheckState() || Matrix == NULL)
        return;

    CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
    CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

    glCopyLinearToMatrix4d(Temp, Matrix);
    glMatrix4dTimes(*CNMatrix, Temp, *CNMatrix);
    glMatrix4dInverse(*CIMatrix, *CNMatrix);
    glSyncCurrentMatrixStackEntry(GlCurrentContext);

    if (GlCurrentContext->Block.XForm.MatrixMode == GL_MATRIX_PROJECTION) {
        glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glMultMatrixf(const GLfloat *Matrix) {
    GLMATRIX4D *CNMatrix;
    GLMATRIX4D *CIMatrix;
    GLMATRIX4D Temp;

    if (!glCheckState() || Matrix == NULL)
        return;

    CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
    CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

    glCopyLinearFloatToMatrix4d(Temp, Matrix);
    glMatrix4dTimes(*CNMatrix, Temp, *CNMatrix);
    glMatrix4dInverse(*CIMatrix, *CNMatrix);
    glSyncCurrentMatrixStackEntry(GlCurrentContext);

    if (GlCurrentContext->Block.XForm.MatrixMode == GL_MATRIX_PROJECTION) {
        glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz) {
    GL_UNUSED(nx);
    GL_UNUSED(ny);
    GL_UNUSED(nz);
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3bv(const GLbyte *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz) {
    if (GlCurrentContext) {
        GlCurrentContext->IVertex.Normal[X] = nx;
        GlCurrentContext->IVertex.Normal[Y] = ny;
        GlCurrentContext->IVertex.Normal[Z] = nz;
        GlCurrentContext->IVertex.Normal[W] = 0.0;
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3dv(const GLdouble *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz) {
    if (GlCurrentContext) {
        GlCurrentContext->IVertex.Normal[X] = (GLdouble)nx;
        GlCurrentContext->IVertex.Normal[Y] = (GLdouble)ny;
        GlCurrentContext->IVertex.Normal[Z] = (GLdouble)nz;
        GlCurrentContext->IVertex.Normal[W] = 0.0;
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3fv(const GLfloat *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3i(GLint nx, GLint ny, GLint nz) {
    GL_UNUSED(nx);
    GL_UNUSED(ny);
    GL_UNUSED(nz);
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3iv(const GLint *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3s(GLshort nx, GLshort ny, GLshort nz) {
    if (GlCurrentContext) {
        GlCurrentContext->IVertex.Normal[X] = (GLdouble)nx / (GLdouble)32767.0;
        GlCurrentContext->IVertex.Normal[Y] = (GLdouble)ny / (GLdouble)32767.0;
        GlCurrentContext->IVertex.Normal[Z] = (GLdouble)nz / (GLdouble)32767.0;
        GlCurrentContext->IVertex.Normal[W] = 0.0;
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3sv(const GLshort *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glOrtho(GLdouble left, GLdouble right, GLdouble bottom,
                             GLdouble top, GLdouble znear, GLdouble zfar) {
    GLdouble tmp[4][4];
    GLMATRIX4D *CNMatrix;
    GLMATRIX4D *CIMatrix;

    if (!glCheckState())
        return;

    if (0 == (right - left))
        right += 0.001;
    if (0 == (top - bottom))
        top += 0.001;
    if (0 == (zfar - znear))
        zfar += 0.001;

    tmp[0][0] = 2.0 / (right - left);
    tmp[0][1] = 0.0;
    tmp[0][2] = 0.0;
    tmp[0][3] = 0.0 - ((right + left) / (right - left));

    tmp[1][0] = 0.0;
    tmp[1][1] = 2.0 / (top - bottom);
    tmp[1][2] = 0.0;
    tmp[1][3] = 0.0 - ((top + bottom) / (top - bottom));

    tmp[2][0] = 0.0;
    tmp[2][1] = 0.0;
    tmp[2][2] = 0.0 - (2.0 / (zfar - znear));
    tmp[2][3] = 0.0 - ((zfar + znear) / (zfar - znear));

    tmp[3][0] = 0.0;
    tmp[3][1] = 0.0;
    tmp[3][2] = 0.0;
    tmp[3][3] = 1.0;

    CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
    CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

    glMatrix4dConcat(*CNMatrix, *CNMatrix, tmp, GL_MATOP_POSTCONCATENATE);
    glMatrix4dInverse(*CIMatrix, *CNMatrix);
    glSyncCurrentMatrixStackEntry(GlCurrentContext);

    if (GlCurrentContext->Block.XForm.MatrixMode == GL_MATRIX_PROJECTION) {
        glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
    }
}

/**********************************************************************************************/

EXPORT GLint APIENTRY glRenderMode(GLenum mode) {
    GL_UNUSED(mode);
    return 0;
}

/**********************************************************************************************/

EXPORT void APIENTRY glPopMatrix() {
    GLenum MatrixMode;
    GLI32 StackDepth;

    if (!glCheckState())
        return;

    MatrixMode = GlCurrentContext->Block.XForm.MatrixMode;
    StackDepth = GlCurrentContext->Block.XForm.MatrixStackDepth[MatrixMode];

    if (StackDepth <= 1) {
        glSetErrorCode(GL_STACK_UNDERFLOW);
        return;
    }

    StackDepth--;
    GlCurrentContext->Block.XForm.MatrixStackDepth[MatrixMode] = StackDepth;

    glCopyMatrix4d(GlCurrentContext->Block.XForm.NMatrix[MatrixMode],
                   GlCurrentContext->Block.XForm.NMatrixStack[MatrixMode]
                                                          [StackDepth - 1]);
    glCopyMatrix4d(GlCurrentContext->Block.XForm.IMatrix[MatrixMode],
                   GlCurrentContext->Block.XForm.IMatrixStack[MatrixMode]
                                                          [StackDepth - 1]);

    if (MatrixMode == GL_MATRIX_PROJECTION) {
        glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glPushMatrix() {
    GLenum MatrixMode;
    GLI32 StackDepth;
    GLI32 MaxDepth;

    if (!glCheckState())
        return;

    MatrixMode = GlCurrentContext->Block.XForm.MatrixMode;
    StackDepth = GlCurrentContext->Block.XForm.MatrixStackDepth[MatrixMode];
    MaxDepth = glGetMatrixStackMaxDepth(MatrixMode);

    if (StackDepth >= MaxDepth) {
        glSetErrorCode(GL_STACK_OVERFLOW);
        return;
    }

    glCopyMatrix4d(GlCurrentContext->Block.XForm.NMatrixStack[MatrixMode]
                                                          [StackDepth],
                   GlCurrentContext->Block.XForm.NMatrix[MatrixMode]);
    glCopyMatrix4d(GlCurrentContext->Block.XForm.IMatrixStack[MatrixMode]
                                                          [StackDepth],
                   GlCurrentContext->Block.XForm.IMatrix[MatrixMode]);

    GlCurrentContext->Block.XForm.MatrixStackDepth[MatrixMode] = StackDepth + 1;
}

/**********************************************************************************************/

EXPORT void APIENTRY glRotated(GLdouble angle, GLdouble x, GLdouble y,
                               GLdouble z) {
    GLdouble Pnt1[3];
    GLdouble Pnt2[3];
    GLdouble tmp[4][4];
    GLMATRIX4D *CNMatrix;
    GLMATRIX4D *CIMatrix;
    GLdouble RadianAngle;

    if (!glCheckState())
        return;

    Pnt1[X] = 0;
    Pnt1[Y] = 0;
    Pnt1[Z] = 0;
    Pnt2[X] = x;
    Pnt2[Y] = y;
    Pnt2[Z] = z;

    RadianAngle = (angle * GL_PI) / 180.0;

    glMatrix4dRotationLine(tmp, RadianAngle, Pnt1, Pnt2);

    CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
    CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

    glMatrix4dConcat(*CNMatrix, *CNMatrix, tmp, GL_MATOP_POSTCONCATENATE);
    glMatrix4dInverse(*CIMatrix, *CNMatrix);
    glSyncCurrentMatrixStackEntry(GlCurrentContext);

    if (GlCurrentContext->Block.XForm.MatrixMode == GL_MATRIX_PROJECTION) {
        glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    glRotated((GLdouble)angle, (GLdouble)x, (GLdouble)y, (GLdouble)z);
}

/**********************************************************************************************/

EXPORT void APIENTRY glScaled(GLdouble x, GLdouble y, GLdouble z) {
    GLdouble scl[3];
    GLdouble tmp[4][4];
    GLMATRIX4D *CNMatrix;
    GLMATRIX4D *CIMatrix;

    if (!glCheckState())
        return;

    scl[X] = x;
    scl[Y] = y;
    scl[Z] = z;
    glCreateScalingMatrix4d(tmp, scl);

    CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
    CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

    glMatrix4dConcat(*CNMatrix, *CNMatrix, tmp, GL_MATOP_POSTCONCATENATE);
    glMatrix4dInverse(*CIMatrix, *CNMatrix);
    glSyncCurrentMatrixStackEntry(GlCurrentContext);

    if (GlCurrentContext->Block.XForm.MatrixMode == GL_MATRIX_PROJECTION) {
        glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z) {
    glScaled((GLdouble)x, (GLdouble)y, (GLdouble)z);
}

/**********************************************************************************************/

EXPORT void APIENTRY glScissor(GLint x, GLint y, GLsizei width,
                               GLsizei height) {
    GL_UNUSED(x);
    GL_UNUSED(y);
    GL_UNUSED(width);
    GL_UNUSED(height);
}

/**********************************************************************************************/

EXPORT void APIENTRY glSelectBuffer(GLsizei size, GLuint *buffer) {
    GL_UNUSED(size);
    GL_UNUSED(buffer);
}

/**********************************************************************************************/

EXPORT void APIENTRY glShadeModel(GLenum mode) { GL_UNUSED(mode); }

/**********************************************************************************************/

EXPORT void APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask) {
    GL_UNUSED(func);
    GL_UNUSED(ref);
    GL_UNUSED(mask);
}

/**********************************************************************************************/

EXPORT void APIENTRY glStencilMask(GLuint mask) { GL_UNUSED(mask); }

/**********************************************************************************************/

EXPORT void APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
    GL_UNUSED(fail);
    GL_UNUSED(zfail);
    GL_UNUSED(zpass);
}

/**********************************************************************************************/

EXPORT void APIENTRY glTranslated(GLdouble x, GLdouble y, GLdouble z) {
    GLdouble vec[3];
    GLdouble tmp[4][4];
    GLMATRIX4D *CNMatrix;
    GLMATRIX4D *CIMatrix;

    if (!glCheckState())
        return;

    vec[X] = x;
    vec[Y] = y;
    vec[Z] = z;
    glCreateTranslationMatrix4d(tmp, vec);

    CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
    CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

    glMatrix4dConcat(*CNMatrix, *CNMatrix, tmp, GL_MATOP_POSTCONCATENATE);
    glMatrix4dInverse(*CIMatrix, *CNMatrix);
    glSyncCurrentMatrixStackEntry(GlCurrentContext);

    if (GlCurrentContext->Block.XForm.MatrixMode == GL_MATRIX_PROJECTION) {
        glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z) {
    glTranslated((GLdouble)x, (GLdouble)y, (GLdouble)z);
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2d(GLdouble x, GLdouble y) {
    if (!GlInputPrimitive)
        return;

    if (GlCurrentContext) {
        GlCurrentContext->IVertex.World[X] = x;
        GlCurrentContext->IVertex.World[Y] = y;
        GlCurrentContext->IVertex.World[Z] = 0.0;
        GlCurrentContext->IVertex.World[W] = 1.0;
        glAddInputVertexToInputPolygon(GlCurrentContext);
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2dv(const GLdouble *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2f(GLfloat x, GLfloat y) {
    if (!GlInputPrimitive)
        return;

    if (GlCurrentContext) {
        GlCurrentContext->IVertex.World[X] = (GLdouble)x;
        GlCurrentContext->IVertex.World[Y] = (GLdouble)y;
        GlCurrentContext->IVertex.World[Z] = 0.0;
        GlCurrentContext->IVertex.World[W] = 1.0;
        glAddInputVertexToInputPolygon(GlCurrentContext);
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2fv(const GLfloat *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2i(GLint x, GLint y) {
    GL_UNUSED(x);
    GL_UNUSED(y);
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2iv(const GLint *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2s(GLshort x, GLshort y) {
    GL_UNUSED(x);
    GL_UNUSED(y);
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2sv(const GLshort *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3d(GLdouble x, GLdouble y, GLdouble z) {
    if (!GlInputPrimitive)
        return;

    if (GlCurrentContext) {
        GlCurrentContext->IVertex.World[X] = x;
        GlCurrentContext->IVertex.World[Y] = y;
        GlCurrentContext->IVertex.World[Z] = z;
        GlCurrentContext->IVertex.World[W] = 1.0;
        glAddInputVertexToInputPolygon(GlCurrentContext);
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3dv(const GLdouble *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3f(GLfloat x, GLfloat y, GLfloat z) {
    if (!GlInputPrimitive)
        return;

    if (GlCurrentContext) {
        GlCurrentContext->IVertex.World[X] = (GLdouble)x;
        GlCurrentContext->IVertex.World[Y] = (GLdouble)y;
        GlCurrentContext->IVertex.World[Z] = (GLdouble)z;
        GlCurrentContext->IVertex.World[W] = 1.0;
        glAddInputVertexToInputPolygon(GlCurrentContext);
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3fv(const GLfloat *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3i(GLint x, GLint y, GLint z) {
    if (!GlInputPrimitive)
        return;

    if (GlCurrentContext) {
        GlCurrentContext->IVertex.World[X] = (GLdouble)x;
        GlCurrentContext->IVertex.World[Y] = (GLdouble)y;
        GlCurrentContext->IVertex.World[Z] = (GLdouble)z;
        GlCurrentContext->IVertex.World[W] = 1.0;
        glAddInputVertexToInputPolygon(GlCurrentContext);
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3iv(const GLint *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3s(GLshort x, GLshort y, GLshort z) {
    GL_UNUSED(x);
    GL_UNUSED(y);
    GL_UNUSED(z);
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3sv(const GLshort *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4d(GLdouble x, GLdouble y, GLdouble z,
                                GLdouble w) {
    if (!GlInputPrimitive)
        return;

    if (GlCurrentContext) {
        GlCurrentContext->IVertex.World[X] = x;
        GlCurrentContext->IVertex.World[Y] = y;
        GlCurrentContext->IVertex.World[Z] = z;
        GlCurrentContext->IVertex.World[W] = w;
        glAddInputVertexToInputPolygon(GlCurrentContext);
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4dv(const GLdouble *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    if (!GlInputPrimitive)
        return;

    if (GlCurrentContext) {
        GlCurrentContext->IVertex.World[X] = (GLdouble)x;
        GlCurrentContext->IVertex.World[Y] = (GLdouble)y;
        GlCurrentContext->IVertex.World[Z] = (GLdouble)z;
        GlCurrentContext->IVertex.World[W] = (GLdouble)w;
        glAddInputVertexToInputPolygon(GlCurrentContext);
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4fv(const GLfloat *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4i(GLint x, GLint y, GLint z, GLint w) {
    if (!GlInputPrimitive)
        return;

    if (GlCurrentContext) {
        GlCurrentContext->IVertex.World[X] = (GLdouble)x;
        GlCurrentContext->IVertex.World[Y] = (GLdouble)y;
        GlCurrentContext->IVertex.World[Z] = (GLdouble)z;
        GlCurrentContext->IVertex.World[W] = (GLdouble)w;
        glAddInputVertexToInputPolygon(GlCurrentContext);
    }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4iv(const GLint *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w) {
    GL_UNUSED(x);
    GL_UNUSED(y);
    GL_UNUSED(z);
    GL_UNUSED(w);
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4sv(const GLshort *v) { GL_UNUSED(v); }

/**********************************************************************************************/

EXPORT void APIENTRY glViewport(GLint x, GLint y, GLsizei width,
                                GLsizei height) {
    LPGLRENDERCONTEXT rc = GlCurrentContext;

    if (!glCheckState())
        return;

    // Check if the size really changes
    if (rc->ViewPort[0] == x && rc->ViewPort[1] == y &&
        rc->ViewPort[2] == width && rc->ViewPort[3] == height)
        return;

    rc->ViewPort[0] = x;
    rc->ViewPort[1] = y;
    rc->ViewPort[2] = width;
    rc->ViewPort[3] = height;
    rc->Width = width;
    rc->Height = height;

    glResizeContext(rc);
}
