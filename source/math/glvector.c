
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

#include "../../include/tinygl.h"

#include "../internal/glstd.h"
#include "../internal/gllist.h"
#include "../internal/glmain.h"
#include "../internal/glvector.h"

#define GLmatrix4f(x) GLfloat x[4][4]
#define GLmatrix4d(x) GLdouble x[4][4]

#define GL_CLIP_PLANE_EPSILON 0.0001f

/**********************************************************************************************/

void glVector3fAdd(GLfloat v1[3], GLfloat v2[3], GLfloat v3[3]) {
    v1[X] = v2[X] + v3[X];
    v1[Y] = v2[Y] + v3[Y];
    v1[Z] = v2[Z] + v3[Z];
}

void glVector3dAdd(GLdouble v1[3], GLdouble v2[3], GLdouble v3[3]) {
    v1[X] = v2[X] + v3[X];
    v1[Y] = v2[Y] + v3[Y];
    v1[Z] = v2[Z] + v3[Z];
}

/**********************************************************************************************/

void glVector3fSub(GLfloat v1[3], GLfloat v2[3], GLfloat v3[3]) {
    v1[X] = v2[X] - v3[X];
    v1[Y] = v2[Y] - v3[Y];
    v1[Z] = v2[Z] - v3[Z];
}

void glVector3dSub(GLdouble v1[3], GLdouble v2[3], GLdouble v3[3]) {
    v1[X] = v2[X] - v3[X];
    v1[Y] = v2[Y] - v3[Y];
    v1[Z] = v2[Z] - v3[Z];
}

/**********************************************************************************************/

void glVector3fInverseScale(GLfloat vec[3], GLfloat k) {
    if (k != 0.0) {
        vec[X] /= k;
        vec[Y] /= k;
        vec[Z] /= k;
    }
}

void glVector3dInverseScale(GLdouble vec[3], GLdouble k) {
    if (k != 0.0) {
        vec[X] /= k;
        vec[Y] /= k;
        vec[Z] /= k;
    }
}

/**********************************************************************************************/

void glVector3fCross(GLfloat res[3], GLfloat v1[3], GLfloat v2[3]) {
    res[X] = v1[Y] * v2[Z] - v1[Z] * v2[Y];
    res[Y] = v1[Z] * v2[X] - v1[X] * v2[Z];
    res[Z] = v1[X] * v2[Y] - v1[Y] * v2[X];
}

void glVector3dCross(GLdouble res[3], GLdouble v1[3], GLdouble v2[3]) {
    res[X] = v1[Y] * v2[Z] - v1[Z] * v2[Y];
    res[Y] = v1[Z] * v2[X] - v1[X] * v2[Z];
    res[Z] = v1[X] * v2[Y] - v1[Y] * v2[X];
}

/**********************************************************************************************/

GLfloat glVector3fDot(GLfloat v1[3], GLfloat v2[3]) {
    return v1[X] * v2[X] + v1[Y] * v2[Y] + v1[Z] * v2[Z];
}

GLdouble glVector3dDot(GLdouble v1[3], GLdouble v2[3]) {
    return v1[X] * v2[X] + v1[Y] * v2[Y] + v1[Z] * v2[Z];
}

/**********************************************************************************************/

GLfloat glVector3fLength(GLfloat vec[3]) {
    return sqrt(vec[X] * vec[X] + vec[Y] * vec[Y] + vec[Z] * vec[Z]);
}

GLdouble glVector3dLength(GLdouble vec[3]) {
    return sqrt(vec[X] * vec[X] + vec[Y] * vec[Y] + vec[Z] * vec[Z]);
}

/**********************************************************************************************/

void glVector3fNormalize(GLfloat vec[3]) {
    GLfloat len;

    len = glVector3fLength(vec);
    if (len != 0.0)
        glVector3fInverseScale(vec, len);
    else {
        vec[X] = 0;
        vec[Y] = 0;
        vec[Z] = 0;
    }
}

