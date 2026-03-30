
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

#include <stdio.h>
#include <math.h>

#include "tinygl.h"

#include "gllist.h"
#include "glvect.h"
#include "glmain.h"
#include "glrast.h"

/**********************************************************************************************/

// Global variables

HINSTANCE             GlLibraryInstance;

GLI32                 GlNumContext;
HGLOBAL               GlRenderContext[GL_MAX_CONTEXT];

HGLRC                 GlCurrentHandle;
LPGLRENDERCONTEXT     GlCurrentContext;

GLI32                 GlNumAllocs;

/**********************************************************************************************/

GLboolean             GlInputPrimitive;
GLenum                GlPrimitiveType;
GLenum                GlNumVertProcessed;

/**********************************************************************************************/

// Variables local to this module

static char szTemp [256];

/**********************************************************************************************/

EXPORT HGLRC WINAPI tinyglCreateContext (HDC);
EXPORT BOOL  WINAPI tinyglDeleteContext (HGLRC);
EXPORT HGLRC WINAPI tinyglGetCurrentContext (VOID);
EXPORT HDC   WINAPI tinyglGetCurrentDC (VOID);
EXPORT PROC  WINAPI tinyglGetProcAddress (LPCSTR);
EXPORT BOOL  WINAPI tinyglMakeCurrent (HDC, HGLRC);
EXPORT BOOL  WINAPI SwapBuffers (HDC);

/**********************************************************************************************/

static GLboolean glCheckState ()
{
  if (GlCurrentContext==NULL) return FALSE;
  if (GlInputPrimitive==TRUE)
  {
    SetLastError(USERERR|GL_INVALID_OPERATION);
    return FALSE;
  }
  return TRUE;
}

/**********************************************************************************************/

GLvoid* glMalloc (GLsizei Size)
{
  GLvoid* Pointer;
  Pointer=(GLvoid*)malloc(Size);
  if (Pointer) GlNumAllocs++;
  return Pointer;
}

/**********************************************************************************************/

void glFree (GLvoid* Pointer)
{
  if (Pointer) { free(Pointer); GlNumAllocs--; }
  if (GlNumAllocs!=0) SetLastError(USERERR|GL_INVALID_OPERATION);
}

/**********************************************************************************************/

void glDestroyPolygonListItem (GLvoid* Data)
{
  LPGLPOLYGON Poly=(LPGLPOLYGON)Data;
  glFree(Poly->Vertex);
  glFree(Poly);
}

/**********************************************************************************************/

void glDestroyRenderListItem (GLvoid* Data)
{
  LPGLRENDERBLOCK Block=(LPGLRENDERBLOCK)Data;
  glList_Destroy(&(Block->PolygonList));
  glFree(Block);
}

/**********************************************************************************************/

GLboolean glCreatePolygon (LPGLPOLYGON poly, GLenum numvert)
{
  LPGLVERTEX    v;

  if (poly==NULL) return FALSE;

  poly->NumVertex=0;
  poly->Vertex=NULL;

  v=(LPGLVERTEX)glMalloc(sizeof(GLVERTEX)*numvert);
  if (v==NULL) return FALSE;

  poly->NumVertex=numvert;
  poly->Vertex=v;

  return TRUE;
}

/**********************************************************************************************/

GLboolean glDestroyPolygon (LPGLPOLYGON poly)
{
  if (poly==NULL) return FALSE;

  poly->NumVertex=0;
  glFree(poly->Vertex);
  poly->Vertex=NULL;

  return TRUE;
}

/**********************************************************************************************/

LPGLPOLYGON glAllocatePolygon (GLenum numvert)
{
  LPGLPOLYGON   poly;

  poly=(LPGLPOLYGON)glMalloc(sizeof(GLPOLYGON));
  if (poly==NULL) return NULL;

  if (glCreatePolygon(poly, numvert)==FALSE)
  {
    glFree(poly);
    return NULL;
  }

  return poly;
}

/**********************************************************************************************/

static void glMatrixGLtoMatrix4d (const GLdouble* m1, GLdouble* m2)
{
  long c, d;
  for (d=0; d<4; d++) for (c=0; c<4; c++) m2[d*4+c]=m1[c*4+d];
}

/**********************************************************************************************/

static void glMatrix4dtoMatrixGL (const GLdouble* m1, GLdouble* m2)
{
  long c, d;
  for (d=0; d<4; d++) for (c=0; c<4; c++) m2[d*4+c]=m1[c*4+d];
}

/**********************************************************************************************/

