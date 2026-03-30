
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

#include <stdio.h>
#include <math.h>

#include "tinygl.h"

#include "gllist.h"
#include "glvector.h"
#include "glmain.h"
#include "glraster.h"

/************************************************************************/

static LPGLRENDERCONTEXT      RasterRenderContext;
static LPGLRENDERBLOCK        RasterRenderBlock;
static GLfloat*               RasterDepthBuffer;

static HBITMAP                hBitmap;

static GLenum                 NumTrianglePassed;
static GLenum                 NumLinePassed;
static GLenum                 NumPixelPassed;

// Variables used by GlRasterTriangle

GLrastervertex        rv[3];                  // Vertices to interpolate
GLrastervertex        dv[2];                  // Delta vertices
GLrastervertex        sv, ev;                 // Start and end vertices
GLrastervertex        tmprv;                  // Temp vertex for swaps

// Variables used by GlRasterLine

GLrastervertex        RasLineStart,  RasLineEnd;
GLrastervertex        RasLineVertex, RasLineDelta;
long                  rll;                    // Scanline

long                  rtexwidth, rtexheight;
long                  rtexwmod, rtexhmod;
long                  rtexpixel, rtexstride;

GLU32*                GlTexturePlane;
GLI32                 GlClip[2][2];

static GLVERTEX       TempVertices [2] [GL_MAX_POLY_VERTEX];

static char szTemp [256];

/**********************************************************************************************/

  #define SWAP(a,b) { ltmp=a; a=b; b=ltmp; }
  #define SWAPRV(a,b) { tmprv=a; a=b; b=tmprv; }

/**********************************************************************************************/

GLboolean glFaceCull (GLrastervertex* v1, GLrastervertex* v2, GLrastervertex* v3)
{
  GLdouble a, tmp;

  GLdouble x1 = v1->x;
  GLdouble x2 = v2->x;
  GLdouble x3 = v3->x;
  GLdouble y1 = v1->y;
  GLdouble y2 = v2->y;
  GLdouble y3 = v3->y;

  // if (RasterRenderBlock->RenderFlag.CullFace==FALSE) return TRUE;

  if (x1 == x2 && x1 == x3) return 0;
  if (y1 == y2 && y1 == y3) return 0;

  #define swap(a,b) { tmp = a; a = b; b = tmp; }

  if (x1 > x2) { swap(x1, x2); swap(y1, y2); swap(x2, x3); swap(y2, y3); }
  if (x1 > x2) { swap(x1, x2); swap(y1, y2); swap(x2, x3); swap(y2, y3); }

  a = (x2 - x1) * (y3 - y2) - (x3 - x2) * (y2 - y1);

  if (a > 0.0) return 1;

  return 0;
}

/**********************************************************************************************/