void glVector3dNormalize(GLdouble vec[3]) {
    GLdouble len;

    len = glVector3dLength(vec);
    if (len != 0.0)
        glVector3dInverseScale(vec, len);
    else {
        vec[X] = 0;
        vec[Y] = 0;
        vec[Z] = 0;
    }
}

/**********************************************************************************************/

void glVector3fTriangleNormal(GLfloat res[3], GLfloat v1[3], GLfloat v2[3],
                              GLfloat v3[3]) {
    GLfloat tv1[3], tv2[3];
    GLfloat len;

    glVector3fSub(&tv1, v3, v1);
    glVector3fSub(&tv2, v2, v1);
    glVector3fCross(res, tv1, tv2);
    glVector3fNormalize(res);
}

void glVector3dTriangleNormal(GLdouble res[3], GLdouble v1[3], GLdouble v2[3],
                              GLdouble v3[3]) {
    GLdouble tv1[3], tv2[3];
    GLdouble len;

    glVector3dSub(&tv1, v3, v1);
    glVector3dSub(&tv2, v2, v1);
    glVector3dCross(res, tv1, tv2);
    glVector3dNormalize(res);
}

/**********************************************************************************************/

void glVector3fRotateX(GLfloat vec[3], GLfloat a) {
    GLfloat tm[4][4];
    GLfloat tv[3];

    tv[X] = a;
    tv[Y] = 0;
    tv[Z] = 0;
    glCreateRotationMatrix4f(tm, tv);
    glMatrix4fTransVector3f(vec, vec, tm);
}

void glVector3dRotateX(GLdouble vec[3], GLdouble a) {
    GLdouble tm[4][4];
    GLdouble tv[3];

    tv[X] = a;
    tv[Y] = 0;
    tv[Z] = 0;
    glCreateRotationMatrix4d(tm, tv);
    glMatrix4dTransVector3d(vec, vec, tm);
}

/**********************************************************************************************/

void glVector3fRotateY(GLfloat vec[3], GLfloat a) {
    GLfloat tm[4][4];
    GLfloat tv[3];

    tv[X] = 0;
    tv[Y] = a;
    tv[Z] = 0;
    glCreateRotationMatrix4f(tm, tv);
    glMatrix4fTransVector3f(vec, vec, tm);
}

void glVector3dRotateY(GLdouble vec[3], GLdouble a) {
    GLdouble tm[4][4];
    GLdouble tv[3];

    tv[X] = 0;
    tv[Y] = a;
    tv[Z] = 0;
    glCreateRotationMatrix4d(tm, tv);
    glMatrix4dTransVector3d(vec, vec, tm);
}

/**********************************************************************************************/

void glVector3fRotateZ(GLfloat vec[3], GLfloat a) {
    GLfloat tm[4][4];
    GLfloat tv[3];

    tv[X] = 0;
    tv[Y] = 0;
    tv[Z] = a;
    glCreateRotationMatrix4f(tm, tv);
    glMatrix4fTransVector3f(vec, vec, tm);
}

void glVector3dRotateZ(GLdouble vec[3], GLdouble a) {
    GLdouble tm[4][4];
    GLdouble tv[3];

    tv[X] = 0;
    tv[Y] = 0;
    tv[Z] = a;
    glCreateRotationMatrix4d(tm, tv);
    glMatrix4dTransVector3d(vec, vec, tm);
}

/**********************************************************************************************/

void glVector4fToVector3f(GLfloat vec[4]) {
    if (vec[W] == 0.0)
        vec[W] = 1.0 / 0.00001;
    else
        vec[W] = 1.0 / vec[W];
    vec[X] *= vec[W];
    vec[Y] *= vec[W];
    vec[Z] *= vec[W];
}

void glVector4dToVector3d(GLdouble vec[4]) {
    if (vec[W] == 0.0)
        vec[W] = 1.0 / 0.00001;
    else
        vec[W] = 1.0 / vec[W];
    vec[X] *= vec[W];
    vec[Y] *= vec[W];
    vec[Z] *= vec[W];
}

