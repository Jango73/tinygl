
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


    Vector

\************************************************************************/

#ifndef GLVECTOR_H_INCLUDED
#define GLVECTOR_H_INCLUDED

/**********************************************************************************************/

#include "../../include/tinygl.h"

/**********************************************************************************************/

#define GL_PI 3.1415926535

#define GL_MATOP_PRECONCATENATE 0
#define GL_MATOP_POSTCONCATENATE 1
#define GL_MATOP_REPLACE 2

/**********************************************************************************************/

void glVector3fAdd(GLfloat[3], GLfloat[3], GLfloat[3]);
void glVector3dAdd(GLdouble[3], GLdouble[3], GLdouble[3]);
void glVector3fSub(GLfloat[3], GLfloat[3], GLfloat[3]);
void glVector3dSub(GLdouble[3], GLdouble[3], GLdouble[3]);
void glVector3fInverseScale(GLfloat[3], GLfloat);
void glVector3dInverseScale(GLdouble[3], GLdouble);
void glVector3fCross(GLfloat[3], GLfloat[3], GLfloat[3]);
void glVector3dCross(GLdouble[3], GLdouble[3], GLdouble[3]);
GLfloat glVector3fDot(GLfloat[3], GLfloat[3]);
GLdouble glVector3dDot(GLdouble[3], GLdouble[3]);
GLfloat glVector3fLength(GLfloat[3]);
GLdouble glVector3dLength(GLdouble[3]);
void glVector3fNormalize(GLfloat[3]);
void glVector3dNormalize(GLdouble[3]);
void glVector3fTriangleNormal(GLfloat[3], GLfloat[3], GLfloat[3], GLfloat[3]);
void glVector3dTriangleNormal(GLdouble[3], GLdouble[3], GLdouble[3], GLdouble[3]);
void glVector3fRotateX(GLfloat[3], GLfloat);
void glVector3dRotateX(GLdouble[3], GLdouble);
void glVector3fRotateY(GLfloat[3], GLfloat);
void glVector3dRotateY(GLdouble[3], GLdouble);
void glVector3fRotateZ(GLfloat[3], GLfloat);
void glVector3dRotateZ(GLdouble[3], GLdouble);
void glVector4fToVector3f(GLfloat[4]);
void glVector4dToVector3d(GLdouble[4]);

/**********************************************************************************************/

void glMatrix4fZero(GLMATRIX4F);
void glMatrix4dZero(GLMATRIX4D);
void glMatrix4fIdentity(GLMATRIX4F);
void glMatrix4dIdentity(GLMATRIX4D);
void glMatrix4fTimes(GLMATRIX4F, GLMATRIX4F, GLMATRIX4F);
void glMatrix4dTimes(GLMATRIX4D, GLMATRIX4D, GLMATRIX4D);
void glMatrix4dConcat(GLMATRIX4D, GLMATRIX4D, GLMATRIX4D, GLenum);
void glMatrix4fTranspose(GLMATRIX4F, GLMATRIX4F);
void glMatrix4dTranspose(GLMATRIX4D, GLMATRIX4D);
void glMatrix4fTransVector3f(GLfloat *, GLfloat *, GLMATRIX4F);
void glMatrix4dTransVector3d(GLdouble *, GLdouble *, GLMATRIX4D);
void glMatrix4dTransVector4d(GLdouble *, GLdouble *, GLMATRIX4D);
void TV3M4(GLdouble *, GLdouble *, GLMATRIX4D);
void glMatrix4fInvTransVector3f(GLfloat *, GLfloat *, GLMATRIX4F);
void glMatrix4dInvTransVector3d(GLdouble *, GLdouble *, GLMATRIX4D);
void glCreateScalingMatrix4f(GLMATRIX4F, const GLfloat *);
void glCreateScalingMatrix4d(GLMATRIX4D, const GLdouble *);
void glCreateTranslationMatrix4f(GLMATRIX4F, const GLfloat *);
void glCreateTranslationMatrix4d(GLMATRIX4D, const GLdouble *);
void glCreateRotationMatrix4f(GLMATRIX4F, const GLfloat *);
void glCreateRotationMatrix4d(GLMATRIX4D, const GLdouble *);
void glMatrix4dRotationLine(GLMATRIX4D, GLdouble, GLdouble[3], GLdouble[3]);
void glMatrix4fInverse(GLMATRIX4F, GLMATRIX4F);
void glMatrix4dInverse(GLMATRIX4D, GLMATRIX4D);

/**********************************************************************************************/

#endif // GLVECTOR_H_INCLUDED