static void glRasterLine ()
{
  register long c;

  HDC           hdc;
  COLORREF      ColorRef;

  GLU32*        Plane;
  GLI32         PlaneOffset;
  GLU32         Pixel;

  GLI32         DepthOffset;
  GLdouble      DepthValue;

  GLU32*        Texture;
  GLI32         TexelOffset;
  GLU32         Texel;
  GLI32         RealU, RealV;

  GLI32         StartX, EndX;

  GLdouble      fr1, fg1, fb1, fr2, fg2, fb2;
  GLdouble      fr3, fg3, fb3, fr4, fg4, fb4;
  GLdouble      fracu, fracv;

  BOOL          PassedDepthTest;

  /*****************************************************/

  GLI32         VLine;
  GLdouble      l, LineLength;
  GLdouble      z, r, g, b, w;

  /*****************************************************/

  // Check if line is in viewport
  if (rll < GlClip[0][Y] || rll > GlClip[1][Y]) return;

  VLine = RasterRenderContext->Height - rll - 1;

  // rle.x++;

  LineLength = RasLineEnd.x - RasLineStart.x;
  if (LineLength == 0.0) return;

  RasLineVertex.x = RasLineStart.x;
  RasLineVertex.z = RasLineStart.z;
  RasLineVertex.r = RasLineStart.r;
  RasLineVertex.g = RasLineStart.g;
  RasLineVertex.b = RasLineStart.b;
  RasLineVertex.a = RasLineStart.a;
  RasLineVertex.u = RasLineStart.u;
  RasLineVertex.v = RasLineStart.v;
  RasLineVertex.w = RasLineStart.w;

  RasLineDelta.z = (RasLineEnd.z-RasLineStart.z) / LineLength;
  RasLineDelta.r = (RasLineEnd.r-RasLineStart.r) / LineLength;
  RasLineDelta.g = (RasLineEnd.g-RasLineStart.g) / LineLength;
  RasLineDelta.b = (RasLineEnd.b-RasLineStart.b) / LineLength;
  RasLineDelta.a = (RasLineEnd.a-RasLineStart.a) / LineLength;
  RasLineDelta.u = (RasLineEnd.u-RasLineStart.u) / LineLength;
  RasLineDelta.v = (RasLineEnd.v-RasLineStart.v) / LineLength;
  RasLineDelta.w = (RasLineEnd.w-RasLineStart.w) / LineLength;

  hdc = RasterRenderContext->Device.Handle;

  PlaneOffset = 0;
  DepthOffset = (rll * RasterRenderContext->Width) + RasLineVertex.x;
  Texture     = (GLU32*) GlTexturePlane;
  Plane       = (GLU32*) GlCurrentContext->DIBSection.Base;

  // Main loop

  for (c = 0; c < LineLength; c++)
  {
    if (RasLineVertex.x > GlClip[1][X]) break;

    if (RasLineVertex.x >= GlClip[0][X])
    {
      // Get the real w value
      w = RasLineVertex.w;

      // Get the real z value
      z = RasLineVertex.z;

  /*****************************************************************/

      DepthValue = RasterDepthBuffer[DepthOffset];
      PassedDepthTest = 0;

      if (RasterRenderBlock->RenderFlag.Depth)
      {
        switch (RasterRenderBlock->RenderFunc.DepthFunc)
        {
          case GL_EQUAL    : if (z == DepthValue) PassedDepthTest = 1; break;
          case GL_LEQUAL   : if (z <= DepthValue) PassedDepthTest = 1; break;
          case GL_GREATER  : if (z >  DepthValue) PassedDepthTest = 1; break;
          case GL_NOTEQUAL : if (z != DepthValue) PassedDepthTest = 1; break;
          case GL_GEQUAL   : if (z >= DepthValue) PassedDepthTest = 1; break;
          case GL_ALWAYS   : PassedDepthTest = 1; break;
          case GL_LESS     :
          default          : if (z <  DepthValue) PassedDepthTest = 1; break;
        }
      }
      else
      {
        PassedDepthTest = 1;
      }

  /*****************************************************************/

      if (PassedDepthTest == 1)
      {

  /*****************************************************************/

        if (0)
        {

          if (w)
          {
            RealU = (GLI32) (RasLineVertex.u / w);
            RealV = (GLI32) (RasLineVertex.v / w);
          }
          else
          {
            RealU = (GLI32) RasLineVertex.u;
            RealV = (GLI32) RasLineVertex.v;
          }

          RealU &= rtexwmod;
          RealV &= rtexhmod;

          TexelOffset = (RealV * rtexwidth) + RealU;

          Texel = Texture[TexelOffset];

        }

        else

        // No texturing
        {
          Texel = 0xFFFFFFFF;
        }

  /*****************************************************************/

        // Do chromakey test
/*
        if
        (
          (GlRenderFunc.ChromaKeyMode==GR_CHROMAKEY_DISABLE)
          ||
          (GlRenderFunc.ChromaKeyMode==GR_CHROMAKEY_ENABLE &&
           (texel&0x00FFFFFF)!=(GlRenderFunc.ChromaKeyValue&0x00FFFFFF))
        )
*/
        {

          r = (GLdouble) ((Texel & 0x00FF0000) >> 16) / 255.0;
          g = (GLdouble) ((Texel & 0x0000FF00) >>  8) / 255.0;
          b = (GLdouble) ((Texel & 0x000000FF) >>  0) / 255.0;

          /*
          r/=255.0f;
          g/=255.0f;
          b/=255.0f;
          */

  /*****************************************************************/

#ifdef FILTER

          // u+1, v
          if (realu < (rtexwidth - 1))
          {
            texeloffset = (realv * rtexwidth) + realu + 1;
            texel = rltex[texeloffset];
            fr1 = (texel & 0x00FF0000) >> 16;
            fg1 = (texel & 0x0000FF00) >>  8;
            fb1 = (texel & 0x000000FF) >>  0;
          }
          else { fr1 = r; fg1 = g; fb1 = b; }

          // u, v
          fr2 = r; fg2 = g; fb2 = b;

          // u+1, v-1
          if (realu < (rtexwidth - 1) && realv > 0)
          {
            texeloffset = ((realv - 1) * rtexwidth) + realu + 1;
            texel = rltex[texeloffset];
            fr3 = (texel & 0x00FF0000) >> 16;
            fg3 = (texel & 0x0000FF00) >>  8;
            fb3 = (texel & 0x000000FF) >>  0;
          }
          else { fr3 = r; fg3 = g; fb3 = b; }

          // u, v-1
          if (realv > 0)
          {
            texeloffset = ((realv - 1) * rtexwidth) + realu;
            texel = rltex[texeloffset];
            fr4 = (texel & 0x00FF0000) >> 16;
            fg4 = (texel & 0x0000FF00) >>  8;
            fb4 = (texel & 0x000000FF) >>  0;
          }
          else { fr4 = r; fg4 = g; fb4 = b; }

#ifdef PCM

          if (w != 0)
          {
            fracu = (float) ((RasLineVertex.u / w) & 255) / 256.0f;
            fracv = (float) ((RasLineVertex.v / w) & 255) / 256.0f;
          }
          else
          {
            fracu = 0;
            fracv = 0;
          }

#else

          fracu = (float) (RasLineVertex.u & 255) / 256.0f;
          fracv = (float) (RasLineVertex.v & 255) / 256.0f;

#endif

          r=(fr1*fracu*fracv)+(fr2*(1-fracu)*fracv)+(fr3*fracu*(1-fracv))+(fr4*(1-fracu)*(1-fracv));
          g=(fg1*fracu*fracv)+(fg2*(1-fracu)*fracv)+(fg3*fracu*(1-fracv))+(fg4*(1-fracu)*(1-fracv));
          b=(fb1*fracu*fracv)+(fb2*(1-fracu)*fracv)+(fb3*fracu*(1-fracv))+(fb4*(1-fracu)*(1-fracv));

#endif

  /*****************************************************************/

/*
          switch (GlRenderFunc.ColorCombineFunction)
          {
            case GR_COLORCOMBINE_CCRGB:
              r=(GlRenderFunc.ConstantColor&0x00FF0000)>>16;
              g=(GlRenderFunc.ConstantColor&0x0000FF00)>> 8;
              b=(GlRenderFunc.ConstantColor&0x000000FF)>> 0;
              break;
            case GR_COLORCOMBINE_ITRGB:
              r=FIXDN(rlv.r, ACR);
              g=FIXDN(rlv.g, ACG);
              b=FIXDN(rlv.b, ACB);
              break;
            case GR_COLORCOMBINE_TEXTURE_TIMES_CCRGB:
              r=DIV256(r*((GlRenderFunc.ConstantColor&0x00FF0000)>>16));
              g=DIV256(g*((GlRenderFunc.ConstantColor&0x0000FF00)>> 8));
              b=DIV256(b*((GlRenderFunc.ConstantColor&0x000000FF)>> 0));
              break;
            case GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB:
              r=DIV256(r*FIXDN(rlv.r, ACR));
              g=DIV256(g*FIXDN(rlv.g, ACG));
              b=DIV256(b*FIXDN(rlv.b, ACB));
              break;
            case GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB_ADD_ALPHA:
              r=DIV256(r*FIXDN(rlv.r, ACR))+FIXDN(rlv.a, ACA);
              g=DIV256(g*FIXDN(rlv.g, ACG))+FIXDN(rlv.a, ACA);
              b=DIV256(b*FIXDN(rlv.b, ACB))+FIXDN(rlv.a, ACA);
              break;
            case GR_COLORCOMBINE_TEXTURE_TIMES_ALPHA:
              r=DIV256(r*FIXDN(rlv.a, ACA));
              g=DIV256(g*FIXDN(rlv.a, ACA));
              b=DIV256(b*FIXDN(rlv.a, ACA));
              break;
            case GR_COLORCOMBINE_TEXTURE_TIMES_ALPHA_ADD_ITRGB:
              r=DIV256(r*FIXDN(rlv.a, ACA))+FIXDN(rlv.r, ACR);
              g=DIV256(g*FIXDN(rlv.a, ACA))+FIXDN(rlv.g, ACG);
              b=DIV256(b*FIXDN(rlv.a, ACA))+FIXDN(rlv.b, ACB);
              break;
            case GR_COLORCOMBINE_TEXTURE_ADD_ITRGB:
              r=r+FIXDN(rlv.r, ACR);
              g=g+FIXDN(rlv.g, ACG);
              b=b+FIXDN(rlv.b, ACB);
              break;
            case GR_COLORCOMBINE_TEXTURE_SUB_ITRGB:
              r=r-FIXDN(rlv.r, ACR);
              g=g-FIXDN(rlv.g, ACG);
              b=b-FIXDN(rlv.b, ACB);
              break;
          }
*/

              r = RasLineVertex.r;
              g = RasLineVertex.g;
              b = RasLineVertex.b;

  /*****************************************************************/

          CLAMP(r, 0.0, 1.0);
          CLAMP(g, 0.0, 1.0);
          CLAMP(b, 0.0, 1.0);

          r *= 255.0;
          g *= 255.0;
          b *= 255.0;

          ColorRef = RGB(r, g, b);
          SetPixel(hdc, RasLineVertex.x, VLine, ColorRef);

          /*
          ColorRef=RGB_TO_RGB888(r, g, b);
          Plane[PlaneOffset++]=ColorRef;
          */

          NumPixelPassed++;

          // Set new z value in z-buffer
          if (RasterRenderBlock->RenderFunc.DepthMask)
          {
            RasterDepthBuffer[DepthOffset] = z;
          }
        }
      }
    }

    // Increment interpolants
    RasLineVertex.x++;
    RasLineVertex.z += RasLineDelta.z;
    RasLineVertex.r += RasLineDelta.r;
    RasLineVertex.g += RasLineDelta.g;
    RasLineVertex.b += RasLineDelta.b;
    RasLineVertex.a += RasLineDelta.a;
    RasLineVertex.u += RasLineDelta.u;
    RasLineVertex.v += RasLineDelta.v;
    RasLineVertex.w += RasLineDelta.w;

    DepthOffset++;
  }

  NumLinePassed++;
}