/**********************************************************************************************/

void glMatrix4fZero(GLmatrix4f(mat)) {
    register GLU32 i, j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            mat[i][j] = 0.0;
}

void glMatrix4dZero(GLmatrix4d(mat)) {
    register GLU32 i, j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            mat[i][j] = 0.0;
}

/**********************************************************************************************/

void glMatrix4fIdentity(GLmatrix4f(mat)) {
    register GLU32 i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (i == j)
                mat[i][j] = 1.0;
            else
                mat[i][j] = 0.0;
        }
    }
}

void glMatrix4dIdentity(GLmatrix4d(mat)) {
    register GLU32 i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (i == j)
                mat[i][j] = 1.0;
            else
                mat[i][j] = 0.0;
        }
    }
}

/**********************************************************************************************/

void glMatrix4fTimes(GLmatrix4f(res), GLmatrix4f(mat1), GLmatrix4f(mat2)) {
    register GLU32 i, j, k;
    GLmatrix4f(temp);

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            temp[i][j] = 0.0;
            for (k = 0; k < 4; k++) {
                temp[i][j] += mat1[i][k] * mat2[k][j];
            }
        }
    }
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            res[i][j] = temp[i][j];
}

void glMatrix4dTimes(GLmatrix4d(res), GLmatrix4d(mat1), GLmatrix4d(mat2)) {
    register GLU32 i, j, k;
    GLmatrix4d(temp);

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            temp[i][j] = 0.0;
            for (k = 0; k < 4; k++) {
                temp[i][j] += mat1[i][k] * mat2[k][j];
            }
        }
    }
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            res[i][j] = temp[i][j];
}

/**********************************************************************************************/

void glMatrix4dConcat(GLdouble res[4][4], GLdouble mat1[4][4],
                      GLdouble mat2[4][4], GLenum op) {
    switch (op) {
    case GL_MATOP_PRECONCATENATE:
        glMatrix4dTimes((GLdouble *)res, (GLdouble *)mat2, (GLdouble *)mat1);
        break;
    case GL_MATOP_POSTCONCATENATE:
        glMatrix4dTimes((GLdouble *)res, (GLdouble *)mat1, (GLdouble *)mat2);
        break;
    case GL_MATOP_REPLACE:
        memcpy(res, mat1, sizeof(GLdouble) * 16);
        break;
    }
}

/**********************************************************************************************/

void glMatrix4dShift(GLdouble mat[4][4], GLdouble vec[3], GLenum op) {
    GLdouble tmp[4][4];
    glMatrix4dIdentity((GLdouble *)tmp);
    tmp[0][3] = vec[X];
    tmp[1][3] = vec[Y];
    tmp[2][3] = vec[Z];
    glMatrix4dConcat(mat, mat, tmp, op);
}

/**********************************************************************************************/

void glMatrix4fTranspose(GLmatrix4f(res), GLmatrix4f(mat)) {
    register GLU32 i, j;
    GLmatrix4f(temp);
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            temp[i][j] = mat[j][i];
        }
    }
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            res[i][j] = temp[i][j];
        }
    }
}

void glMatrix4dTranspose(GLmatrix4d(res), GLmatrix4d(mat)) {
    register GLU32 i, j;
    GLmatrix4d(temp);
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            temp[i][j] = mat[j][i];
        }
    }
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            res[i][j] = temp[i][j];
        }
    }
}

/**********************************************************************************************/

void glMatrix4fTransVector3f(GLfloat *res, GLfloat *vec, GLmatrix4f(mat)) {
    GLdouble tmp[3];

    tmp[X] = mat[0][0] * vec[X] + mat[0][1] * vec[Y] + mat[0][2] * vec[Z] +
             mat[0][3];
    tmp[Y] = mat[1][0] * vec[X] + mat[1][1] * vec[Y] + mat[1][2] * vec[Z] +
             mat[1][3];
    tmp[Z] = mat[2][0] * vec[X] + mat[2][1] * vec[Y] + mat[2][2] * vec[Z] +
             mat[2][3];

    res[X] = tmp[X];
    res[Y] = tmp[Y];
    res[Z] = tmp[Z];
}

