
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

#include "../../include/tinygl.h"
#include "../internal/glstd.h"
#include "../internal/gllist.h"
#include "../internal/glmain.h"
#include "../internal/glvector.h"

/*****************************************************************************************************/

#define X 0
#define Y 1
#define Z 2
#define W 3

// Variables local to this module

static char szTemp[256];

/*****************************************************************************************************/

static void glMatrixGLtoMatrix4d(const GLdouble *m1, GLdouble *m2) {
    long c, d;
    for (d = 0; d < 4; d++)
        for (c = 0; c < 4; c++)
            m2[d * 4 + c] = m1[c * 4 + d];
}

/*****************************************************************************************************/

static void glMatrix4dtoMatrixGL(const GLdouble *m1, GLdouble *m2) {
    long c, d;
    for (d = 0; d < 4; d++)
        for (c = 0; c < 4; c++)
            m2[d * 4 + c] = m1[c * 4 + d];
}

/*****************************************************************************************************/

EXPORT void APIENTRY gluOrtho2D(GLdouble left, GLdouble right, GLdouble bottom,
                                GLdouble top) {}

/*****************************************************************************************************/

EXPORT void APIENTRY gluPerspective(GLdouble fovy, GLdouble aspect,
                                    GLdouble zNear, GLdouble zFar) {}

/*****************************************************************************************************/

EXPORT void APIENTRY gluPickMatrix(GLdouble x, GLdouble y, GLdouble width,
                                   GLdouble height, GLint viewport[4]) {}

/*****************************************************************************************************/

EXPORT void APIENTRY gluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez,
                               GLdouble centerx, GLdouble centery,
                               GLdouble centerz, GLdouble upx, GLdouble upy,
                               GLdouble upz) {}

/*****************************************************************************************************/

EXPORT int APIENTRY gluProject(GLdouble objx, GLdouble objy, GLdouble objz,
                               const GLdouble momatrix[16],
                               const GLdouble prmatrix[16],
                               const GLint viewport[4], GLdouble *winx,
                               GLdouble *winy, GLdouble *winz) {
    GLdouble mat1[4][4];
    GLdouble mat2[4][4];
    GLdouble vec[4];
    GLdouble sx, sy, w;

    // Check pointers
    if (momatrix == NULL || prmatrix == NULL || winx == NULL || winy == NULL ||
        winz == NULL) {
        glSetErrorCode(GL_INVALID_OPERATION);
        return 0;
    }

    // Get viewport center
    sx = (GLdouble)viewport[2] / 2.0;
    sy = (GLdouble)viewport[3] / 2.0;

    // Assign original vector
    vec[X] = objx;
    vec[Y] = objy;
    vec[Z] = objz;
    vec[W] = 1.0;

    // Transform point with modelview matrix
    glMatrix4dTransVector4d(vec, vec, momatrix);

    // Transform point with projection matrix
    glMatrix4dTransVector4d(vec, vec, prmatrix);

    // Transform 4d homogeneous point to 3d point
    glVector4dToVector3d(vec);

    // Scale point to viewport
    vec[X] *= sx;
    vec[Y] *= sy;

    vec[X] += sx;
    vec[Y] += sy;

    // Assign result
    (*winx) = (GLdouble)viewport[0] + vec[X];
    (*winy) = (GLdouble)viewport[1] + vec[Y];
    (*winz) = vec[Z];

    return 1;
}

/*****************************************************************************************************/

EXPORT int APIENTRY gluUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
                                 const GLdouble momatrix[16],
                                 const GLdouble prmatrix[16],
                                 const GLint viewport[4], GLdouble *objx,
                                 GLdouble *objy, GLdouble *objz) {
    GLdouble mat1[4][4];
    GLdouble mat2[4][4];
    GLdouble vec[4];
    GLdouble sx, sy, w;
    GLint c;

    // Check pointers
    if (momatrix == NULL || prmatrix == NULL || objx == NULL || objy == NULL ||
        objz == NULL) {
        glSetErrorCode(GL_INVALID_OPERATION);
        return 0;
    }

    // Get viewport center
    sx = ((GLdouble)viewport[2] - (GLdouble)viewport[0]) / 2.0;
    sy = ((GLdouble)viewport[3] - (GLdouble)viewport[1]) / 2.0;

    memcpy(mat1, momatrix, sizeof(GLdouble) * 16);
    memcpy(mat2, prmatrix, sizeof(GLdouble) * 16);

    // Inverse matrices (time consuming)
    glMatrix4dInverse(mat1, mat1);
    glMatrix4dInverse(mat2, mat2);

    // Assign original vector
    vec[X] = winx - sx;
    vec[Y] = winy - sy;
    vec[Z] = winz;
    vec[W] = 1.0;

    // Divide point by viewport
    if (sx != 0.0)
        vec[X] /= sx;
    if (sy != 0.0)
        vec[Y] /= sy;

    // Transform point with inverse of projection matrix
    glMatrix4dTransVector4d(vec, vec, mat2);

    // Transform point with inverse of modelview matrix
    glMatrix4dTransVector4d(vec, vec, mat1);

    // Assign result
    (*objx) = vec[X];
    (*objy) = vec[Y];
    (*objz) = vec[Z];

    return 1;
}

/*****************************************************************************************************/