/**********************************************************************************************/

void glRasterTriangle ()
{
  long step, ln;

  if (glFaceCull(rv + 0, rv + 1, rv + 2) == FALSE) return;

  // Do swaps
  if (rv[0].y > rv[1].y) SWAPRV(rv[0], rv[1]);
  if (rv[0].y > rv[2].y) SWAPRV(rv[0], rv[2]);
  if (rv[1].y > rv[2].y) SWAPRV(rv[1], rv[2]);

  if (rv[0].y != rv[2].y)
  {
    step = rv[0].y - rv[2].y;
    if (step == 0.0) step = 1.0;

    dv[0].x =(rv[0].x - rv[2].x) / step;
    dv[0].z =(rv[0].z - rv[2].z) / step;
    dv[0].r =(rv[0].r - rv[2].r) / step;
    dv[0].g =(rv[0].g - rv[2].g) / step;
    dv[0].b =(rv[0].b - rv[2].b) / step;
    dv[0].a =(rv[0].a - rv[2].a) / step;
    dv[0].u =(rv[0].u - rv[2].u) / step;
    dv[0].v =(rv[0].v - rv[2].v) / step;
    dv[0].w =(rv[0].w - rv[2].w) / step;
  }
  else
  {
    dv[0].x = 0;
    dv[0].z = 0;
    dv[0].r = 0;
    dv[0].g = 0;
    dv[0].b = 0;
    dv[0].a = 0;
    dv[0].u = 0;
    dv[0].v = 0;
    dv[0].w = 0;
  }

  if (rv[0].y != rv[1].y)
  {
    step = rv[1].y - rv[0].y;
    if (step == 0.0) step = 1.0;

    dv[1].x = (rv[1].x - rv[0].x) / step;
    dv[1].z = (rv[1].z - rv[0].z) / step;
    dv[1].r = (rv[1].r - rv[0].r) / step;
    dv[1].g = (rv[1].g - rv[0].g) / step;
    dv[1].b = (rv[1].b - rv[0].b) / step;
    dv[1].a = (rv[1].a - rv[0].a) / step;
    dv[1].u = (rv[1].u - rv[0].u) / step;
    dv[1].v = (rv[1].v - rv[0].v) / step;
    dv[1].w = (rv[1].w - rv[0].w) / step;
  }
  else
  {
    dv[1].x = 0;
    dv[1].z = 0;
    dv[1].r = 0;
    dv[1].g = 0;
    dv[1].b = 0;
    dv[1].a = 0;
    dv[1].u = 0;
    dv[1].v = 0;
    dv[1].w = 0;
  }

  sv.x = rv[0].x; ev.x = sv.x;
  sv.z = rv[0].z; ev.z = sv.z;
  sv.r = rv[0].r; ev.r = sv.r;
  sv.g = rv[0].g; ev.g = sv.g;
  sv.b = rv[0].b; ev.b = sv.b;
  sv.a = rv[0].a; ev.a = sv.a;
  sv.u = rv[0].u; ev.u = sv.u;
  sv.v = rv[0].v; ev.v = sv.v;
  sv.w = rv[0].w; ev.w = sv.w;

  if (dv[0].x < dv[1].x)
  {
    for (rll = rv[0].y + 1; rll <= rv[1].y; rll++)
    {
      sv.x += dv[0].x; ev.x += dv[1].x;
      sv.z += dv[0].z; ev.z += dv[1].z;
      sv.r += dv[0].r; ev.r += dv[1].r;
      sv.g += dv[0].g; ev.g += dv[1].g;
      sv.b += dv[0].b; ev.b += dv[1].b;
      sv.a += dv[0].a; ev.a += dv[1].a;
      sv.u += dv[0].u; ev.u += dv[1].u;
      sv.v += dv[0].v; ev.v += dv[1].v;
      sv.w += dv[0].w; ev.w += dv[1].w;

      rls.x = sv.x; rle.x = ev.x;
      rls.z = sv.z; rle.z = ev.z;
      rls.r = sv.r; rle.r = ev.r;
      rls.g = sv.g; rle.g = ev.g;
      rls.b = sv.b; rle.b = ev.b;
      rls.a = sv.a; rle.a = ev.a;
      rls.u = sv.u; rle.u = ev.u;
      rls.v = sv.v; rle.v = ev.v;
      rls.w = sv.w; rle.w = ev.w;

      glRasterLine();
    }
  }
  else
  {
    for (rll=rv[0].y+1; rll<=rv[1].y; rll++)
    {
      sv.x+=dv[0].x; ev.x+=dv[1].x;
      sv.z+=dv[0].z; ev.z+=dv[1].z;
      sv.r+=dv[0].r; ev.r+=dv[1].r;
      sv.g+=dv[0].g; ev.g+=dv[1].g;
      sv.b+=dv[0].b; ev.b+=dv[1].b;
      sv.a+=dv[0].a; ev.a+=dv[1].a;
      sv.u+=dv[0].u; ev.u+=dv[1].u;
      sv.v+=dv[0].v; ev.v+=dv[1].v;
      sv.w+=dv[0].w; ev.w+=dv[1].w;

      rls.x=ev.x; rle.x=sv.x;
      rls.z=ev.z; rle.z=sv.z;
      rls.r=ev.r; rle.r=sv.r;
      rls.g=ev.g; rle.g=sv.g;
      rls.b=ev.b; rle.b=sv.b;
      rls.a=ev.a; rle.a=sv.a;
      rls.u=ev.u; rle.u=sv.u;
      rls.v=ev.v; rle.v=sv.v;
      rls.w=ev.w; rle.w=sv.w;

      glRasterLine();
    }
  }

  if (rv[1].y!=rv[2].y)
  {
    step        =rv[1].y-rv[2].y;
    if (step==0.0) step=1.0;

    dv[1].x     =(rv[1].x-rv[2].x)/step;
    dv[1].z     =(rv[1].z-rv[2].z)/step;
    dv[1].r     =(rv[1].r-rv[2].r)/step;
    dv[1].g     =(rv[1].g-rv[2].g)/step;
    dv[1].b     =(rv[1].b-rv[2].b)/step;
    dv[1].a     =(rv[1].a-rv[2].a)/step;
    dv[1].u     =(rv[1].u-rv[2].u)/step;
    dv[1].v     =(rv[1].v-rv[2].v)/step;
    dv[1].w     =(rv[1].w-rv[2].w)/step;

    ev.x        =rv[1].x;
    ev.z        =rv[1].z;
    ev.r        =rv[1].r;
    ev.g        =rv[1].g;
    ev.b        =rv[1].b;
    ev.a        =rv[1].a;
    ev.u        =rv[1].u;
    ev.v        =rv[1].v;
    ev.w        =rv[1].w;

    if (dv[1].x<dv[0].x)
    {
      for (rll=rv[1].y+1; rll<=rv[2].y; rll++)
      {
        sv.x+=dv[0].x; ev.x+=dv[1].x;
        sv.z+=dv[0].z; ev.z+=dv[1].z;
        sv.r+=dv[0].r; ev.r+=dv[1].r;
        sv.g+=dv[0].g; ev.g+=dv[1].g;
        sv.b+=dv[0].b; ev.b+=dv[1].b;
        sv.a+=dv[0].a; ev.a+=dv[1].a;
        sv.u+=dv[0].u; ev.u+=dv[1].u;
        sv.v+=dv[0].v; ev.v+=dv[1].v;
        sv.w+=dv[0].w; ev.w+=dv[1].w;

        rls.x=sv.x; rle.x=ev.x;
        rls.z=sv.z; rle.z=ev.z;
        rls.r=sv.r; rle.r=ev.r;
        rls.g=sv.g; rle.g=ev.g;
        rls.b=sv.b; rle.b=ev.b;
        rls.a=sv.a; rle.a=ev.a;
        rls.u=sv.u; rle.u=ev.u;
        rls.v=sv.v; rle.v=ev.v;
        rls.w=sv.w; rle.w=ev.w;

        glRasterLine();
      }
    }
    else
    {
      for (rll=rv[1].y+1; rll<=rv[2].y; rll++)
      {
        sv.x+=dv[0].x; ev.x+=dv[1].x;
        sv.z+=dv[0].z; ev.z+=dv[1].z;
        sv.r+=dv[0].r; ev.r+=dv[1].r;
        sv.g+=dv[0].g; ev.g+=dv[1].g;
        sv.b+=dv[0].b; ev.b+=dv[1].b;
        sv.a+=dv[0].a; ev.a+=dv[1].a;
        sv.u+=dv[0].u; ev.u+=dv[1].u;
        sv.v+=dv[0].v; ev.v+=dv[1].v;
        sv.w+=dv[0].w; ev.w+=dv[1].w;

        rls.x=ev.x; rle.x=sv.x;
        rls.z=ev.z; rle.z=sv.z;
        rls.r=ev.r; rle.r=sv.r;
        rls.g=ev.g; rle.g=sv.g;
        rls.b=ev.b; rle.b=sv.b;
        rls.a=ev.a; rle.a=sv.a;
        rls.u=ev.u; rle.u=sv.u;
        rls.v=ev.v; rle.v=sv.v;
        rls.w=ev.w; rle.w=sv.w;

        glRasterLine();
      }
    }
  }

  NumTrianglePassed++;
}