void glMatrix4dTransVector3d(GLdouble *res, GLdouble *vec, GLmatrix4d(mat)) {
    GLdouble tmp[3];

    tmp[X] = mat[0][0] * vec[X] + mat[0][1] * vec[Y] + mat[0][2] * vec[Z] +
             mat[0][3];
    tmp[Y] = mat[1][0] * vec[X] + mat[1][1] * vec[Y] + mat[1][2] * vec[Z] +
             mat[1][3];
    tmp[Z] = mat[2][0] * vec[X] + mat[2][1] * vec[Y] + mat[2][2] * vec[Z] +
             mat[2][3];

    res[X] = tmp[X];
    res[Y] = tmp[Y];
    res[Z] = tmp[Z];
}

/**********************************************************************************************/

void glMatrix4dTransVector4d(GLdouble *res, GLdouble *vec, GLmatrix4d(mat)) {
    GLdouble tmp[4];

    tmp[X] = mat[0][0] * vec[X] + mat[0][1] * vec[Y] + mat[0][2] * vec[Z] +
             mat[0][3] * vec[W];
    tmp[Y] = mat[1][0] * vec[X] + mat[1][1] * vec[Y] + mat[1][2] * vec[Z] +
             mat[1][3] * vec[W];
    tmp[Z] = mat[2][0] * vec[X] + mat[2][1] * vec[Y] + mat[2][2] * vec[Z] +
             mat[2][3] * vec[W];
    tmp[W] = mat[3][0] * vec[X] + mat[3][1] * vec[Y] + mat[3][2] * vec[Z] +
             mat[3][3] * vec[W];

    res[X] = tmp[X];
    res[Y] = tmp[Y];
    res[Z] = tmp[Z];
    res[W] = tmp[W];
}

/**********************************************************************************************/

void TV3M4(GLdouble *res, GLdouble *vec, GLdouble mat[4][4]) {
    GLdouble tmp[4];

    tmp[X] = mat[0][0] * vec[X] + mat[0][1] * vec[Y] + mat[0][2] * vec[Z] +
             mat[0][3] * vec[W];
    tmp[Y] = mat[1][0] * vec[X] + mat[1][1] * vec[Y] + mat[1][2] * vec[Z] +
             mat[1][3] * vec[W];
    tmp[Z] = mat[2][0] * vec[X] + mat[2][1] * vec[Y] + mat[2][2] * vec[Z] +
             mat[2][3] * vec[W];
    tmp[W] = mat[3][0] * vec[X] + mat[3][1] * vec[Y] + mat[3][2] * vec[Z] +
             mat[3][3] * vec[W];

    if (tmp[W] == 0.0)
        tmp[W] = 1.0 / 0.00001;
    else
        tmp[W] = 1.0 / tmp[W];

    tmp[X] *= tmp[W];
    tmp[Y] *= tmp[W];
    tmp[Z] *= tmp[W];

    res[X] = tmp[X];
    res[Y] = tmp[Y];
    res[Z] = tmp[Z];
    res[W] = tmp[W];
}

/**********************************************************************************************/

void glMatrix4fInvTransVector3f(GLfloat *res, GLfloat *vec, GLmatrix4f(mat)) {
    GLmatrix4f(temp);
    register GLU32 i;
    GLfloat ans[4];

    glMatrix4fInverse(&temp, mat);

    for (i = 0; i < 3; i++) {
        ans[i] = vec[X] * temp[0][i] + vec[Y] * temp[1][i] +
                 vec[Z] * temp[2][i] + mat[3][i];
    }

    res[X] = ans[0];
    res[Y] = ans[1];
    res[Z] = ans[2];
}

