
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

void glVector3fAdd(GLfloat *, GLfloat *, GLfloat *);
void glVector3dAdd(GLdouble *, GLdouble *, GLdouble *);
void glVector3fSub(GLfloat *, GLfloat *, GLfloat *);
void glVector3dSub(GLdouble *, GLdouble *, GLdouble *);
void glVector3fInverseScale(GLfloat *, GLfloat);
void glVector3dInverseScale(GLdouble *, GLdouble);
void glVector3fCross(GLfloat *, GLfloat *, GLfloat *);
void glVector3dCross(GLdouble *, GLdouble *, GLdouble *);
GLfloat glVector3fDot(GLfloat *, GLfloat *);
GLdouble glVector3dDot(GLdouble *, GLdouble *);
GLfloat glVector3fLength(GLfloat *);
GLdouble glVector3dLength(GLdouble *);
void glVector3fNormalize(GLfloat *);
void glVector3dNormalize(GLdouble *);
void glVector3fTriangleNormal(GLfloat *, GLfloat *, GLfloat *, GLfloat *);
void glVector3dTriangleNormal(GLdouble *, GLdouble *, GLdouble *, GLdouble *);
void glVector3fRotateX(GLfloat *, GLfloat);
void glVector3dRotateX(GLdouble *, GLdouble);
void glVector3fRotateY(GLfloat *, GLfloat);
void glVector3dRotateY(GLdouble *, GLdouble);
void glVector3fRotateZ(GLfloat *, GLfloat);
void glVector3dRotateZ(GLdouble *, GLdouble);
void glVector4fToVector3f(GLfloat *);
void glVector4dToVector3d(GLdouble *);

/**********************************************************************************************/

void glMatrix4fZero(GLfloat[4][4]);
void glMatrix4dZero(GLdouble[4][4]);
void glMatrix4fIdentity(GLfloat[4][4]);
void glMatrix4dIdentity(GLdouble[4][4]);
void glMatrix4fTimes(GLfloat[4][4], GLfloat[4][4], GLfloat[4][4]);
void glMatrix4dTimes(GLdouble[4][4], GLdouble[4][4], GLdouble[4][4]);
void glMatrix4dConcat(GLdouble[4][4], GLdouble[4][4], GLdouble[4][4], GLenum);
void glMatrix4fTranspose(GLfloat[4][4], GLfloat[4][4]);
void glMatrix4dTranspose(GLdouble[4][4], GLdouble[4][4]);
void glMatrix4fTransVector3f(GLfloat *, GLfloat *, GLfloat[4][4]);
void glMatrix4dTransVector3d(GLdouble *, GLdouble *, GLdouble[4][4]);
void glMatrix4dTransVector4d(GLdouble *, GLdouble *, GLdouble[4][4]);
void TV3M4(GLdouble *, GLdouble *, GLdouble[4][4]);
void glMatrix4fInvTransVector3f(GLfloat *, GLfloat *, GLfloat[4][4]);
void glMatrix4dInvTransVector3d(GLdouble *, GLdouble *, GLdouble[4][4]);
void glCreateScalingMatrix4f(GLfloat[4][4], const GLfloat *);
void glCreateScalingMatrix4d(GLdouble[4][4], const GLdouble *);
void glCreateTranslationMatrix4f(GLfloat[4][4], const GLfloat *);
void glCreateTranslationMatrix4d(GLdouble[4][4], const GLdouble *);
void glCreateRotationMatrix4f(GLfloat[4][4], const GLfloat *);
void glCreateRotationMatrix4d(GLdouble[4][4], const GLdouble *);
void glMatrix4dRotationLine(GLdouble[4][4], GLdouble, GLdouble *, GLdouble *);
void glMatrix4fInverse(GLfloat[4][4], GLfloat[4][4]);
void glMatrix4dInverse(GLdouble[4][4], GLdouble[4][4]);

/**********************************************************************************************/

#endif // GLVECTOR_H_INCLUDED