/**********************************************************************************************/

void glRasterizeTriangle (const GLVERTEX* a, const GLVERTEX* b, const GLVERTEX* c)
{
  GLVERTEX*     v;
  GLdouble      f;

  if (a->Screen[Z] <= 0.0 || b->Screen[Z] <= 0.0 || c->Screen[Z] <= 0.0) return;

  v = a;

  rv[0].x = (GLI32) v->Screen[X];
  rv[0].y = (GLI32) v->Screen[Y];
  rv[0].z = v->Screen[Z];
  rv[0].w = v->Screen[W];

  rv[0].r = v->Color[R];
  rv[0].g = v->Color[G];
  rv[0].b = v->Color[B];
  rv[0].a = v->Color[A];

  rv[0].u = 1.0;
  rv[0].v = 1.0;

  v = b;

  rv[1].x = (GLI32) v->Screen[X];
  rv[1].y = (GLI32) v->Screen[Y];
  rv[1].z = v->Screen[Z];
  rv[1].w = v->Screen[W];

  rv[1].r = v->Color[R];
  rv[1].g = v->Color[G];
  rv[1].b = v->Color[B];
  rv[1].a = v->Color[A];

  rv[1].u = 1.0;
  rv[1].v = 1.0;

  v = c;

  rv[2].x = (GLI32) v->Screen[X];
  rv[2].y = (GLI32) v->Screen[Y];
  rv[2].z = v->Screen[Z];
  rv[2].w = v->Screen[W];

  rv[2].r = v->Color[R];
  rv[2].g = v->Color[G];
  rv[2].b = v->Color[B];
  rv[2].a = v->Color[A];

  rv[2].u = 1.0;
  rv[2].v = 1.0;

  glRasterTriangle();
}