void glMatrix4dInvTransVector3d(GLdouble *res, GLdouble *vec, GLmatrix4d(mat)) {
    GLmatrix4d(temp);
    register GLU32 i;
    GLdouble ans[4];

    glMatrix4dInverse(&temp, mat);

    for (i = 0; i < 3; i++) {
        ans[i] = vec[X] * temp[0][i] + vec[Y] * temp[1][i] +
                 vec[Z] * temp[2][i] + mat[3][i];
    }

    res[X] = ans[0];
    res[Y] = ans[1];
    res[Z] = ans[2];
}

/**********************************************************************************************/

void glCreateScalingMatrix4f(GLmatrix4f(res), const GLfloat *vec) {
    glMatrix4fIdentity(res);
    res[0][0] = vec[X];
    res[1][1] = vec[Y];
    res[2][2] = vec[Z];
}

void glCreateScalingMatrix4d(GLmatrix4d(res), const GLdouble *vec) {
    glMatrix4dIdentity(res);
    res[0][0] = vec[X];
    res[1][1] = vec[Y];
    res[2][2] = vec[Z];
}

/**********************************************************************************************/

void glCreateTranslationMatrix4f(GLmatrix4f(res), const GLfloat *vec) {
    glMatrix4fIdentity(res);
    res[0][3] = vec[X];
    res[1][3] = vec[Y];
    res[2][3] = vec[Z];
}

void glCreateTranslationMatrix4d(GLmatrix4d(res), const GLdouble *vec) {
    glMatrix4dIdentity(res);
    res[0][3] = vec[X];
    res[1][3] = vec[Y];
    res[2][3] = vec[Z];
}

/**********************************************************************************************/

void glCreateRotationMatrix4f(GLmatrix4f(res), const GLfloat *vec) {
    GLfloat cosx, cosy, cosz, sinx, siny, sinz;
    GLmatrix4f(temp);

    glMatrix4fIdentity(res);

    cosx = cos(vec[X]);
    sinx = sin(vec[X]);
    cosy = cos(vec[Y]);
    siny = sin(vec[Y]);
    cosz = cos(vec[Z]);
    sinz = sin(vec[Z]);

    res[1][1] = cosx;
    res[2][2] = cosx;
    res[1][2] = sinx;
    res[2][1] = 0.0 - sinx;

    glMatrix4fIdentity(&temp);

    temp[0][0] = cosy;
    temp[2][2] = cosy;
    temp[0][2] = 0.0 - siny;
    temp[2][0] = siny;

    glMatrix4fTimes(res, res, &temp);

    glMatrix4fIdentity(&temp);

    temp[0][0] = cosz;
    temp[1][1] = cosz;
    temp[0][1] = sinz;
    temp[1][0] = 0.0 - sinz;

    glMatrix4fTimes(res, res, &temp);
}

void glCreateRotationMatrix4d(GLmatrix4d(res), const GLdouble *vec) {
    GLdouble cosx, cosy, cosz, sinx, siny, sinz;
    GLmatrix4d(temp);

    glMatrix4dIdentity(res);

    cosx = cos(vec[X]);
    sinx = sin(vec[X]);
    cosy = cos(vec[Y]);
    siny = sin(vec[Y]);
    cosz = cos(vec[Z]);
    sinz = sin(vec[Z]);

    res[1][1] = cosx;
    res[2][2] = cosx;
    res[1][2] = sinx;
    res[2][1] = 0.0 - sinx;

    glMatrix4dIdentity(&temp);

    temp[0][0] = cosy;
    temp[2][2] = cosy;
    temp[0][2] = 0.0 - siny;
    temp[2][0] = siny;

    glMatrix4dTimes(res, &temp, res);

    glMatrix4dIdentity(&temp);

    temp[0][0] = cosz;
    temp[1][1] = cosz;
    temp[0][1] = sinz;
    temp[1][0] = 0.0 - sinz;

    glMatrix4dTimes(res, &temp, res);
}

/**********************************************************************************************/