void glComputeFrustum (LPGLRENDERCONTEXT Context, LPGLRENDERBLOCK Block)
{
  GLdouble      Pos [4];
  GLdouble      Nor [4];
  GLdouble      Box [8][4];
  GLint         Idx [6][4];
  GLI32         c;

  GLdouble MINX = -0.8;
  GLdouble MAXX =  0.8;
  GLdouble MINY = -0.8;
  GLdouble MAXY =  0.8;
  GLdouble MINZ =  0.8;
  GLdouble MAXZ =  0.8;

  MINX*=(GLdouble)Context->ViewPort[2]/2.0;
  MAXX*=(GLdouble)Context->ViewPort[2]/2.0;

  MINY*=(GLdouble)Context->ViewPort[3]/2.0;
  MAXY*=(GLdouble)Context->ViewPort[3]/2.0;

  Box[0][X]=MINX; Box[0][Y]=MINY; Box[0][Z]=MINZ; Box[0][W]=1.0;
  Box[1][X]=MAXX; Box[1][Y]=MINY; Box[1][Z]=MINZ; Box[1][W]=1.0;
  Box[2][X]=MINX; Box[2][Y]=MINY; Box[2][Z]=MAXZ; Box[2][W]=1.0;
  Box[3][X]=MAXX; Box[3][Y]=MINY; Box[3][Z]=MAXZ; Box[3][W]=1.0;
  Box[4][X]=MINX; Box[4][Y]=MAXY; Box[4][Z]=MINZ; Box[4][W]=1.0;
  Box[5][X]=MAXX; Box[5][Y]=MAXY; Box[5][Z]=MINZ; Box[5][W]=1.0;
  Box[6][X]=MINX; Box[6][Y]=MAXY; Box[6][Z]=MAXZ; Box[6][W]=1.0;
  Box[7][X]=MAXX; Box[7][Y]=MAXY; Box[7][Z]=MAXZ; Box[7][W]=1.0;

  // Left plane
  Idx[0][0]=0; Idx[0][1]=2; Idx[0][2]=6; Idx[0][3]=4;
  // Idx[0][0]=4; Idx[0][1]=6; Idx[0][2]=2; Idx[0][3]=0;

  // Right plane
  Idx[1][0]=3; Idx[1][1]=1; Idx[1][2]=5; Idx[1][3]=7;
  // Idx[1][0]=7; Idx[1][1]=5; Idx[1][2]=1; Idx[1][3]=3;

  // Bottom plane
  Idx[2][0]=1; Idx[2][1]=3; Idx[2][2]=2; Idx[2][3]=0;
  // Idx[2][0]=0; Idx[2][1]=2; Idx[2][2]=3; Idx[2][3]=1;

  // Top plane
  Idx[3][0]=4; Idx[3][1]=6; Idx[3][2]=7; Idx[3][3]=5;
  // Idx[3][0]=5; Idx[3][1]=7; Idx[3][2]=6; Idx[3][3]=4;

  // Near plane
  Idx[4][0]=0; Idx[4][1]=4; Idx[4][2]=5; Idx[4][3]=1;
  // Idx[4][0]=1; Idx[4][1]=5; Idx[4][2]=4; Idx[4][3]=0;

  // Far plane
  Idx[5][0]=3; Idx[5][1]=7; Idx[5][2]=6; Idx[5][3]=2;
  // Idx[5][0]=2; Idx[5][1]=6; Idx[5][2]=7; Idx[5][3]=3;

  /*
  for (c=0; c<8; c++)
  {
    glMatrix4dTransVector4d(Box[c], Box[c], Block->XForm.NMatrix[GL_MATRIX_PROJECTION]);

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

  for (c=0; c<6; c++)
  {
    Pos[X]=(Box[Idx[c][0]][X]+Box[Idx[c][1]][X]+Box[Idx[c][2]][X]+Box[Idx[c][3]][X])/4.0;
    Pos[Y]=(Box[Idx[c][0]][Y]+Box[Idx[c][1]][Y]+Box[Idx[c][2]][Y]+Box[Idx[c][3]][Y])/4.0;
    Pos[Z]=(Box[Idx[c][0]][Z]+Box[Idx[c][1]][Z]+Box[Idx[c][2]][Z]+Box[Idx[c][3]][Z])/4.0;
    Pos[W]=(Box[Idx[c][0]][W]+Box[Idx[c][1]][W]+Box[Idx[c][2]][W]+Box[Idx[c][3]][W])/4.0;

    glVector3dTriangleNormal(Nor, Box[Idx[c][0]], Box[Idx[c][1]], Box[Idx[c][2]]);

    glSetupSingleClipPlane(&(Block->XForm.Plane[c]), Pos, Nor);
  }
}

/**********************************************************************************************/

static GLboolean glInitLibrary (HINSTANCE hInstance)
{
  GLU32 c;

  GlLibraryInstance     = hInstance;
  GlNumAllocs           = 0;

  GlNumContext          = 0;

  GlCurrentHandle       = NULL;
  GlCurrentContext      = NULL;

  GlInputPrimitive      = FALSE;

  // Clear the render context handles
  for (c=0; c<GL_MAX_CONTEXT; c++) GlRenderContext[c]=NULL;

  return TRUE;
}

/**********************************************************************************************/

static void glDeInitLibrary ()
{
  glDestroyAllContexts();
}

/**********************************************************************************************/

static void glResizeContext (LPGLRENDERCONTEXT rc)
{
  if (rc==NULL) return;

  // Free the current depth buffer
  glFree(rc->DepthBuffer);
  rc->DepthBuffer=NULL;

  // Compute the size in bytes of the depth buffer
  rc->DepthBufferSize=rc->Width*rc->Height;

  // Allocate memory for the depth buffer
  rc->DepthBuffer=(GLU8*)glMalloc(rc->DepthBufferSize*sizeof(GLfloat));

  // Did the allocation fail ?
  if (rc->DepthBuffer==NULL)
  {
    SetLastError(USERERR|GL_OUT_OF_MEMORY);
  }
}

/**********************************************************************************************/

void glResetVertex (LPGLVERTEX Vertex)
{
  Vertex->World[X]      =0.0;
  Vertex->World[Y]      =0.0;
  Vertex->World[Z]      =0.0;
  Vertex->World[W]      =1.0;

  Vertex->Color[R]      =1.0;
  Vertex->Color[G]      =1.0;
  Vertex->Color[B]      =1.0;
  Vertex->Color[A]      =1.0;

  Vertex->Normal[X]     =0.0;
  Vertex->Normal[Y]     =0.0;
  Vertex->Normal[Z]     =1.0;
  Vertex->Normal[W]     =0.0;

  Vertex->Tex[X]        =0.0;
  Vertex->Tex[Y]        =0.0;
}

/**********************************************************************************************/

void glResetMaterial (LPGLMATERIAL Material)
{
  Material->Ambient[R]  =0.2;
  Material->Ambient[G]  =0.2;
  Material->Ambient[B]  =0.2;
  Material->Ambient[A]  =1.0;

  Material->Diffuse[R]  =0.8;
  Material->Diffuse[G]  =0.8;
  Material->Diffuse[B]  =0.8;
  Material->Diffuse[A]  =1.0;

  Material->Specular[R] =0.0;
  Material->Specular[G] =0.0;
  Material->Specular[B] =0.0;
  Material->Specular[A] =1.0;

  Material->Emission[R] =0.0;
  Material->Emission[G] =0.0;
  Material->Emission[B] =0.0;
  Material->Emission[A] =1.0;

  Material->Shininess   =0.0;
}

/**********************************************************************************************/

void glResetPolygon (LPGLPOLYGON Polygon)
{
  Polygon->NumVertex=0;
  glResetMaterial(&(Polygon->FrontFace));
  glResetMaterial(&(Polygon->BackFace));
}

/**********************************************************************************************/

static void glInitRenderContext (LPGLRENDERCONTEXT rc)
{
  GLint c;

  rc->Block.RenderFunc.RenderMode       = 1;

  rc->Block.RenderFlag.Alpha            = FALSE;
  rc->Block.RenderFlag.Blend            = FALSE;
  rc->Block.RenderFlag.Depth            = FALSE;
  rc->Block.RenderFlag.Dither           = FALSE;
  rc->Block.RenderFlag.EdgeFlag         = FALSE;
  rc->Block.RenderFlag.Fog              = FALSE;

  rc->Block.RenderFunc.AlphaFunc        = GL_ALWAYS;

  rc->Block.RenderFunc.BlendFuncSFactor = GL_SRC_ALPHA;
  rc->Block.RenderFunc.BlendFuncDFactor = GL_ONE_MINUS_SRC_ALPHA;

  rc->Block.RenderFunc.RMask            = TRUE;
  rc->Block.RenderFunc.GMask            = TRUE;
  rc->Block.RenderFunc.BMask            = TRUE;
  rc->Block.RenderFunc.AMask            = TRUE;

  rc->Block.RenderFunc.DepthMask        = TRUE;
  rc->Block.RenderFunc.DepthFunc        = GL_LESS;
  rc->Block.RenderFunc.DepthRangeNear   = 0.0;
  rc->Block.RenderFunc.DepthRangeFar    = 1.0;
  rc->Block.RenderFunc.ClearDepth       = 1.0;

  rc->Block.RenderFunc.FogMode          = GL_EXP;
  rc->Block.RenderFunc.FogDensity       = 1.0;
  rc->Block.RenderFunc.FogStart         = 0.0;
  rc->Block.RenderFunc.FogEnd           = 1.0;
  rc->Block.RenderFunc.FogIndex         = 0.0;
  rc->Block.RenderFunc.FogColor[R]      = 0.0;
  rc->Block.RenderFunc.FogColor[G]      = 0.0;
  rc->Block.RenderFunc.FogColor[B]      = 0.0;
  rc->Block.RenderFunc.FogColor[A]      = 0.0;

  rc->Block.XForm.MatrixMode            = GL_MATRIX_MODELVIEW;

  for (c=0; c<3; c++)
  {
    glMatrix4dIdentity(&(rc->Block.XForm.NMatrix[c][0][0]));
    glMatrix4dInverse(&(rc->Block.XForm.IMatrix[c][0][0]),
                      &(rc->Block.XForm.NMatrix[c][0][0]));
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

  for (c=0; c<GL_MAX_USER_PLANES; c++)
  {
    rc->Block.UserPlane[c].On                   = FALSE;
    rc->Block.UserPlane[c].Normal[X]            = 0.0;
    rc->Block.UserPlane[c].Normal[Y]            = 0.0;
    rc->Block.UserPlane[c].Normal[Z]            = 1.0;
    rc->Block.UserPlane[c].Normal[W]            = 1.0;
    rc->Block.UserPlane[c].Distance             = 0.0;
  }

  for (c=0; c<GL_MAX_USER_LIGHTS; c++)
  {
    rc->Block.Light[c].On               =    FALSE;

    rc->Block.Light[c].Ambient[R]       =    0.0;
    rc->Block.Light[c].Ambient[G]       =    0.0;
    rc->Block.Light[c].Ambient[B]       =    0.0;
    rc->Block.Light[c].Ambient[A]       =    1.0;

    rc->Block.Light[c].Diffuse[R]       =    1.0;
    rc->Block.Light[c].Diffuse[G]       =    1.0;
    rc->Block.Light[c].Diffuse[B]       =    1.0;
    rc->Block.Light[c].Diffuse[A]       =    1.0;

    rc->Block.Light[c].Specular[R]      =    1.0;
    rc->Block.Light[c].Specular[G]      =    1.0;
    rc->Block.Light[c].Specular[B]      =    1.0;
    rc->Block.Light[c].Specular[A]      =    1.0;

    rc->Block.Light[c].Position[X]      =    0.0;
    rc->Block.Light[c].Position[Y]      =    0.0;
    rc->Block.Light[c].Position[Z]      =    1.0;
    rc->Block.Light[c].Position[W]      =    0.0;

    rc->Block.Light[c].Direction[X]     =    0.0;
    rc->Block.Light[c].Direction[Y]     =    0.0;
    rc->Block.Light[c].Direction[Z]     =   -1.0;

    rc->Block.Light[c].SpotExponent     =    0.0;
    rc->Block.Light[c].SpotCutOff       =  180.0;

    rc->Block.Light[c].ConstantAtten    =    1.0;
    rc->Block.Light[c].LinearAtten      =    0.0;
    rc->Block.Light[c].QuadraticAtten   =    0.0;
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

static void glDestroySingleContext (HGLRC hrc)
{
  LPGLRENDERCONTEXT rc;

  if (hrc==NULL) return;

  // Check if this is the current context
  // If so unlock the handle to the global memory object
  if (hrc==GlCurrentHandle)
  {
    GlobalUnlock(hrc);
    GlCurrentHandle=NULL;
    GlCurrentContext=NULL;
  }

  // Get a pointer to this context
  rc=(LPGLRENDERCONTEXT)GlobalLock(hrc);

  if (rc)
  {
    // Destroy the rendering list
    // glList_Destroy(&(rc->RenderList));

    // Destroy the depth buffer
    glFree(rc->DepthBuffer);

    // Destroy the DIB section
    DeleteObject(rc->DIBSection.Handle);

    // Destroy the input polygon
    glFree(rc->IPolygon.Vertex);

    GlobalUnlock(hrc);
  }

  GlobalFree(hrc);
}

/**********************************************************************************************/

static void glDestroyAllContexts ()
{
  LPGLRENDERCONTEXT     rc;
  long                  c;

  for (c=0; c<GL_MAX_CONTEXT; c++)
  {
    if (GlRenderContext[c]!=NULL)
    {
      glDestroySingleContext(GlRenderContext[c]);

      GlRenderContext[c]=NULL;

      GlNumContext--;
    }
  }

  if (GlNumContext!=0) SetLastError(USERERR|GL_INVALID_OPERATION);
}

/**********************************************************************************************/

static void glEnableDisable (GLenum cap, GLboolean val)
{
  LPGLRENDERCONTEXT rc=GlCurrentContext;

  // Check if this is a light name
  if (cap>=GL_LIGHT0 && cap<GL_LIGHT0+GL_MAX_USER_LIGHTS)
  {
    cap-=GL_LIGHT0;
    rc->Block.Light[cap].On=val;
    return;
  }

  // Check if this is a clipping plane name
  if (cap>=GL_CLIP_PLANE0 && cap<GL_CLIP_PLANE0+GL_MAX_USER_PLANES)
  {
    cap-=GL_CLIP_PLANE0;
    rc->Block.UserPlane[cap].On=val;
    return;
  }

  switch (cap)
  {
    case GL_ALPHA_TEST:         rc->Block.RenderFlag.Alpha          = val; return;
    case GL_AUTO_NORMAL:                                                   return;
    case GL_BLEND:              rc->Block.RenderFlag.Blend          = val; return;
    case GL_COLOR_MATERIAL:                                                return;
    case GL_CULL_FACE:          rc->Block.RenderFlag.CullFace       = val; return;
    case GL_DEPTH_TEST:         rc->Block.RenderFlag.Depth          = val; return;
    case GL_DITHER:             rc->Block.RenderFlag.Dither         = val; return;
    case GL_FOG:                rc->Block.RenderFlag.Fog            = val; return;
    case GL_LIGHTING:           rc->Block.RenderFlag.Lighting       = val; return;
    case GL_LINE_SMOOTH:        rc->Block.RenderFlag.LineSmooth     = val; return;
    case GL_LINE_STIPPLE:       rc->Block.RenderFlag.LineStipple    = val; return;
    case GL_LOGIC_OP:           rc->Block.RenderFlag.LogicOp        = val; return;
    case GL_MAP1_COLOR_4:       rc->Block.RenderFlag.Map1Color4     = val; return;
    case GL_MAP1_INDEX:         rc->Block.RenderFlag.Map1Index      = val; return;
    case GL_NORMALIZE:          rc->Block.RenderFlag.Normalize      = val; return;
    case GL_POINT_SMOOTH:       rc->Block.RenderFlag.PointSmooth    = val; return;
    case GL_POLYGON_SMOOTH:     rc->Block.RenderFlag.PolygonSmooth  = val; return;
    case GL_POLYGON_STIPPLE:    rc->Block.RenderFlag.PolygonStipple = val; return;
    case GL_SCISSOR_TEST:       rc->Block.RenderFlag.Scissor        = val; return;
    case GL_STENCIL_TEST:       rc->Block.RenderFlag.Stencil        = val; return;
    case GL_TEXTURE_1D:         rc->Block.RenderFlag.Texture1D      = val; return;
    case GL_TEXTURE_2D:         rc->Block.RenderFlag.Texture2D      = val; return;
    case GL_TEXTURE_GEN_Q:      rc->Block.RenderFlag.TextureGenQ    = val; return;
    case GL_TEXTURE_GEN_R:      rc->Block.RenderFlag.TextureGenR    = val; return;
    case GL_TEXTURE_GEN_S:      rc->Block.RenderFlag.TextureGenS    = val; return;
    case GL_TEXTURE_GEN_T:      rc->Block.RenderFlag.TextureGenT    = val; return;
  }

  SetLastError(USERERR|GL_INVALID_ENUM);
}

/**********************************************************************************************/

GLdouble* glGetContextCurrentNMatrix (LPGLRENDERCONTEXT rc)
{
  return (GLdouble*)&(rc->Block.XForm.NMatrix[rc->Block.XForm.MatrixMode]);
}

/**********************************************************************************************/

GLdouble* glGetContextCurrentIMatrix (LPGLRENDERCONTEXT rc)
{
  return (GLdouble*)&(rc->Block.XForm.IMatrix[rc->Block.XForm.MatrixMode]);
}

/**********************************************************************************************/

void glAddInputPolygonToRenderList (LPGLRENDERCONTEXT rc)
{
  LPGLRENDERBLOCK       Block;
  LPGLPOLYGON           Poly;

  if (rc->IPolygon.NumVertex==0) return;

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

  rc->IPolygon.NumVertex=0;
}

/**********************************************************************************************/

void glCheckInputPolygon (LPGLRENDERCONTEXT rc)
{
  switch (GlPrimitiveType)
  {
    case GL_POINTS:
      if (rc->IPolygon.NumVertex==1) glAddInputPolygonToRenderList(rc);
      break;
    case GL_LINES:
      if (rc->IPolygon.NumVertex==2) glAddInputPolygonToRenderList(rc);
      break;
    case GL_LINE_STRIP:
      break;
    case GL_LINE_LOOP:
      break;
    case GL_TRIANGLES:
      if (rc->IPolygon.NumVertex==3) glAddInputPolygonToRenderList(rc);
      break;
    case GL_TRIANGLE_STRIP:
      break;
    case GL_TRIANGLE_FAN:
      break;
    case GL_QUADS:
      if (rc->IPolygon.NumVertex==4) glAddInputPolygonToRenderList(rc);
      break;
    case GL_QUAD_STRIP:
      break;
    case GL_POLYGON:
      break;
  }
}

/**********************************************************************************************/

void glAddInputVertexToInputPolygon (LPGLRENDERCONTEXT rc)
{
  GLdouble* CNMatrix;
  GLdouble* CIMatrix;
  GLenum nv=rc->IPolygon.NumVertex;

  if (nv<GL_MAX_POLY_VERTEX)
  {
    // Copy the vertex to the destination polygon
    memcpy(&(rc->IPolygon.Vertex[nv]), &(rc->IVertex), sizeof(GLVERTEX));

    // Get the current modelview matrix
    CNMatrix=(GLdouble*)rc->Block.XForm.NMatrix[GL_MATRIX_MODELVIEW];
    CIMatrix=(GLdouble*)rc->Block.XForm.IMatrix[GL_MATRIX_MODELVIEW];

    // Transform the vertex with the modelview matrix
    glMatrix4dTransVector4d
    (rc->IPolygon.Vertex[nv].Eye, rc->IPolygon.Vertex[nv].World, CNMatrix);

    // Transform the vertex normal with the modelview matrix
    glMatrix4dTransVector4d
    (rc->IPolygon.Vertex[nv].Normal, rc->IPolygon.Vertex[nv].Normal, CNMatrix);

    rc->IPolygon.NumVertex++;
  }

  glCheckInputPolygon(rc);
}

/**********************************************************************************************/

EXPORT HGLRC APIENTRY tinyglCreateContext (HDC hDC)
{
  LPGLRENDERCONTEXT     rc;
  HGLOBAL               hrc;
  GLU8*                 DIBBase;
  GLU32                 c;
  GLboolean             Ok = 0;

  if (hDC==NULL || GlInputPrimitive || GlNumContext>=GL_MAX_CONTEXT) return NULL;

  // Allocate global memory for this context
  hrc=GlobalAlloc(GMEM_MOVEABLE, sizeof(GLRENDERCONTEXT));

  if (hrc)
  {
    // Store the handle in our global context handle array
    for (c=0; c<GL_MAX_CONTEXT; c++)
    {
      if (GlRenderContext[c]==NULL)
      {
        Ok = 1;
        GlRenderContext[c] = hrc;
        break;
      }
    }

    if (Ok==0) goto Error;

    // Get a pointer to this context
    rc=(LPGLRENDERCONTEXT)GlobalLock(hrc);

    // Set the context's ID
    rc->Id=0x0873;

    // Get information about the device context
    rc->Device.Handle           = hDC;
    rc->Device.Version          = GetDeviceCaps(hDC, DRIVERVERSION);
    rc->Device.Technology       = GetDeviceCaps(hDC, TECHNOLOGY);
    rc->Device.Width            = GetDeviceCaps(hDC, HORZRES);
    rc->Device.Height           = GetDeviceCaps(hDC, VERTRES);
    rc->Device.BitsPerPixel     = GetDeviceCaps(hDC, BITSPIXEL);
    rc->Device.NumPlanes        = GetDeviceCaps(hDC, PLANES);
    rc->Device.ColorRes         = GetDeviceCaps(hDC, COLORRES);
    rc->Device.RasterCaps       = GetDeviceCaps(hDC, RASTERCAPS);

    // Set viewport limits
    rc->ViewPort[0]             = 0;
    rc->ViewPort[1]             = 0;
    rc->ViewPort[2]             = 100;
    rc->ViewPort[3]             = 100;

    rc->Width                   = rc->ViewPort[2];
    rc->Height                  = rc->ViewPort[3];

    rc->BitmapInfo.bmiHeader.biSize             = sizeof(BITMAPINFOHEADER);
    rc->BitmapInfo.bmiHeader.biWidth            = rc->Device.Width;
    rc->BitmapInfo.bmiHeader.biHeight           = -2;
    rc->BitmapInfo.bmiHeader.biPlanes           =  1;
    // rc->BitmapInfo.bmiHeader.biBitCount=24;
    rc->BitmapInfo.bmiHeader.biBitCount         = 32;
    rc->BitmapInfo.bmiHeader.biCompression      = BI_RGB;
    rc->BitmapInfo.bmiHeader.biSizeImage        = 0;
    rc->BitmapInfo.bmiHeader.biXPelsPerMeter    = 0;
    rc->BitmapInfo.bmiHeader.biYPelsPerMeter    = 0;
    rc->BitmapInfo.bmiHeader.biClrUsed          = 0;
    rc->BitmapInfo.bmiHeader.biClrImportant     = 0;

    // Create a DIB section for the rasterizer
    rc->DIBSection.Handle=
    CreateDIBSection(rc->Device.Handle, &(rc->BitmapInfo), DIB_RGB_COLORS, &DIBBase, NULL, 0);

    // Is the DIB section created ?
    if (rc->DIBSection.Handle==NULL)
    {
      GlobalUnlock(hrc);
      SetLastError(USERERR|GL_OUT_OF_MEMORY);
      goto Error;
    }

    // Store the pointer to the bitmap plane
    rc->DIBSection.Base=DIBBase;

    // Compute the size in bytes of the depth buffer
    rc->DepthBufferSize=rc->Width*rc->Height;

    // Allocate memory for the depth buffer
    rc->DepthBuffer=(GLU8*)glMalloc(rc->DepthBufferSize*sizeof(GLfloat));

    // Did the allocation fail ?
    if (rc->DepthBuffer==NULL)
    {
      GlobalUnlock(hrc);
      SetLastError(USERERR|GL_OUT_OF_MEMORY);
      goto Error;
    }

    glInitRenderContext(rc);

    GlobalUnlock(hrc);
    GlNumContext++;
  }

  return hrc;

  Error:
  GlobalFree(hrc);
  return NULL;
}

/**********************************************************************************************/

EXPORT BOOL APIENTRY tinyglDeleteContext (HGLRC hrc)
{
  LPGLRENDERCONTEXT     rc;
  GLU32                 c;
  GLboolean             Found=0;

  if (hrc==NULL || GlInputPrimitive) return 0;

  // Find the handle of this context in our global context handle array
  for (c=0; c<GL_MAX_CONTEXT; c++)
  {
    if (GlRenderContext[c] == hrc)
    {
      Found = 1;
      glDestroySingleContext(GlRenderContext[c]);
      GlRenderContext[c] = NULL;
      GlNumContext--;
      break;
    }
  }

  // Check if the number of open contexts is valid
  if (GlNumContext<0)
  {
    GlNumContext = 0;
    SetLastError(USERERR|GL_INVALID_OPERATION);
  }

  return Found;
}

/**********************************************************************************************/

EXPORT HGLRC APIENTRY tinyglGetCurrentContext ()
{
  if (!glCheckState()) return NULL;

  if (GlCurrentHandle && GlCurrentContext)
  {
    return GlCurrentHandle;
  }

  return NULL;
}

/**********************************************************************************************/

EXPORT HDC APIENTRY tinyglGetCurrentDC ()
{
  if (!glCheckState()) return NULL;

  if (GlCurrentHandle && GlCurrentContext)
  {
    return GlCurrentContext->Device.Handle;
  }

  return NULL;
}

/**********************************************************************************************/

EXPORT PROC APIENTRY tinyglGetProcAddress (LPCSTR proc)
{
  return NULL;
}

/**********************************************************************************************/

EXPORT BOOL APIENTRY tinyglMakeCurrent (HDC hDC, HGLRC hRC)
{
  LPGLRENDERCONTEXT lpRC;

  if (hDC==NULL || GlInputPrimitive) return FALSE;

  // If we have a current context, unlock its handle
  if (GlCurrentHandle!=NULL)
  {
    GlobalUnlock(GlCurrentHandle);
    GlCurrentHandle  = NULL;
    GlCurrentContext = NULL;
  }

  // Just return if HRC is NULL
  if (hRC==NULL) return TRUE;

  // Get a pointer to this context
  lpRC=(LPGLRENDERCONTEXT)GlobalLock(hRC);

  if (lpRC!=NULL)
  {
    // Check context's ID
    if (lpRC->Id!=0x0873) goto Error;
    // if (rc->Device.Handle!=hdc) goto Error;
    lpRC->Device.Handle=hDC;
  }
  else return FALSE;

  GlCurrentHandle=hRC;
  GlCurrentContext=lpRC;

  return TRUE;

  Error:
  GlobalUnlock(hRC);
  return FALSE;
}

/**********************************************************************************************/

EXPORT BOOL APIENTRY SwapBuffers (HDC hdc)
{
  return FALSE;
}

/**********************************************************************************************/

EXPORT void APIENTRY glBegin (GLenum mode)
{
  LPGLRENDERBLOCK lpBlock;

  if (!glCheckState()) return;

  switch (mode)
  {
    case GL_POINTS         : GlPrimitiveType = GL_POINTS;         break;
    case GL_LINES          : GlPrimitiveType = GL_LINES;          break;
    case GL_LINE_STRIP     : GlPrimitiveType = GL_LINE_STRIP;     break;
    case GL_LINE_LOOP      : GlPrimitiveType = GL_LINE_LOOP;      break;
    case GL_TRIANGLES      : GlPrimitiveType = GL_TRIANGLES;      break;
    case GL_TRIANGLE_STRIP : GlPrimitiveType = GL_TRIANGLE_STRIP; break;
    case GL_TRIANGLE_FAN   : GlPrimitiveType = GL_TRIANGLE_FAN;   break;
    case GL_QUADS          : GlPrimitiveType = GL_QUADS;          break;
    case GL_QUAD_STRIP     : GlPrimitiveType = GL_QUAD_STRIP;     break;
    case GL_POLYGON        : GlPrimitiveType = GL_POLYGON;        break;
    default                : SetLastError(USERERR|GL_INVALID_ENUM); return;
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
  GlInputPrimitive=TRUE;
}

/**********************************************************************************************/

EXPORT void APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor)
{
  if (!glCheckState()) return;

  if (sfactor!=GL_ZERO && sfactor!=GL_ONE && sfactor!=GL_DST_COLOR &&
      sfactor!=GL_ONE_MINUS_DST_COLOR && sfactor!=GL_SRC_ALPHA &&
      sfactor!=GL_ONE_MINUS_SRC_ALPHA && sfactor!=GL_DST_ALPHA &&
      sfactor!=GL_ONE_MINUS_DST_ALPHA && sfactor!=GL_SRC_ALPHA_SATURATE)
  {
    SetLastError(USERERR|GL_INVALID_ENUM);
  }

  if (dfactor!=GL_ZERO && dfactor!=GL_ONE && dfactor!=GL_SRC_COLOR &&
      dfactor!=GL_ONE_MINUS_SRC_COLOR && dfactor!=GL_SRC_ALPHA &&
      dfactor!=GL_ONE_MINUS_SRC_ALPHA && dfactor!=GL_DST_ALPHA &&
      dfactor!=GL_ONE_MINUS_DST_ALPHA)
  {
    SetLastError(USERERR|GL_INVALID_ENUM);
  }

  GlCurrentContext->Block.RenderFunc.BlendFuncSFactor=sfactor;
  GlCurrentContext->Block.RenderFunc.BlendFuncDFactor=dfactor;
}

/**********************************************************************************************/

EXPORT void APIENTRY glClear (GLbitfield mask)
{
  GLfloat*      DepthBuffer;
  GLfloat       ClearDepth;
  GLint         c;

  if (!glCheckState()) return;

  if (mask & GL_DEPTH_BUFFER_BIT)
  {
    if (GlCurrentContext->DepthBuffer)
    {
      DepthBuffer=(GLfloat*)GlCurrentContext->DepthBuffer;
      ClearDepth=GlCurrentContext->Block.RenderFunc.ClearDepth;
      for (c=0; c<GlCurrentContext->DepthBufferSize; c++)
      {
        DepthBuffer[c]=ClearDepth;
      }
    }
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glClearAccum
(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glClearColor
(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glClearDepth (GLclampd depth)
{
  if (!glCheckState()) return;

  GlCurrentContext->Block.RenderFunc.ClearDepth=depth;
}

/**********************************************************************************************/

EXPORT void APIENTRY glClearIndex (GLfloat c)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glClearStencil (GLint s)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glClipPlane (GLenum plane, const GLdouble *equation)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3b (GLbyte red, GLbyte green, GLbyte blue)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3bv (const GLbyte *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3d (GLdouble red, GLdouble green, GLdouble blue)
{
  if (GlCurrentContext)
  {
    GlCurrentContext->IVertex.Color[R] = red;
    GlCurrentContext->IVertex.Color[G] = green;
    GlCurrentContext->IVertex.Color[B] = blue;
    GlCurrentContext->IVertex.Color[A] = 1.0;
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3dv (const GLdouble *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3f (GLfloat red, GLfloat green, GLfloat blue)
{
  if (GlCurrentContext)
  {
    GlCurrentContext->IVertex.Color[R] = red;
    GlCurrentContext->IVertex.Color[G] = green;
    GlCurrentContext->IVertex.Color[B] = blue;
    GlCurrentContext->IVertex.Color[A] = 1.0;
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3fv (const GLfloat *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3i (GLint red, GLint green, GLint blue)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3iv (const GLint *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3s (GLshort red, GLshort green, GLshort blue)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3sv (const GLshort *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3ub (GLubyte red, GLubyte green, GLubyte blue)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3ubv (const GLubyte *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3ui (GLuint red, GLuint green, GLuint blue)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3uiv (const GLuint *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3us (GLushort red, GLushort green, GLushort blue)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glColor3usv (const GLushort *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glDisable (GLenum cap)
{
  if (!glCheckState()) return;
  glEnableDisable(cap, FALSE);
}

/**********************************************************************************************/

EXPORT void APIENTRY glDrawBuffer (GLenum mode)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glEnable (GLenum cap)
{
  if (!glCheckState()) return;
  glEnableDisable(cap, TRUE);
}

/**********************************************************************************************/

EXPORT void APIENTRY glEnd ()
{
  LPGLRENDERBLOCK       Block;

  if (GlInputPrimitive==FALSE)
  {
    SetLastError(USERERR|GL_INVALID_OPERATION);
    return;
  }

  glAddInputPolygonToRenderList(GlCurrentContext);

  GlInputPrimitive=FALSE;

  /*
  if (!glCheckState()) return;

  Block=glList_Tail(&(GlCurrentContext->RenderList));

  if (Block)
  {
    memcpy(Block, &(GlCurrentContext->Block), sizeof(GLRENDERBLOCK)-sizeof(GLLIST));
  }
  else
  {
    SetLastError(USERERR|GL_INTERNAL_ERROR);
  }
  */
}

/**********************************************************************************************/

EXPORT void APIENTRY glFlush ()
{
  LPGLLIST              List;
  LPGLRENDERBLOCK       Block;
  GLenum                Count=0;

  if (!glCheckState()) return;

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

EXPORT void APIENTRY glFrustum
(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble znear, GLdouble zfar)
{
  GLdouble      tmp[4][4];
  GLdouble*     CNMatrix;
  GLdouble*     CIMatrix;

  if (!glCheckState()) return;

  if (0.0==(right-left)) right+=0.001;
  if (0.0==(top-bottom)) top+=0.001;
  if (0.0==(zfar-znear)) zfar+=0.001;

  tmp[0][0]     =  (2.0*znear)  / (right-left);
  tmp[0][1]     =  0.0;
  tmp[0][2]     =  (right+left) / (right-left);
  tmp[0][3]     =  0.0;

  tmp[1][0]     =  0.0;
  tmp[1][1]     =  (2.0*znear)  / (top-bottom);
  tmp[1][2]     =  (top+bottom) / (top-bottom);
  tmp[1][3]     =  0.0;

  tmp[2][0]     =  0.0;
  tmp[2][1]     =  0.0;
  tmp[2][2]     =  (2.0*znear)  / (zfar-znear);
  tmp[2][3]     = -(zfar+znear) / (zfar-znear);

  tmp[3][0]     =  0.0;
  tmp[3][1]     =  0.0;
  tmp[3][2]     =  1.0;
  tmp[3][3]     =  0.0;

  CNMatrix      = glGetContextCurrentNMatrix(GlCurrentContext);
  CIMatrix      = glGetContextCurrentIMatrix(GlCurrentContext);

  glMatrix4dConcat  (CNMatrix, CNMatrix, tmp, GL_MATOP_POSTCONCATENATE);
  glMatrix4dInverse (CIMatrix, CNMatrix);

  if (GlCurrentContext->Block.XForm.MatrixMode==GL_MATRIX_PROJECTION)
  {
    glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glGetBooleanv (GLenum pname, GLboolean *params)
{
  if (!glCheckState()) return;
}

/**********************************************************************************************/

EXPORT void APIENTRY glGetClipPlane (GLenum plane, GLdouble *equation)
{
  if (!glCheckState()) return;
}

/**********************************************************************************************/

EXPORT void APIENTRY glGetDoublev (GLenum pname, GLdouble* params)
{
  LPGLRENDERCONTEXT     rc;
  GLdouble*             pm1;
  GLdouble*             pm2;
  GLint                 c;

  if (!glCheckState()) return;

  rc=GlCurrentContext;

  if (params==NULL)
  {
    SetLastError(USERERR|GL_INVALID_OPERATION);
    return;
  }

  // Check if this is a light name
  if (pname>=GL_LIGHT0 && pname<GL_LIGHT0+GL_MAX_USER_LIGHTS)
  {
    params[0]=rc->Block.Light[pname-GL_LIGHT0].On ? 1.0 : 0.0;
    return;
  }

  switch (pname)
  {
    case GL_MODELVIEW_MATRIX:
    {
      pm1=(GLdouble*)&(rc->Block.XForm.NMatrix[GL_MATRIX_MODELVIEW]);
      for (c=0; c<16; c++) params[c]=pm1[c];
      return;
    }

    case GL_NORMALIZE:
    {
      params[0]=rc->Block.RenderFlag.Normalize ? 1.0 : 0.0;
      return;
    }

    case GL_PROJECTION_MATRIX:
    {
      pm1=(GLdouble*)&(rc->Block.XForm.NMatrix[GL_MATRIX_PROJECTION]);
      for (c=0; c<16; c++) params[c]=pm1[c];
      return;
    }
  }

  SetLastError(USERERR|GL_INVALID_ENUM);
}

/**********************************************************************************************/

EXPORT GLenum APIENTRY glGetError ()
{
  if (!glCheckState())
  {
    SetLastError(USERERR|GL_INVALID_OPERATION);
    return 0;
  }

  return ((GetLastError())&(~USERERR));
}

/**********************************************************************************************/

EXPORT void APIENTRY glGetFloatv (GLenum pname, GLfloat* params)
{
  LPGLRENDERCONTEXT     rc;
  GLdouble*             pm1;
  GLdouble*             pm2;
  GLint                 c;

  if (!glCheckState()) return;

  rc=GlCurrentContext;

  if (params==NULL)
  {
    SetLastError(USERERR|GL_INVALID_OPERATION);
    return;
  }

  // Check if this is a light name
  if (pname>=GL_LIGHT0 && pname<GL_LIGHT0+GL_MAX_USER_LIGHTS)
  {
    params[0]=rc->Block.Light[pname-GL_LIGHT0].On ? 1.0 : 0.0;
    return;
  }

  switch (pname)
  {
    case GL_MODELVIEW_MATRIX:
    {
      pm1=(GLdouble*)&(rc->Block.XForm.NMatrix[GL_MATRIX_MODELVIEW]);
      for (c=0; c<16; c++) params[c]=(GLfloat)pm1[c];
      return;
    }

    case GL_NORMALIZE:
    {
      params[0]=rc->Block.RenderFlag.Normalize ? 1.0 : 0.0;
      return;
    }

    case GL_PROJECTION_MATRIX:
    {
      pm1=(GLdouble*)&(rc->Block.XForm.NMatrix[GL_MATRIX_PROJECTION]);
      for (c=0; c<16; c++) params[c]=(GLfloat)pm1[c];
      return;
    }

    case GL_VIEWPORT:
    {
      params[0]=(GLfloat)rc->ViewPort[0];
      params[1]=(GLfloat)rc->ViewPort[1];
      params[2]=(GLfloat)rc->ViewPort[2];
      params[3]=(GLfloat)rc->ViewPort[3];
      return;
    }
  }

  SetLastError(USERERR|GL_INVALID_ENUM);
}

/**********************************************************************************************/

EXPORT void APIENTRY glGetIntegerv (GLenum pname, GLint* params)
{
  LPGLRENDERCONTEXT lpRC;

  if (!glCheckState()) return;

  if (params==NULL)
  {
    SetLastError(USERERR|GL_INVALID_OPERATION);
    return;
  }

  lpRC=GlCurrentContext;

  // Check if this is a light name
  if (pname>=GL_LIGHT0 && pname<GL_LIGHT0+GL_MAX_USER_LIGHTS)
  {
    params[0]=lpRC->Block.Light[pname-GL_LIGHT0].On ? 1 : 0;
    return;
  }

  switch (pname)
  {
    case GL_DEPTH_FUNC:
    {
      params[0]=lpRC->Block.RenderFunc.DepthFunc;
      return;
    }

    case GL_DEPTH_TEST:
    {
      params[0]=lpRC->Block.RenderFlag.Depth ? 1 : 0;
      return;
    }

    case GL_MAX_LIGHTS:
    {
      params[0]=GL_MAX_USER_LIGHTS;
      return;
    }

    case GL_NORMALIZE:
    {
      params[0]=lpRC->Block.RenderFlag.Normalize ? 1 : 0;
      return;
    }

    case GL_VIEWPORT:
    {
      params[0] = lpRC->ViewPort[0];
      params[1] = lpRC->ViewPort[1];
      params[2] = lpRC->ViewPort[2];
      params[3] = lpRC->ViewPort[3];
      return;
    }
  }

  SetLastError(USERERR|GL_INVALID_ENUM);
}

/**********************************************************************************************/

EXPORT void APIENTRY glLightf (GLenum light, GLenum pname, GLfloat param)
{
  if (!glCheckState()) return;

  light-=GL_LIGHT0;

  if (light>GL_MAX_USER_LIGHTS) { SetLastError(USERERR|GL_INVALID_ENUM); return; }

  switch (pname)
  {
    case GL_SPOT_EXPONENT:
    {
      GlCurrentContext->Block.Light[light].SpotExponent=param;
    }
    break;

    case GL_SPOT_CUTOFF:
    {
      GlCurrentContext->Block.Light[light].SpotCutOff=param;
    }
    break;

    case GL_CONSTANT_ATTENUATION:
    {
      GlCurrentContext->Block.Light[light].ConstantAtten=param;
    }
    break;

    case GL_LINEAR_ATTENUATION:
    {
      GlCurrentContext->Block.Light[light].LinearAtten=param;
    }
    break;

    case GL_QUADRATIC_ATTENUATION:
    {
      GlCurrentContext->Block.Light[light].QuadraticAtten=param;
    }
    break;

    default: SetLastError(USERERR|GL_INVALID_ENUM); break;
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glLightfv (GLenum light, GLenum pname, const GLfloat* params)
{
  GLdouble*     mat;
  GLdouble*     vec;

  if (!glCheckState() || params==NULL) return;

  light-=GL_LIGHT0;

  if (light>=GL_MAX_USER_LIGHTS) { SetLastError(USERERR|GL_INVALID_ENUM); return; }

  switch (pname)
  {
    case GL_AMBIENT :
    {
      GlCurrentContext->Block.Light[light].Ambient[R] = (GLdouble)params[0];
      GlCurrentContext->Block.Light[light].Ambient[G] = (GLdouble)params[1];
      GlCurrentContext->Block.Light[light].Ambient[B] = (GLdouble)params[2];
      GlCurrentContext->Block.Light[light].Ambient[A] = (GLdouble)params[3];
    }
    break;

    case GL_DIFFUSE :
    {
      GlCurrentContext->Block.Light[light].Diffuse[R] = (GLdouble)params[0];
      GlCurrentContext->Block.Light[light].Diffuse[G] = (GLdouble)params[1];
      GlCurrentContext->Block.Light[light].Diffuse[B] = (GLdouble)params[2];
      GlCurrentContext->Block.Light[light].Diffuse[A] = (GLdouble)params[3];
    }
    break;

    case GL_SPECULAR :
    {
      GlCurrentContext->Block.Light[light].Specular[R] = (GLdouble)params[0];
      GlCurrentContext->Block.Light[light].Specular[G] = (GLdouble)params[1];
      GlCurrentContext->Block.Light[light].Specular[B] = (GLdouble)params[2];
      GlCurrentContext->Block.Light[light].Specular[A] = (GLdouble)params[3];
    }
    break;

    case GL_POSITION :
    {
      GlCurrentContext->Block.Light[light].Position[X] = (GLdouble)params[0];
      GlCurrentContext->Block.Light[light].Position[Y] = (GLdouble)params[1];
      GlCurrentContext->Block.Light[light].Position[Z] = (GLdouble)params[2];
      GlCurrentContext->Block.Light[light].Position[W] = (GLdouble)params[3];

      vec=(GLdouble*)&(GlCurrentContext->Block.Light[light].Position);
      mat=(GLdouble*)&(GlCurrentContext->Block.XForm.NMatrix[GL_MATRIX_MODELVIEW]);

      glMatrix4dTransVector4d(vec, vec, mat);
    }
    break;

    default: SetLastError(USERERR|GL_INVALID_ENUM); break;
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glLighti (GLenum light, GLenum pname, GLint param)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glLightiv (GLenum light, GLenum pname, const GLint *params)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glLineStipple (GLint factor, GLushort pattern)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glLineWidth (GLfloat width)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glListBase (GLuint base)
{
  if (!glCheckState()) return;
}

/**********************************************************************************************/

EXPORT void APIENTRY glLoadIdentity ()
{
  GLdouble*     mat1;
  GLdouble*     mat2;

  if (!glCheckState()) return;

  mat1 = (GLdouble*) glGetContextCurrentNMatrix(GlCurrentContext);
  mat2 = (GLdouble*) glGetContextCurrentIMatrix(GlCurrentContext);

  glMatrix4dIdentity (mat1);
  glMatrix4dInverse  (mat2, mat1);

  if (GlCurrentContext->Block.XForm.MatrixMode==GL_MATRIX_PROJECTION)
  {
    glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glLoadMatrixd (const GLdouble* Matrix)
{
  GLdouble*     CNMatrix;
  GLdouble*     CIMatrix;
  GLint         c;

  if (!glCheckState() || Matrix==NULL) return;

  CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
  CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

  for (c=0; c<16; c++) CNMatrix[c]=Matrix[c];

  glMatrix4dInverse(CIMatrix, CNMatrix);

  if (GlCurrentContext->Block.XForm.MatrixMode==GL_MATRIX_PROJECTION)
  {
    glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glLoadMatrixf (const GLfloat* Matrix)
{
  GLdouble*     CNMatrix;
  GLdouble*     CIMatrix;
  GLint         c;

  if (!glCheckState() || Matrix==NULL) return;

  CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
  CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

  for (c=0; c<16; c++) CNMatrix[c]=Matrix[c];

  glMatrix4dInverse(CIMatrix, CNMatrix);

  if (GlCurrentContext->Block.XForm.MatrixMode==GL_MATRIX_PROJECTION)
  {
    glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glMaterialf (GLenum face, GLenum pname, GLfloat param)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glMaterialfv (GLenum face, GLenum pname, const GLfloat* params)
{
  LPGLRENDERCONTEXT rc=GlCurrentContext;

  if (rc==NULL) return;

  switch (pname)
  {
    case GL_AMBIENT:
    {
      if (face==GL_FRONT || face==GL_FRONT_AND_BACK)
      {
        rc->IPolygon.FrontFace.Ambient[R] = params[R];
        rc->IPolygon.FrontFace.Ambient[G] = params[G];
        rc->IPolygon.FrontFace.Ambient[B] = params[B];
        rc->IPolygon.FrontFace.Ambient[A] = params[A];
      }
      if (face==GL_BACK || face==GL_FRONT_AND_BACK)
      {
        rc->IPolygon.BackFace.Ambient[R] = params[R];
        rc->IPolygon.BackFace.Ambient[G] = params[G];
        rc->IPolygon.BackFace.Ambient[B] = params[B];
        rc->IPolygon.BackFace.Ambient[A] = params[A];
      }
    }
    break;

    case GL_DIFFUSE:
    {
      if (face==GL_FRONT || face==GL_FRONT_AND_BACK)
      {
        rc->IPolygon.FrontFace.Diffuse[R] = params[R];
        rc->IPolygon.FrontFace.Diffuse[G] = params[G];
        rc->IPolygon.FrontFace.Diffuse[B] = params[B];
        rc->IPolygon.FrontFace.Diffuse[A] = params[A];
      }
      if (face==GL_BACK || face==GL_FRONT_AND_BACK)
      {
        rc->IPolygon.BackFace.Diffuse[R] = params[R];
        rc->IPolygon.BackFace.Diffuse[G] = params[G];
        rc->IPolygon.BackFace.Diffuse[B] = params[B];
        rc->IPolygon.BackFace.Diffuse[A] = params[A];
      }
    }
    break;

    case GL_SPECULAR:
    {
      if (face==GL_FRONT || face==GL_FRONT_AND_BACK)
      {
        rc->IPolygon.FrontFace.Specular[R] = params[R];
        rc->IPolygon.FrontFace.Specular[G] = params[G];
        rc->IPolygon.FrontFace.Specular[B] = params[B];
        rc->IPolygon.FrontFace.Specular[A] = params[A];
      }
      if (face==GL_BACK || face==GL_FRONT_AND_BACK)
      {
        rc->IPolygon.BackFace.Specular[R] = params[R];
        rc->IPolygon.BackFace.Specular[G] = params[G];
        rc->IPolygon.BackFace.Specular[B] = params[B];
        rc->IPolygon.BackFace.Specular[A] = params[A];
      }
    }
    break;

    case GL_EMISSION:
    {
      if (face==GL_FRONT || face==GL_FRONT_AND_BACK)
      {
        rc->IPolygon.FrontFace.Emission[R] = params[R];
        rc->IPolygon.FrontFace.Emission[G] = params[G];
        rc->IPolygon.FrontFace.Emission[B] = params[B];
        rc->IPolygon.FrontFace.Emission[A] = params[A];
      }
      if (face==GL_BACK || face==GL_FRONT_AND_BACK)
      {
        rc->IPolygon.BackFace.Emission[R] = params[R];
        rc->IPolygon.BackFace.Emission[G] = params[G];
        rc->IPolygon.BackFace.Emission[B] = params[B];
        rc->IPolygon.BackFace.Emission[A] = params[A];
      }
    }
    break;

    case GL_SHININESS:
    {
      if (face==GL_FRONT || face==GL_FRONT_AND_BACK)
      {
        rc->IPolygon.FrontFace.Shininess = params[0];
      }
      if (face==GL_BACK || face==GL_FRONT_AND_BACK)
      {
        rc->IPolygon.BackFace.Shininess = params[0];
      }
    }
    break;

    default: SetLastError(USERERR|GL_INVALID_ENUM); break;
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glMateriali (GLenum face, GLenum pname, GLint param)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glMaterialiv (GLenum face, GLenum pname, const GLint *params)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glMatrixMode (GLenum mode)
{
  if (!glCheckState()) return;

  switch (mode)
  {
    case GL_MODELVIEW:
    {
      GlCurrentContext->Block.XForm.MatrixMode=GL_MATRIX_MODELVIEW;
    }
    break;

    case GL_PROJECTION:
    {
      GlCurrentContext->Block.XForm.MatrixMode=GL_MATRIX_PROJECTION;
    }
    break;

    case GL_TEXTURE:
    {
      GlCurrentContext->Block.XForm.MatrixMode=GL_MATRIX_TEXTURE;
    }
    break;

    default: SetLastError(USERERR|GL_INVALID_ENUM); break;
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glMultMatrixd (const GLdouble* Matrix)
{
  GLdouble*     CNMatrix;
  GLdouble*     CIMatrix;
  GLdouble      Temp [16];
  GLint         c;

  if (!glCheckState() || Matrix==NULL) return;

  CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
  CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

  for (c=0; c<16; c++) Temp[c]=Matrix[c];

  glMatrix4dTimes   (CNMatrix, Temp, CNMatrix);
  glMatrix4dInverse (CIMatrix, CNMatrix);

  if (GlCurrentContext->Block.XForm.MatrixMode==GL_MATRIX_PROJECTION)
  {
    glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glMultMatrixf (const GLfloat* Matrix)
{
  GLdouble*     CNMatrix;
  GLdouble*     CIMatrix;
  GLdouble      Temp [16];
  GLint         c;

  if (!glCheckState() || Matrix==NULL) return;

  CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
  CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

  for (c=0; c<16; c++) Temp[c]=Matrix[c];

  glMatrix4dTimes   (CNMatrix, Temp, CNMatrix);
  glMatrix4dInverse (CIMatrix, CNMatrix);

  if (GlCurrentContext->Block.XForm.MatrixMode==GL_MATRIX_PROJECTION)
  {
    glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3b (GLbyte nx, GLbyte ny, GLbyte nz)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3bv (const GLbyte *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3d (GLdouble nx, GLdouble ny, GLdouble nz)
{
  if (GlCurrentContext)
  {
    GlCurrentContext->IVertex.Normal[X] = nx;
    GlCurrentContext->IVertex.Normal[Y] = ny;
    GlCurrentContext->IVertex.Normal[Z] = nz;
    GlCurrentContext->IVertex.Normal[W] = 0.0;
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3dv (const GLdouble *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz)
{
  if (GlCurrentContext)
  {
    GlCurrentContext->IVertex.Normal[X] = (GLdouble)nx;
    GlCurrentContext->IVertex.Normal[Y] = (GLdouble)ny;
    GlCurrentContext->IVertex.Normal[Z] = (GLdouble)nz;
    GlCurrentContext->IVertex.Normal[W] = 0.0;
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3fv (const GLfloat *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3i (GLint nx, GLint ny, GLint nz)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3iv (const GLint *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3s (GLshort nx, GLshort ny, GLshort nz)
{
  if (GlCurrentContext)
  {
    GlCurrentContext->IVertex.Normal[X] = (GLdouble)nx / (GLdouble)32767.0;
    GlCurrentContext->IVertex.Normal[Y] = (GLdouble)ny / (GLdouble)32767.0;
    GlCurrentContext->IVertex.Normal[Z] = (GLdouble)nz / (GLdouble)32767.0;
    GlCurrentContext->IVertex.Normal[W] = 0.0;
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glNormal3sv (const GLshort *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glOrtho
(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble znear, GLdouble zfar)
{
  GLdouble      tmp [4][4];
  GLdouble*     CNMatrix;
  GLdouble*     CIMatrix;

  if (!glCheckState()) return;

  if (0==(right-left)) right+=0.001;
  if (0==(top-bottom)) top+=0.001;
  if (0==(zfar-znear)) zfar+=0.001;

  tmp[0][0]     =  2.0/(right-left);
  tmp[0][1]     =  0.0;
  tmp[0][2]     =  0.0;
  tmp[0][3]     =  (right+left)/(right-left);

  tmp[1][0]     =  0.0;
  tmp[1][1]     =  2.0/(top-bottom);
  tmp[1][2]     =  0.0;
  tmp[1][3]     =  (top+bottom)/(top-bottom);

  tmp[2][0]     =  0.0;
  tmp[2][1]     =  0.0;
  tmp[2][2]     =  2.0/(zfar-znear);
  tmp[2][3]     =  (zfar+znear)/(zfar-znear);

  tmp[3][0]     =  0.0;
  tmp[3][1]     =  0.0;
  tmp[3][2]     =  0.0;
  tmp[3][3]     =  1.0;

  CNMatrix      = glGetContextCurrentNMatrix(GlCurrentContext);
  CIMatrix      = glGetContextCurrentIMatrix(GlCurrentContext);

  glMatrix4dConcat  (CNMatrix, CNMatrix, tmp, GL_MATOP_POSTCONCATENATE);
  glMatrix4dInverse (CIMatrix, CNMatrix);

  if (GlCurrentContext->Block.XForm.MatrixMode==GL_MATRIX_PROJECTION)
  {
    glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
  }
}

/**********************************************************************************************/

EXPORT GLint APIENTRY glRenderMode (GLenum mode)
{
  return 0;
}

/**********************************************************************************************/

EXPORT void APIENTRY glRotated (GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
  GLdouble      Pnt1 [3];
  GLdouble      Pnt2 [3];
  GLdouble      tmp [4][4];
  GLdouble*     CNMatrix;
  GLdouble*     CIMatrix;
  GLdouble      RadianAngle;

  if (!glCheckState()) return;

  Pnt1[X]=0; Pnt1[Y]=0; Pnt1[Z]=0;
  Pnt2[X]=x; Pnt2[Y]=y; Pnt2[Z]=z;

  RadianAngle=(angle*GL_PI)/180.0;

  glMatrix4dRotationLine(tmp, RadianAngle, Pnt1, Pnt2);

  CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
  CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

  glMatrix4dConcat  (CNMatrix, CNMatrix, tmp, GL_MATOP_POSTCONCATENATE);
  glMatrix4dInverse (CIMatrix, CNMatrix);

  if (GlCurrentContext->Block.XForm.MatrixMode==GL_MATRIX_PROJECTION)
  {
    glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
  glRotated((GLdouble)angle, (GLdouble)x, (GLdouble)y, (GLdouble)z);
}

/**********************************************************************************************/

EXPORT void APIENTRY glScaled (GLdouble x, GLdouble y, GLdouble z)
{
  GLdouble      scl [3];
  GLdouble      tmp [4][4];
  GLdouble*     CNMatrix;
  GLdouble*     CIMatrix;

  if (!glCheckState()) return;

  scl[X]=x; scl[Y]=y; scl[Z]=z;
  glCreateScalingMatrix4d(tmp, scl);

  CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
  CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

  glMatrix4dConcat  (CNMatrix, CNMatrix, tmp, GL_MATOP_POSTCONCATENATE);
  glMatrix4dInverse (CIMatrix, CNMatrix);

  if (GlCurrentContext->Block.XForm.MatrixMode==GL_MATRIX_PROJECTION)
  {
    glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glScalef (GLfloat x, GLfloat y, GLfloat z)
{
  glScaled((GLdouble)x, (GLdouble)y, (GLdouble)z);
}

/**********************************************************************************************/

EXPORT void APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glSelectBuffer (GLsizei size, GLuint *buffer)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glShadeModel (GLenum mode)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glStencilMask (GLuint mask)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glTranslated (GLdouble x, GLdouble y, GLdouble z)
{
  GLdouble      vec [3];
  GLdouble      tmp [4][4];
  GLdouble*     CNMatrix;
  GLdouble*     CIMatrix;

  if (!glCheckState()) return;

  vec[X]=x; vec[Y]=y; vec[Z]=z;
  glCreateTranslationMatrix4d(tmp, vec);

  CNMatrix = glGetContextCurrentNMatrix(GlCurrentContext);
  CIMatrix = glGetContextCurrentIMatrix(GlCurrentContext);

  glMatrix4dConcat  (CNMatrix, CNMatrix, tmp, GL_MATOP_POSTCONCATENATE);
  glMatrix4dInverse (CIMatrix, CNMatrix);

  if (GlCurrentContext->Block.XForm.MatrixMode==GL_MATRIX_PROJECTION)
  {
    glComputeFrustum(GlCurrentContext, &(GlCurrentContext->Block));
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glTranslatef (GLfloat x, GLfloat y, GLfloat z)
{
  glTranslated((GLdouble)x, (GLdouble)y, (GLdouble)z);
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2d (GLdouble x, GLdouble y)
{
  if (!GlInputPrimitive) return;

  if (GlCurrentContext)
  {
    GlCurrentContext->IVertex.World[X] = x;
    GlCurrentContext->IVertex.World[Y] = y;
    GlCurrentContext->IVertex.World[Z] = 0.0;
    GlCurrentContext->IVertex.World[W] = 1.0;
    glAddInputVertexToInputPolygon(GlCurrentContext);
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2dv (const GLdouble *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2f (GLfloat x, GLfloat y)
{
  if (!GlInputPrimitive) return;

  if (GlCurrentContext)
  {
    GlCurrentContext->IVertex.World[X] = (GLdouble)x;
    GlCurrentContext->IVertex.World[Y] = (GLdouble)y;
    GlCurrentContext->IVertex.World[Z] = 0.0;
    GlCurrentContext->IVertex.World[W] = 1.0;
    glAddInputVertexToInputPolygon(GlCurrentContext);
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2fv (const GLfloat *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2i (GLint x, GLint y)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2iv (const GLint *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2s (GLshort x, GLshort y)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex2sv (const GLshort *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3d (GLdouble x, GLdouble y, GLdouble z)
{
  if (!GlInputPrimitive) return;

  if (GlCurrentContext)
  {
    GlCurrentContext->IVertex.World[X] = x;
    GlCurrentContext->IVertex.World[Y] = y;
    GlCurrentContext->IVertex.World[Z] = z;
    GlCurrentContext->IVertex.World[W] = 1.0;
    glAddInputVertexToInputPolygon(GlCurrentContext);
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3dv (const GLdouble *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3f (GLfloat x, GLfloat y, GLfloat z)
{
  if (!GlInputPrimitive) return;

  if (GlCurrentContext)
  {
    GlCurrentContext->IVertex.World[X] = (GLdouble)x;
    GlCurrentContext->IVertex.World[Y] = (GLdouble)y;
    GlCurrentContext->IVertex.World[Z] = (GLdouble)z;
    GlCurrentContext->IVertex.World[W] = 1.0;
    glAddInputVertexToInputPolygon(GlCurrentContext);
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3fv (const GLfloat *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3i (GLint x, GLint y, GLint z)
{
  if (!GlInputPrimitive) return;

  if (GlCurrentContext)
  {
    GlCurrentContext->IVertex.World[X] = (GLdouble)x;
    GlCurrentContext->IVertex.World[Y] = (GLdouble)y;
    GlCurrentContext->IVertex.World[Z] = (GLdouble)z;
    GlCurrentContext->IVertex.World[W] = 1.0;
    glAddInputVertexToInputPolygon(GlCurrentContext);
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3iv (const GLint *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3s (GLshort x, GLshort y, GLshort z)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex3sv (const GLshort *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
  if (!GlInputPrimitive) return;

  if (GlCurrentContext)
  {
    GlCurrentContext->IVertex.World[X] = x;
    GlCurrentContext->IVertex.World[Y] = y;
    GlCurrentContext->IVertex.World[Z] = z;
    GlCurrentContext->IVertex.World[W] = w;
    glAddInputVertexToInputPolygon(GlCurrentContext);
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4dv (const GLdouble *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  if (!GlInputPrimitive) return;

  if (GlCurrentContext)
  {
    GlCurrentContext->IVertex.World[X] = (GLdouble)x;
    GlCurrentContext->IVertex.World[Y] = (GLdouble)y;
    GlCurrentContext->IVertex.World[Z] = (GLdouble)z;
    GlCurrentContext->IVertex.World[W] = (GLdouble)w;
    glAddInputVertexToInputPolygon(GlCurrentContext);
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4fv (const GLfloat *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4i (GLint x, GLint y, GLint z, GLint w)
{
  if (!GlInputPrimitive) return;

  if (GlCurrentContext)
  {
    GlCurrentContext->IVertex.World[X] = (GLdouble)x;
    GlCurrentContext->IVertex.World[Y] = (GLdouble)y;
    GlCurrentContext->IVertex.World[Z] = (GLdouble)z;
    GlCurrentContext->IVertex.World[W] = (GLdouble)w;
    glAddInputVertexToInputPolygon(GlCurrentContext);
  }
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4iv (const GLint *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4s (GLshort x, GLshort y, GLshort z, GLshort w)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glVertex4sv (const GLshort *v)
{
}

/**********************************************************************************************/

EXPORT void APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height)
{
  LPGLRENDERCONTEXT rc=GlCurrentContext;

  if (!glCheckState()) return;

  // Check if the size really changes
  if (rc->ViewPort[0] == x     && rc->ViewPort[1] == y &&
      rc->ViewPort[2] == width && rc->ViewPort[3] == height) return;

  rc->ViewPort[0]       = x;
  rc->ViewPort[1]       = y;
  rc->ViewPort[2]       = width;
  rc->ViewPort[3]       = height;
  rc->Width             = width;
  rc->Height            = height;

  glResizeContext(rc);
}