/**********************************************************************************************/

EXPORT GLboolean APIENTRY glSetupSingleClipPlane
(LPGLCLIPPLANE plane, GLdouble* pos, GLdouble* norm)
{
  plane->Normal[X] = norm[X];
  plane->Normal[Y] = norm[Y];
  plane->Normal[Z] = norm[Z];
  plane->Normal[W] = 1.0;
  plane->Distance  = glVector3dDot(pos, norm);
  plane->Distance += 0.00001;
  return 1;
}

/**********************************************************************************************/

/*
BOOL GlSetUpClipPlanesF (GlI32 w, GlI32 h, GlFloat* d)
{
  GlFloat       fov, angle;
  GlFloat       s, c;
  GlVector3F    pos, normal;

  GlNumClipPlanes=4;

  // Setup frustum
  pos[X]=0; pos[Y]=0; pos[Z]=0;

  fov=((GlFloat)(w+1)/(*d))/2;
  angle=atan(1.0/fov);

  s=sin(angle);
  c=cos(angle);

  // Left clip plane
  normal[X]=s; normal[Y]=0; normal[Z]=c;
  GlSetupSingleClipPlane(0, &pos, &normal);

  // Right clip plane
  normal[X]=-s; normal[Y]=0; normal[Z]=c;
  GlSetupSingleClipPlane(1, &pos, &normal);

  fov=((GlFloat)(h+1)/(*d))/2;
  angle=atan(1.0/fov);

  s=sin(angle);
  c=cos(angle);

  // Bottom clip plane
  normal[X]=0; normal[Y]=s; normal[Z]=c;
  GlSetupSingleClipPlane(2, &pos, &normal);

  // Top clip plane
  normal[X]=0; normal[Y]=-s; normal[Z]=c;
  GlSetupSingleClipPlane(3, &pos, &normal);

  // Back clip plane
  // pos[X]=0; pos[Y]=0; pos[Z]=1;
  // normal[X]=0; normal[Y]=0; normal[Z]=1;
  // GlSetupSingleClipPlane(0, &pos, &normal);

  return TRUE;
}
*/