void glMatrix4dRotationLine(GLdouble res[4][4], GLdouble angle, GLdouble p1[3],
                            GLdouble p2[3]) {
    GLdouble tmp[4][4];
    GLdouble v1[3];
    GLdouble v2[3];
    GLdouble cosa, sina;

    glVector3dSub(v1, p2, p1);

    glVector3dNormalize(v1);

    if (v1[X] == 0.0 && v1[Y] == 0.0 && v1[Z] == 0.0)
        return;

    cosa = cos(angle);
    sina = sin(angle);

    glMatrix4dIdentity((GLdouble *)tmp);

    tmp[0][0] = (v1[X] * v1[X]) + (1.0 - (v1[X] * v1[X])) * cosa;
    tmp[1][1] = (v1[Y] * v1[Y]) + (1.0 - (v1[Y] * v1[Y])) * cosa;
    tmp[2][2] = (v1[Z] * v1[Z]) + (1.0 - (v1[Z] * v1[Z])) * cosa;
    tmp[1][0] = (v1[X] * v1[Y] * (1.0 - cosa)) + (v1[Z] * sina);
    tmp[2][0] = (v1[X] * v1[Z] * (1.0 - cosa)) - (v1[Y] * sina);
    tmp[0][1] = (v1[X] * v1[Y] * (1.0 - cosa)) - (v1[Z] * sina);
    tmp[2][1] = (v1[Y] * v1[Z] * (1.0 - cosa)) + (v1[X] * sina);
    tmp[0][2] = (v1[X] * v1[Z] * (1.0 - cosa)) + (v1[Y] * sina);
    tmp[1][2] = (v1[Y] * v1[Z] * (1.0 - cosa)) - (v1[X] * sina);

    v2[X] = p1[X] * -1.0;
    v2[Y] = p1[Y] * -1.0;
    v2[Z] = p1[Z] * -1.0;

    glMatrix4dShift(tmp, v2, GL_MATOP_PRECONCATENATE);
    glMatrix4dShift(tmp, p1, GL_MATOP_POSTCONCATENATE);

    glMatrix4dConcat(res, tmp, tmp, GL_MATOP_REPLACE);
}

/**********************************************************************************************/