/**********************************************************************************************/

EXPORT GLboolean APIENTRY glClipPolygonToSinglePlane
(LPGLCLIPPLANE plane, GLI32* numin, LPGLVERTEX vin, GLI32* numout, LPGLVERTEX vout)
{
  LPGLVERTEX invert;
  LPGLVERTEX outvert;
  GLdouble   vec [3];
  GLdouble   curdot, nextdot, scale, nsc;
  GLI32      c, nextvert, count=0;
  GLboolean  curin, nextin;

  invert  = vin;
  outvert = vout;

  vec[X] = invert->Screen[X];
  vec[Y] = invert->Screen[Y];
  vec[Z] = invert->Screen[Z];

  curdot = glVector3dDot(vec, (GLdouble*)&(plane->Normal));
  curin  = (curdot>=plane->Distance);

  for (c = 0; c < (*numin); c++)
  {
    nextvert = (c + 1) % (*numin);

    if (count >= GL_MAX_POLY_VERTEX) return FALSE;

    // Keep the current vertex if it's inside the plane
    if (curin)
    {
      *outvert++ = *invert;
      count++;
    }

    // Compute dot product of next vertex
    vec[X] = vin[nextvert].Screen[X];
    vec[Y] = vin[nextvert].Screen[Y];
    vec[Z] = vin[nextvert].Screen[Z];

    nextdot = glVector3dDot(vec, (GLdouble*)&(plane->Normal));
    nextin  = (nextdot >= plane->Distance);

    // Add a clipped vertex if one end of the current edge is
    // inside the plane and the other is outside
    if (curin != nextin)
    {
      nsc = nextdot-curdot;
      if (nsc!=0.0) scale = (plane->Distance - curdot) / nsc; else scale = 0.0;

      outvert->Screen[X] = invert->Screen[X] + ((vin[nextvert].Screen[X] - invert->Screen[X]) * scale);
      outvert->Screen[Y] = invert->Screen[Y] + ((vin[nextvert].Screen[Y] - invert->Screen[Y]) * scale);
      outvert->Screen[Z] = invert->Screen[Z] + ((vin[nextvert].Screen[Z] - invert->Screen[Z]) * scale);
      outvert->Screen[W] = invert->Screen[W] + ((vin[nextvert].Screen[W] - invert->Screen[W]) * scale);

      outvert->Color[R]  = invert->Color[R] + ((vin[nextvert].Color[R] - invert->Color[R]) * scale);
      outvert->Color[G]  = invert->Color[G] + ((vin[nextvert].Color[G] - invert->Color[G]) * scale);
      outvert->Color[B]  = invert->Color[B] + ((vin[nextvert].Color[B] - invert->Color[B]) * scale);
      outvert->Color[A]  = invert->Color[A] + ((vin[nextvert].Color[A] - invert->Color[A]) * scale);

      outvert->Tex[X]    = invert->Tex[X] + ((vin[nextvert].Tex[X] - invert->Tex[X]) * scale);
      outvert->Tex[Y]    = invert->Tex[Y] + ((vin[nextvert].Tex[Y] - invert->Tex[Y]) * scale);

      outvert++;
      count++;
    }

    curdot = nextdot;
    curin  = nextin;
    invert++;
  }

  (*numout) = count;

  if ((*numout) < 3) return 0;

  return 1;
}

/**********************************************************************************************/

EXPORT GLboolean APIENTRY glClipPolygon
(LPGLCLIPPLANE ClipPlane, GLI32 NumClipPlane, GLI32* numvin, LPGLVERTEX vin,
 GLI32* numvout, LPGLVERTEX vout)
{
  LPGLVERTEX    curlist;
  GLI32         c, curtemp, curnumvin;

  curtemp   = 0;
  curlist   = vin;
  curnumvin = (*numvin);

  for (c = 0; c < NumClipPlane-1; c++)
  {
    if
    (
      glClipPolygonToSinglePlane
      (&(ClipPlane[c]), &curnumvin, curlist, numvout, &(TempVertices[curtemp][0])) == FALSE
    ) return FALSE;

    curlist    = (LPGLVERTEX)&(TempVertices[curtemp][0]);
    curnumvin  = (*numvout);
    curtemp   ^= 1;
  }

  return glClipPolygonToSinglePlane
  (&(ClipPlane[NumClipPlane-1]), &curnumvin, curlist, numvout, vout);
}

/**********************************************************************************************/

void glPolygonToClipping (LPGLPOLYGON Polygon, GLdouble* Matrix)
{
  GLint c;

  for (c = 0; c < Polygon->NumVertex; c++)
  {
    glMatrix4dTransVector4d(Polygon->Vertex[c].Screen, Polygon->Vertex[c].Eye, Matrix);
  }
}

/**********************************************************************************************/

void glPolygonToViewport (LPGLPOLYGON Polygon, GLI32* Viewport)
{
  GLdouble      sx, sy;
  GLenum        c;

  sx = (GLdouble)Viewport[2] / 2.0;
  sy = (GLdouble)Viewport[3] / 2.0;

  for (c = 0; c < Polygon->NumVertex; c++)
  {
    glVector4dToVector3d(Polygon->Vertex[c].Screen);

    Polygon->Vertex[c].Screen[Z] += 1.0;
    Polygon->Vertex[c].Screen[Z] /= 2.0;

    Polygon->Vertex[c].Screen[X] *= sx;
    Polygon->Vertex[c].Screen[Y] *= sy;

    Polygon->Vertex[c].Screen[X] += sx;
    Polygon->Vertex[c].Screen[Y] += sy;
  }
}

/**********************************************************************************************/