void glMatrix4fInverse(GLmatrix4f(r), GLmatrix4f(m)) {
    GLfloat d00, d01, d02, d03;
    GLfloat d10, d11, d12, d13;
    GLfloat d20, d21, d22, d23;
    GLfloat d30, d31, d32, d33;
    GLfloat m00, m01, m02, m03;
    GLfloat m10, m11, m12, m13;
    GLfloat m20, m21, m22, m23;
    GLfloat m30, m31, m32, m33;
    GLfloat D;

    m00 = m[0][0];
    m01 = m[0][1];
    m02 = m[0][2];
    m03 = m[0][3];
    m10 = m[1][0];
    m11 = m[1][1];
    m12 = m[1][2];
    m13 = m[1][3];
    m20 = m[2][0];
    m21 = m[2][1];
    m22 = m[2][2];
    m23 = m[2][3];
    m30 = m[3][0];
    m31 = m[3][1];
    m32 = m[3][2];
    m33 = m[3][3];

    d00 = m11 * m22 * m33 + m12 * m23 * m31 + m13 * m21 * m32 -
          m31 * m22 * m13 - m32 * m23 * m11 - m33 * m21 * m12;
    d01 = m10 * m22 * m33 + m12 * m23 * m30 + m13 * m20 * m32 -
          m30 * m22 * m13 - m32 * m23 * m10 - m33 * m20 * m12;
    d02 = m10 * m21 * m33 + m11 * m23 * m30 + m13 * m20 * m31 -
          m30 * m21 * m13 - m31 * m23 * m10 - m33 * m20 * m11;
    d03 = m10 * m21 * m32 + m11 * m22 * m30 + m12 * m20 * m31 -
          m30 * m21 * m12 - m31 * m22 * m10 - m32 * m20 * m11;

    d10 = m01 * m22 * m33 + m02 * m23 * m31 + m03 * m21 * m32 -
          m31 * m22 * m03 - m32 * m23 * m01 - m33 * m21 * m02;
    d11 = m00 * m22 * m33 + m02 * m23 * m30 + m03 * m20 * m32 -
          m30 * m22 * m03 - m32 * m23 * m00 - m33 * m20 * m02;
    d12 = m00 * m21 * m33 + m01 * m23 * m30 + m03 * m20 * m31 -
          m30 * m21 * m03 - m31 * m23 * m00 - m33 * m20 * m01;
    d13 = m00 * m21 * m32 + m01 * m22 * m30 + m02 * m20 * m31 -
          m30 * m21 * m02 - m31 * m22 * m00 - m32 * m20 * m01;

    d20 = m01 * m12 * m33 + m02 * m13 * m31 + m03 * m11 * m32 -
          m31 * m12 * m03 - m32 * m13 * m01 - m33 * m11 * m02;
    d21 = m00 * m12 * m33 + m02 * m13 * m30 + m03 * m10 * m32 -
          m30 * m12 * m03 - m32 * m13 * m00 - m33 * m10 * m02;
    d22 = m00 * m11 * m33 + m01 * m13 * m30 + m03 * m10 * m31 -
          m30 * m11 * m03 - m31 * m13 * m00 - m33 * m10 * m01;
    d23 = m00 * m11 * m32 + m01 * m12 * m30 + m02 * m10 * m31 -
          m30 * m11 * m02 - m31 * m12 * m00 - m32 * m10 * m01;

    d30 = m01 * m12 * m23 + m02 * m13 * m21 + m03 * m11 * m22 -
          m21 * m12 * m03 - m22 * m13 * m01 - m23 * m11 * m02;
    d31 = m00 * m12 * m23 + m02 * m13 * m20 + m03 * m10 * m22 -
          m20 * m12 * m03 - m22 * m13 * m00 - m23 * m10 * m02;
    d32 = m00 * m11 * m23 + m01 * m13 * m20 + m03 * m10 * m21 -
          m20 * m11 * m03 - m21 * m13 * m00 - m23 * m10 * m01;
    d33 = m00 * m11 * m22 + m01 * m12 * m20 + m02 * m10 * m21 -
          m20 * m11 * m02 - m21 * m12 * m00 - m22 * m10 * m01;

    D = m00 * d00 - m01 * d01 + m02 * d02 - m03 * d03;

    if (D != 0.0) {
        r[0][0] = d00 / D;
        r[0][1] = -d10 / D;
        r[0][2] = d20 / D;
        r[0][3] = -d30 / D;
        r[1][0] = -d01 / D;
        r[1][1] = d11 / D;
        r[1][2] = -d21 / D;
        r[1][3] = d31 / D;
        r[2][0] = d02 / D;
        r[2][1] = -d12 / D;
        r[2][2] = d22 / D;
        r[2][3] = -d32 / D;
        r[3][0] = -d03 / D;
        r[3][1] = d13 / D;
        r[3][2] = -d23 / D;
        r[3][3] = d33 / D;
    }
}

void glMatrix4dInverse(GLmatrix4d(r), GLmatrix4d(m)) {
    GLdouble d00, d01, d02, d03;
    GLdouble d10, d11, d12, d13;
    GLdouble d20, d21, d22, d23;
    GLdouble d30, d31, d32, d33;
    GLdouble m00, m01, m02, m03;
    GLdouble m10, m11, m12, m13;
    GLdouble m20, m21, m22, m23;
    GLdouble m30, m31, m32, m33;
    GLdouble D;

    m00 = m[0][0];
    m01 = m[0][1];
    m02 = m[0][2];
    m03 = m[0][3];
    m10 = m[1][0];
    m11 = m[1][1];
    m12 = m[1][2];
    m13 = m[1][3];
    m20 = m[2][0];
    m21 = m[2][1];
    m22 = m[2][2];
    m23 = m[2][3];
    m30 = m[3][0];
    m31 = m[3][1];
    m32 = m[3][2];
    m33 = m[3][3];

    d00 = m11 * m22 * m33 + m12 * m23 * m31 + m13 * m21 * m32 -
          m31 * m22 * m13 - m32 * m23 * m11 - m33 * m21 * m12;
    d01 = m10 * m22 * m33 + m12 * m23 * m30 + m13 * m20 * m32 -
          m30 * m22 * m13 - m32 * m23 * m10 - m33 * m20 * m12;
    d02 = m10 * m21 * m33 + m11 * m23 * m30 + m13 * m20 * m31 -
          m30 * m21 * m13 - m31 * m23 * m10 - m33 * m20 * m11;
    d03 = m10 * m21 * m32 + m11 * m22 * m30 + m12 * m20 * m31 -
          m30 * m21 * m12 - m31 * m22 * m10 - m32 * m20 * m11;

    d10 = m01 * m22 * m33 + m02 * m23 * m31 + m03 * m21 * m32 -
          m31 * m22 * m03 - m32 * m23 * m01 - m33 * m21 * m02;
    d11 = m00 * m22 * m33 + m02 * m23 * m30 + m03 * m20 * m32 -
          m30 * m22 * m03 - m32 * m23 * m00 - m33 * m20 * m02;
    d12 = m00 * m21 * m33 + m01 * m23 * m30 + m03 * m20 * m31 -
          m30 * m21 * m03 - m31 * m23 * m00 - m33 * m20 * m01;
    d13 = m00 * m21 * m32 + m01 * m22 * m30 + m02 * m20 * m31 -
          m30 * m21 * m02 - m31 * m22 * m00 - m32 * m20 * m01;

    d20 = m01 * m12 * m33 + m02 * m13 * m31 + m03 * m11 * m32 -
          m31 * m12 * m03 - m32 * m13 * m01 - m33 * m11 * m02;
    d21 = m00 * m12 * m33 + m02 * m13 * m30 + m03 * m10 * m32 -
          m30 * m12 * m03 - m32 * m13 * m00 - m33 * m10 * m02;
    d22 = m00 * m11 * m33 + m01 * m13 * m30 + m03 * m10 * m31 -
          m30 * m11 * m03 - m31 * m13 * m00 - m33 * m10 * m01;
    d23 = m00 * m11 * m32 + m01 * m12 * m30 + m02 * m10 * m31 -
          m30 * m11 * m02 - m31 * m12 * m00 - m32 * m10 * m01;

    d30 = m01 * m12 * m23 + m02 * m13 * m21 + m03 * m11 * m22 -
          m21 * m12 * m03 - m22 * m13 * m01 - m23 * m11 * m02;
    d31 = m00 * m12 * m23 + m02 * m13 * m20 + m03 * m10 * m22 -
          m20 * m12 * m03 - m22 * m13 * m00 - m23 * m10 * m02;
    d32 = m00 * m11 * m23 + m01 * m13 * m20 + m03 * m10 * m21 -
          m20 * m11 * m03 - m21 * m13 * m00 - m23 * m10 * m01;
    d33 = m00 * m11 * m22 + m01 * m12 * m20 + m02 * m10 * m21 -
          m20 * m11 * m02 - m21 * m12 * m00 - m22 * m10 * m01;

    D = m00 * d00 - m01 * d01 + m02 * d02 - m03 * d03;

    if (D != 0.0) {
        r[0][0] = d00 / D;
        r[0][1] = -d10 / D;
        r[0][2] = d20 / D;
        r[0][3] = -d30 / D;
        r[1][0] = -d01 / D;
        r[1][1] = d11 / D;
        r[1][2] = -d21 / D;
        r[1][3] = d31 / D;
        r[2][0] = d02 / D;
        r[2][1] = -d12 / D;
        r[2][2] = d22 / D;
        r[2][3] = -d32 / D;
        r[3][0] = -d03 / D;
        r[3][1] = d13 / D;
        r[3][2] = -d23 / D;
        r[3][3] = d33 / D;
    }
}