void glIlluminatePolygon (LPGLPOLYGON Polygon, LPGLRENDERBLOCK Block)
{
  GLdouble Vertex2Eye [3];
  GLdouble Vertex2Light [3];
  GLdouble EyePos [3] = {0, 0, 0};
  GLdouble Color [4];
  GLdouble Diffuse;
  GLdouble Specular;
  GLI32    c, d;

  for (c = 0; c < Polygon->NumVertex; c++)
  {
    // Assign material emission to vertex color
    Polygon->Vertex[c].Color[R] = Polygon->FrontFace.Emission[R];
    Polygon->Vertex[c].Color[G] = Polygon->FrontFace.Emission[G];
    Polygon->Vertex[c].Color[B] = Polygon->FrontFace.Emission[B];

    Polygon->Vertex[c].Color[A] = Polygon->FrontFace.Diffuse[A];

    for (d = 0; d < GL_MAX_USER_LIGHTS; d++)
    {
      if (Block->Light[d].On)
      {
        glVector3dSub(Vertex2Light, Block->Light[d].Position, Polygon->Vertex[c].Eye);
        glVector3dNormalize(Vertex2Light);

        glVector3dSub(Vertex2Eye, EyePos, Polygon->Vertex[c].Eye);
        glVector3dNormalize(Vertex2Eye);

        // Compute diffuse intensity
        Diffuse = glVector3dDot(Vertex2Light, Polygon->Vertex[c].Normal);
        if (Diffuse < 0.0) Diffuse = 0.0;

        // Compute specular intensity
        Specular = glVector3dDot(Vertex2Light, Vertex2Eye);
        if (Specular < 0.0) Specular = 0.0;

        // Raise specular to the power of shininess
        //

        // Set color to black
        Color[R] = 0.0;
        Color[G] = 0.0;
        Color[B] = 0.0;

        // Add ambient contribution
        Color[R] += Polygon->FrontFace.Ambient[R] * Block->Light[d].Ambient[R];
        Color[G] += Polygon->FrontFace.Ambient[G] * Block->Light[d].Ambient[G];
        Color[B] += Polygon->FrontFace.Ambient[B] * Block->Light[d].Ambient[B];

        // Add diffuse contribution
        Color[R] += Polygon->FrontFace.Diffuse[R] * Block->Light[d].Diffuse[R] * Diffuse;
        Color[G] += Polygon->FrontFace.Diffuse[G] * Block->Light[d].Diffuse[G] * Diffuse;
        Color[B] += Polygon->FrontFace.Diffuse[B] * Block->Light[d].Diffuse[B] * Diffuse;

        // Add specular contribution
        Color[R] += Polygon->FrontFace.Specular[R] * Block->Light[d].Specular[R] * Specular;
        Color[G] += Polygon->FrontFace.Specular[G] * Block->Light[d].Specular[G] * Specular;
        Color[B] += Polygon->FrontFace.Specular[B] * Block->Light[d].Specular[B] * Specular;

        // Add resulting color to vertex
        Polygon->Vertex[c].Color[R] += Color[R];
        Polygon->Vertex[c].Color[G] += Color[G];
        Polygon->Vertex[c].Color[B] += Color[B];
      }
    }
  }
}

/**********************************************************************************************/

GLboolean glRasterSetContextAndBlock (LPGLRENDERCONTEXT Context, LPGLRENDERBLOCK Block)
{
  if (Context == NULL || Block == NULL) return 0;

  RasterRenderContext = Context;
  RasterRenderBlock   = Block;
  RasterDepthBuffer   = (GLfloat*)Context->DepthBuffer;

  GlClip[0][X] = 0; GlClip[1][X] = Context->Width - 1;
  GlClip[0][Y] = 0; GlClip[1][Y] = Context->Height - 1;

  return 1;
}

/**********************************************************************************************/

void glRasterizePolygon (LPGLRENDERCONTEXT Context, LPGLRENDERBLOCK Block, LPGLPOLYGON Polygon)
{
  GLPOLYGON     ClippedPolygon;
  GLenum        c, n;

  n=Polygon->NumVertex-2;

  if (n<1) return;

  if (glCreatePolygon(&(ClippedPolygon), GL_MAX_POLY_VERTEX)==FALSE) return;

  glRasterSetContextAndBlock(Context, Block);

  glIlluminatePolygon(Polygon, Block);

  glPolygonToClipping(Polygon, Block->XForm.NMatrix[GL_MATRIX_PROJECTION]);

  glPolygonToViewport(Polygon, &(Context->ViewPort));
  for (c = 0; c < n; c++)
  {
    glRasterizeTriangle(Polygon->Vertex, Polygon->Vertex + c + 1, Polygon->Vertex + c + 2);
  }

  glDestroyPolygon(&(ClippedPolygon));
}

/**********************************************************************************************/

GLboolean glRasterSetContext (LPGLRENDERCONTEXT Context)
{
  if (Context == NULL) return FALSE;

  NumTrianglePassed = 0;
  NumLinePassed     = 0;
  NumPixelPassed    = 0;

  RasterRenderContext = Context;
  RasterRenderBlock   = &(Context->Block);
  RasterDepthBuffer   = (GLfloat*)Context->DepthBuffer;

  GlClip[0][X] = 0; GlClip[1][X] = RasterRenderContext->Width-1;
  GlClip[0][Y] = 0; GlClip[1][Y] = RasterRenderContext->Height-1;

  return TRUE;
}

/**********************************************************************************************/

void glRasterizeBlock (LPGLRENDERCONTEXT Context, LPGLRENDERBLOCK Block)
{
  LPGLLIST      List;
  LPGLPOLYGON   Polygon;
  GLenum        Count = 0;

  if (Context == NULL || Block == NULL) return;

  NumTrianglePassed = 0;
  NumLinePassed     = 0;
  NumPixelPassed    = 0;

  RasterRenderContext = Context;
  RasterRenderBlock   = Block;
  RasterDepthBuffer   = (GLfloat*)Context->DepthBuffer;

  GlClip[0][X] = 0; GlClip[1][X] = RasterRenderContext->Width-1;
  GlClip[0][Y] = 0; GlClip[1][Y] = RasterRenderContext->Height-1;

  List    = &(Block->PolygonList);
  Polygon = glList_GetPointer(List, Count++);

  while (Polygon)
  {
    Polygon = glList_GetPointer(List, Count++);
  }
}
