
/*----------------------------------------------------------------------------

 Module name: transformations utility package.
 
 Authors: W.T. Hewitt, K.M. Singleton.
 
 Function: contains functions for performing two-dimensional and 
 three-dimensional transformations. 

 Dependencies: machine.h, transformtypes.h.

 Internal function list: validbounds.

 External function list: ptk_equal, ptk_point, ptk_point3, ptk_vector,
 ptk_vector3, ptk_vec3topt3, ptk_pt3tovec3, ptk_limit, ptk_limit3,
 ptk_dotv, ptk_crossv,
 ptk_nullv, ptk_modv, ptk_unitv, ptk_scalev, ptk_subv, ptk_addv,
 ptk_unitmatrix3, ptk_transposematrix3, ptk_multiplymatrix3, 
 ptk_concatenatematrix3,
 ptk_shift3, ptk_scale3, ptk_rotatecs3, ptk_rotate3, ptk_shear3,
 ptk_rotatevv3, ptk_rotateline3, ptk_pt3topt4, ptk_pt4topt3, ptk_transform4,
 ptk_transform3, ptk_matrixtomatrix3, ptk_outputmatrix3, ptk_box3tobox3, 
 ptk_accumulatetran3, ptk_evalvieworientation3,
 ptk_evalviewmapping3, ptk_stackmatrix3, ptk_unstackmatrix3, 
 ptk_examinestackmatrix3,
 ptk_3ptto3pt, ptk_0to3pt, ptk_oto3pt, ptk_invertmatrix3.

 Hashtables used: none.

 Modification history: (Version), (Date), (Name), (Description).

 1.0,  2nd February 1986,  W.T. Hewitt,  First version.

 1.1,  1st April 1986,  K.M. Singleton, W.T. Hewitt, Added viewing 
 and other utility routines.

 1.2,  8th June 1987,  K.M. Wyrwas,  Added routine to invert a matrix.
 
 1.3,  8th July 1988,  S.Larkin,  Modified to work with VAX PHIGS$.

 1.4,  30th October 1990,  W.T. Hewitt,   Fixed to work with VAX PHIGS$.

 2.0,  4th January 1991,  G. Williams,  Converted from Pascal to C.

 2.1,  13th February 1991, G. Williams, Added functions - ptk_vector3,
 ptk_vec3topt3 and ptk_pt3tovec3.

 2.2, 3rd May 1991, G. Williams, Added function - ptk_3x3to4x4.

----------------------------------------------------------------------------*/

#include <stdio.h>
#include <math.h>

#define SUN

#ifdef SUN
#include <malloc.h>
#include <memory.h>
#endif

#include "machine.h"
#include "ptktype.h"
#include "trantype.h"

/* Pointer records */

typedef struct ptksstack {
  Pmatrix3 ptkamat;
  struct ptksstack *ptkpnext;
} ptksstack;

#define ptkcdtor         (3.14159263 / 180.0)

ptksstack *listhead = NULL, *newone = NULL;

/*--------------------------------------------------------------------------*/

/*function:external*/
extern boolean ptk_equal(C(Pfloat) one, C(Pfloat) two)
PreANSI(Pfloat one)
PreANSI(Pfloat two)
/*
** \parambegin
** \param{Pfloat}{one}{floating point number}{IN}
** \param{Pfloat}{two}{floating point number}{IN}
** \paramend
** \blurb{This function returns TRUE if \pardesc{one} and \pardesc{two}
** are equal, or their difference is less than the global 
** constant tolerance \pardesc{ptkcpceps}.}
*/
{
  return (fabs(one - two) <= ptkcpceps);
}  /* ptk_equal */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Ppoint ptk_point(C(Pfloat) x, C(Pfloat) y)
PreANSI(Pfloat x)
PreANSI(Pfloat y)
/*
** \parambegin
** \param{Pfloat}{x}{x coordinate}{IN}
** \param{Pfloat}{y}{y coordinate}{IN}
** \paramend
** \blurb{This function returns a structure representing a 2D point.}
*/
{
  Ppoint temp;

  temp.x = x;
  temp.y = y;
  return temp;
}  /* ptk_point */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Ppoint3 ptk_point3(C(Pfloat) x, C(Pfloat) y, C(Pfloat) z)
PreANSI(Pfloat x)
PreANSI(Pfloat y)
PreANSI(Pfloat z)
/*
** \parambegin
** \param{Pfloat}{x}{x coordinate}{IN}
** \param{Pfloat}{y}{y coordinate}{IN}
** \param{Pfloat}{z}{z coordinate}{IN}
** \paramend
** \blurb{This function returns a structure representing a 3D point.}
*/
{
  Ppoint3 temp;

  temp.x = x;
  temp.y = y;
  temp.z = z;
  return temp;
}  /* ptk_point3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Pvector ptk_vector(C(Pfloat) x, C(Pfloat) y)
PreANSI(Pfloat x)
PreANSI(Pfloat y)
/*
** \parambegin
** \param{Pfloat}{x}{x coordinate}{IN}
** \param{Pfloat}{y}{y coordinate}{IN}
** \paramend
** \blurb{This function returns a structure representing a 2D vector.}
*/
{
  Pvector temp;

  temp.x = x;
  temp.y = y;
  return temp;
}  /* ptk_vector */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Pvector3 ptk_vector3(C(Pfloat) x, C(Pfloat) y, C(Pfloat) z)
PreANSI(Pfloat x)
PreANSI(Pfloat y)
PreANSI(Pfloat z)
/*
** \parambegin
** \param{Pfloat}{x}{x coordinate}{IN}
** \param{Pfloat}{y}{y coordinate}{IN}
** \param{Pfloat}{z}{z coordinate}{IN}
** \paramend
** \blurb{This function returns a structure representing a 3D vector.}
*/
{
  Pvector3 temp;

  temp.x = x;
  temp.y = y;
  temp.z = z;
  return temp;
}  /* ptk_vector3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Plimit ptk_limit(C(Pfloat) xmin, C(Pfloat) xmax, C(Pfloat) ymin,
                        C(Pfloat) ymax)
PreANSI(Pfloat xmin)
PreANSI(Pfloat xmax)
PreANSI(Pfloat ymin)
PreANSI(Pfloat ymax)
/*
** \parambegin
** \param{Pfloat}{xmin}{minimum x coordinate}{IN}
** \param{Pfloat}{xmax}{maximum x coordinate}{IN}
** \param{Pfloat}{ymin}{minimum y coordinate}{IN}
** \param{Pfloat}{ymax}{maximum y coordinate}{IN}
** \paramend
** \blurb{This function returns a structure representing a 2D area.}
*/
{
  Plimit temp;

  temp.xmin = xmin;
  temp.xmax = xmax;
  temp.ymin = ymin;
  temp.ymax = ymax;
  return temp;
}  /* ptk_limit */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Plimit3 ptk_limit3(C(Pfloat) xmin, C(Pfloat) xmax, C(Pfloat) ymin,
                          C(Pfloat) ymax, C(Pfloat) zmin, C(Pfloat) zmax)
PreANSI(Pfloat xmin)
PreANSI(Pfloat xmax)
PreANSI(Pfloat ymin)
PreANSI(Pfloat ymax)
PreANSI(Pfloat zmin)
PreANSI(Pfloat zmax)
/*
** \parambegin
** \param{Pfloat}{xmin}{minimum x coordinate}{IN}
** \param{Pfloat}{xmax}{maximum x coordinate}{IN}
** \param{Pfloat}{ymin}{minimum y coordinate}{IN}
** \param{Pfloat}{ymax}{maximum y coordinate}{IN}
** \param{Pfloat}{zmin}{minimum z coordinate}{IN}
** \param{Pfloat}{zmax}{maximum z coordinate}{IN}
** \paramend
** \blurb{This function returns a structure representing a 3D volume.}
*/
{
  Plimit3 temp;

  temp.xmin = xmin;
  temp.xmax = xmax;
  temp.ymin = ymin;
  temp.ymax = ymax;
  temp.zmin = zmin;
  temp.zmax = zmax;
  return temp;
}  /* ptk_limit3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Ppoint3 ptk_vec3topt3(C(Pvector3 *) vec)
PreANSI(Pvector3 *vec)
/*
** \parambegin
** \param{Pvector3 *}{vec}{3D vector}{IN}
** \paramend
** \blurb{Convert 3D vector to a 3D point.}
*/
{
  Ppoint3 temp;

  temp = ptk_point3(vec->x, vec->y, vec->z);
  return temp;
}  /* ptk_vec3topt3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Pvector3 ptk_pt3tovec3(C(Ppoint3 *) pt)
PreANSI(Ppoint3 *pt)
/*
** \parambegin
** \param{Ppoint3 *}{pt}{3D point}{IN}
** \paramend
** \blurb{Convert 3D point to a 3D vector.}
*/
{
  Pvector3 temp;

  temp = ptk_vector3(pt->x, pt->y, pt->z);
  return temp;
}  /* ptk_pt3tovec3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Pfloat ptk_dotv(C(Pvector3 *) v1, C(Pvector3 *) v2)
PreANSI(Pvector3 *v1)
PreANSI(Pvector3 *v2)
/*
** \parambegin
** \param{Pvector3 *}{v1}{3D vector}{IN}
** \param{Pvector3 *}{v2}{3D vector}{IN}
** \paramend
** \blurb{Evaluates the dot product of the two vectors \pardesc{v1} and
** \pardesc{v2}, returning it as the value of the function.}
*/
{
  return (v1->x * v2->x + v1->y * v2->y + v1->z * v2->z);
}  /* ptk_dotv */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Pvector3 ptk_crossv(C(Pvector3 *) v1, C(Pvector3 *) v2)
PreANSI(Pvector3 *v1)
PreANSI(Pvector3 *v2)
/*
** \parambegin
** \param{Pvector3 *}{v1}{3D vector}{IN}
** \param{Pvector3 *}{v2}{3D vector}{IN}
** \paramend
** \blurb{Evaluates the cross product of the two vectors \pardesc{v1} and
** \pardesc{v2}, returning the new vector as
** the function result. Since a local copy is made statements such as
**        v2 := ptk\_crossv(v1, v2)
** will produce the correct answer.}
*/
{
  Pvector3 temp;

  temp.x = v1->y * v2->z - v1->z * v2->y;
  temp.y = v1->z * v2->x - v1->x * v2->z;
  temp.z = v1->x * v2->y - v1->y * v2->x;
  return temp;
}  /* ptk_crossv */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern boolean ptk_nullv(C(Pvector3 *) vec)
PreANSI(Pvector3 *vec)
/*
** \parambegin
** \param{Pvector3 *}{vec}{3D vector}{IN}
** \paramend
** \blurb{Returns \pardesc{TRUE} if the modulus of the vector \pardesc{vec} 
** is less than the global tolerance \pardesc{ptkpceps}, otherwise 
** \pardesc{FALSE}.}
*/
{
  return (ptk_equal(vec->x, 0.0) && ptk_equal(vec->y, 0.0) &&
	  ptk_equal(vec->z, 0.0));
}  /* ptk_nullv */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Pfloat ptk_modv(C(Pvector3 *) vec)
PreANSI(Pvector3 *vec)
/*
** \parambegin
** \param{Pvector3 *}{vec}{3D vector}{IN}
** \paramend
** \blurb{Returns the modulus of the vector \pardesc{vec}.}
*/
{
  return sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
}  /* ptk_modv */

/*-------------------------------------------------------------------------*/

/*function:external*/
extern Pvector3 ptk_unitv(C(Pvector3 *) vec)
PreANSI(Pvector3 *vec)
/*
** \parambegin
** \param{Pvector3 *}{vec}{3D vector}{IN}
** \paramend
** \blurb{Generates and returns a unit vector from the supplied vector 
** \pardesc{vec}.}
*/
{
  Pfloat modu;
  Pvector3 temp;

  modu = ptk_modv(vec);
  if (!ptk_equal(modu, 0.0)) 
  {
    temp = ptk_vector3(vec->x / modu, vec->y / modu, vec->z / modu);
    return temp;
  } 
  else 
  {
    temp = ptk_vector3(0.0, 0.0, 0.0);
    return temp;
  }
}  /* ptk_unitv */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Pvector3 ptk_scalev(C(Pvector3 *) vec, C(Pfloat) scale)
PreANSI(Pvector3 *vec)
PreANSI(Pfloat scale)
/*
** \parambegin
** \param{Pvector3 *}{vec}{3D vector}{IN}
** \param{Pfloat}{scale}{scale factor}{IN}
** \paramend
** \blurb{Multiplies the  vector \pardesc{v} by the  scalar \pardesc{s} and
** returns the result.}
*/
{
  Pvector3 temp;

  temp = ptk_vector3(vec->x * scale, vec->y * scale, vec->z * scale);
  return temp;
}  /* ptk_scalev */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Pvector3 ptk_subv(C(Pvector3 *) p1, C(Pvector3 *) p2)
PreANSI(Pvector3 *p1)
PreANSI(Pvector3 *p2)
/*
** \parambegin
** \param{Pvector3 *}{p1}{3D vector}{IN}
** \param{Pvector3 *}{p2}{3D vector}{IN}
** \paramend
** \blurb{Evaluates the vector \pardesc {v1-v2}.}
*/
{
  Pvector3 v;

  v = ptk_vector3(p1->x - p2->x, p1->y - p2->y, p1->z - p2->z);
  return v;
}  /* ptk_subv */

/*----------------------------------------------------------------------------*/

/*function:external*/
extern Pvector3 ptk_addv(C(Pvector3 *) p1, C(Pvector3 *) p2)
PreANSI(Pvector3 *p1)
PreANSI(Pvector3 *p2)
/*
** \parambegin
** \param{Pvector3 *}{p1}{3D vector}{IN}
** \param{Pvector3 *}{p2}{3D vector}{IN}
** \paramend
** \blurb{Add the two vectors \pardesc{v1} and \pardesc{v2}.}
*/
{
  Pvector3 v;

  v = ptk_vector3(p1->x + p2->x, p1->y + p2->y, p1->z + p2->z);
  return v;
}  /* ptk_addv */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern boolean ptk_equalpoint(C(Ppoint *) pt1, C(Ppoint *) pt2)
PreANSI(Ppoint *pt1)
PreANSI(Ppoint *pt2)
/*
** \parambegin
** \param{Ppoint *}{pt1}{2D point}{IN}
** \param{Ppoint *}{pt2}{2D point}{IN}
** \paramend
** \blurb{This function returns TRUE if \pardesc{pt1} and \pardesc{pt2}
** are equal, otherwise FALSE.}
*/
{
  return ((pt1->x == pt2->x) && (pt1->y == pt2->y));
}  /* ptk_equalpoint */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern boolean ptk_equalpoint3(C(Ppoint3 *) pt1, C(Ppoint3 *) pt2)
PreANSI(Ppoint3 *pt1)
PreANSI(Ppoint3 *pt2)
/*
** \parambegin
** \param{Ppoint3 *}{pt1}{3D point}{IN}
** \param{Ppoint3 *}{pt2}{3D point}{IN}
** \paramend
** \blurb{This function returns TRUE if \pardesc{pt1} and \pardesc{pt2}
** are equal, otherwise FALSE.}
*/
{
  return ((pt1->x == pt2->x) && (pt1->y == pt2->y) && (pt1->z == pt2->z));
}  /* ptk_equalpoint3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern boolean ptk_equalvector(C(Pvector *) vec1, C(Pvector *) vec2)
PreANSI(Pvector *vec1)
PreANSI(Pvector *vec2)
/*
** \parambegin
** \param{Pvector *}{vec1}{2D vector}{IN}
** \param{Pvector *}{vec2}{2D vector}{IN}
** \paramend
** \blurb{This function returns TRUE if \pardesc{vec1} and \pardesc{vec2}
** are equal, otherwise FALSE.}
*/
{
  return ((vec1->x == vec2->x) && (vec1->y == vec2->y));
}  /* ptk_equalvector */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern boolean ptk_equalvector3(C(Pvector3 *) vec1, C(Pvector3 *) vec2)
PreANSI(Pvector3 *vec1)
PreANSI(Pvector3 *vec2)
/*
** \parambegin
** \param{Pvector3 *}{vec1}{3D vector}{IN}
** \param{Pvector3 *}{vec2}{3D vector}{IN}
** \paramend
** \blurb{This function returns TRUE if \pardesc{vec1} and \pardesc{vec2}
** are equal, otherwise FALSE.}
*/
{
  return ((vec1->x == vec2->x) && (vec1->y == vec2->y) && 
          (vec1->z == vec2->z));
}  /* ptk_equalvector3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_unitmatrix(C(Pmatrix) matrix)
PreANSI(Pmatrix matrix)
/*
** \parambegin
** \param{Pmatrix}{matrix}{3x3 matrix}{IN}
** \paramend
** \blurb{This procedure creates a unit matrix, and stores it in 
** \pardesc{matrix}.}
*/
{
  /*  */
  Pint jj, ii;

  for (ii = 0; ii <= 2; ii++) 
  {
    for (jj = 0; jj <= 2; jj++)
      matrix[ii][jj] = 0.0;
    matrix[ii][ii] = 1.0;
  }
}  /* ptk_unitmatrix */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_unitmatrix3(C(Pmatrix3) matrix)
PreANSI(Pmatrix3 matrix)
/*
** \parambegin
** \param{Pmatrix3}{matrix}{4x4 matrix}{IN}
** \paramend
** \blurb{This procedure creates a unit matrix, and stores it in 
** \pardesc{matrix}.}
*/
{
  /*  */
  Pint jj, ii;

  for (ii = 0; ii <= 3; ii++) 
  {
    for (jj = 0; jj <= 3; jj++)
      matrix[ii][jj] = 0.0;
    matrix[ii][ii] = 1.0;
  }
}  /* ptk_unitmatrix3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern boolean ptk_equalmatrix(C(Pmatrix) mat1, C(Pmatrix) mat2)
PreANSI(Pmatrix mat1)
PreANSI(Pmatrix mat2)
/*
** \parambegin
** \param{Pmatrix}{mat1}{3x3 matrix}{IN}
** \param{Pmatrix}{mat2}{3x3 matrix}{IN}
** \paramend
** \blurb{This function returns true if \pardesc{mat1} and \pardesc{mat2}
** are equal.}
*/
{
  boolean result;
  Pint i, j;

  for (i = 0; i <= 2; i++)
    for (j = 0; j <= 2; j++)
      if (mat1[i][j] != mat2[i][j])
        return FALSE;
  return TRUE;
}  /* ptk_equalmatrix */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern boolean ptk_equalmatrix3(C(Pmatrix3) mat1, C(Pmatrix3) mat2)
PreANSI(Pmatrix3 mat1)
PreANSI(Pmatrix3 mat2)
/*
** \parambegin
** \param{Pmatrix3}{mat1}{4x4 matrix}{IN}
** \param{Pmatrix3}{mat2}{4x4 matrix}{IN}
** \paramend
** \blurb{This function returns true if \pardesc{mat1} and \pardesc{mat2}
** are equal.}
*/
{
  boolean result;
  Pint i, j;

  for (i = 0; i <= 3; i++)
    for (j = 0; j <= 3; j++)
      if (mat1[i][j] != mat2[i][j])
        return FALSE;
  return TRUE;
}  /* ptk_equalmatrix3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_transposematrix3(C(Pmatrix3) matrix, C(Pmatrix3) result)
PreANSI(Pmatrix3 matrix)
PreANSI(Pmatrix3 result)
/*
** \parambegin
** \param{Pmatrix3}{matrix}{4x4 matrix}{IN}
** \param{Pmatrix3}{result}{4x4 matrix}{OUT}
** \paramend
** \blurb{Makes \pardesc{matrix2} the transpose of \pardesc{matrix1}.
** Note \pardesc{matrix2} can be \pardesc{matrix1} since a copy is made 
** first.}
*/
{
  Pmatrix3 temp;
  Pint i, j;

  memcpy(temp, matrix, sizeof(Pmatrix3));
  for (i = 0; i <= 3; i++) 
  {
    for (j = 0; j <= 3; j++)
      result[i][j] = temp[j][i];
  }
}  /* ptk_transposematrix3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_multiplymatrix3(C(Pmatrix3) matrix1, C(Pmatrix3) matrix2, 
                          C(Pmatrix3) result)
PreANSI(Pmatrix3 matrix1)
PreANSI(Pmatrix3 matrix2)
PreANSI(Pmatrix3 result)
/*
** \parambegin
** \param{Pmatrix3}{matrix1}{4x4 matrix}{IN}
** \param{Pmatrix3}{matrix2}{4x4 matrix}{IN}
** \param{Pmatrix3}{result}{4x4 matrix}{OUT}
** \paramend
** \blurb{Makes \pardesc{result} the product of \pardesc{matrix1} and
** \pardesc{matrix2}, with {\tt result := matrix1 * matrix2}.
** Note \pardesc{result} can also be \pardesc{matrix1} or \pardesc{matrix2}
** since a copy is made.}
*/
{
  Pint ii, jj;
  Pmatrix3 sum;
  Pfloat temp;
  Pfloat d[4], e[4];

  for (ii = 0; ii <= 3; ii++) 
  {
    d[ii] = (matrix1[ii][1] * matrix1[ii][0]) + 
            (matrix1[ii][3] * matrix1[ii][2]);
    e[ii] = (matrix2[0][ii] * matrix2[1][ii]) + 
            (matrix2[2][ii] * matrix2[3][ii]);
  }

  for (ii = 0; ii <= 3; ii++) 
  {
    for (jj = 0; jj <= 3; jj++) 
    {
      temp = (matrix1[ii][1] + matrix2[0][jj]) * 
             (matrix1[ii][0] + matrix2[1][jj]);
      temp += (matrix1[ii][3] + matrix2[2][jj]) * 
              (matrix1[ii][2] + matrix2[3][jj]) - d[ii] - e[jj];
      sum[ii][jj] = temp;
    }
  }
  /* copy matrix back in case result is one of the other two */
  memcpy(result, sum, sizeof(Pmatrix3));
}  /* ptk_multiplymatrix3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_concatenatematrix3(C(Pcomptype) operation, 
            C(Pmatrix3) matrix1, C(Pmatrix3) matrix2, C(Pmatrix3) result)
PreANSI(Pcomptype operation)
PreANSI(Pmatrix3 matrix1)
PreANSI(Pmatrix3 matrix2)
PreANSI(Pmatrix3 result)
/*
** \parambegin
** \param{Pcomptype}{operation}{concatenation operation}{IN}
** \param{Pmatrix3}{matrix1}{4x4 matrix}{IN}
** \param{Pmatrix3}{matrix2}{4x4 matrix}{IN}
** \param{Pmatrix3}{result}{4x4 matrix}{OUT}
** \paramend
** \blurb{Concatenates \pardesc{matrix1}  and \pardesc{matrix2}
** on the basis of \pardesc{operation}.
** The result is stored in \pardesc{result}.
** Note \pardesc{result} can also be \pardesc{matrix1} or \pardesc{matrix2}
** since a copy is made. When \pardesc{operation} $=$ 
** \pardesc{PVPreconcatenate}, then \pardesc{result := matrix2 * matrix1}.
** When \pardesc{operation} $=$ \pardesc{PVPostConcatenate}, 
** \pardesc{result := matrix1 * matrix2}.}
*/
{
  switch (operation) 
  {
  case PPRECONCATENATE:  
    ptk_multiplymatrix3(matrix2, matrix1, result);
    break;

  case PPOSTCONCATENATE:  
    ptk_multiplymatrix3(matrix1, matrix2, result);
    break;

  case PREPLACE:  
    memcpy(result, matrix1, sizeof(Pmatrix3));
    break;
  }
}  /* ptk_concatenatematrix3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_shift3(C(Pvector3 *) shift, C(Pcomptype) operation, 
                       C(Pmatrix3) matrix)
PreANSI(Pvector3 *shift)
PreANSI(Pcomptype operation)
PreANSI(Pmatrix3 matrix)
/*
** \parambegin
** \param{Pvector3 *}{shift}{shift factor}{IN}
** \param{Pcomptype}{operation}{concatenation operation}{IN}
** \param{Pmatrix3}{matrix}{4x4 matrix}{OUT}
** \paramend
** \blurb{Computes a matrix to perform the given shift and concatenates 
** this with \pardesc{matrix} on the basis of \pardesc{operation}.}
*/
{
  Pmatrix3 temp;

  ptk_unitmatrix3(temp);
  temp[0][3] = shift->x;
  temp[1][3] = shift->y;
  temp[2][3] = shift->z;
  ptk_concatenatematrix3(operation, temp, matrix, matrix);
}  /* ptk_shift3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_scale3(C(Pvector3 *) scale, C(Pcomptype) operation, 
                       C(Pmatrix3) matrix)
PreANSI(Pvector3 *scale)
PreANSI(Pcomptype operation)
PreANSI(Pmatrix3 matrix)
/*
** \parambegin
** \param{Pvector3 *}{shift}{shift factor}{IN}
** \param{Pcomptype}{operation}{concatenation operation}{IN}
** \param{Pmatrix3}{matrix}{4x4 matrix}{OUT}
** \paramend
** \blurb{Computes a matrix to perform the given scale and concatenates 
** this with \pardesc{matrix} on the basis of \pardesc{operation}.}
*/
{
  Pmatrix3 temp;

  ptk_unitmatrix3(temp);
  temp[0][0] = scale->x;  
  temp[1][1] = scale->y;
  temp[2][2] = scale->z;
  ptk_concatenatematrix3(operation, temp, matrix, matrix);
}  /* ptk_scale3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_rotatecs3(C(Pfloat) costheta, C(Pfloat) sinetheta, 
                          C(ptkeaxistype) axis, C(Pcomptype) operation, 
                          C(Pmatrix3) matrix)
PreANSI(Pfloat costheta)
PreANSI(Pfloat sinetheta)
PreANSI(ptkeaxistype axis)
PreANSI(Pcomptype operation)
PreANSI(Pmatrix3 matrix)
/*
** \parambegin
** \param{Pfloat}{costheta}{cosine of angle}{IN}
** \param{Pfloat}{sinetheta}{sine of angle}{IN}
** \param{ptkeaxistype}{axis}{x, y or z axis}{IN}
** \param{Pcomptype}{operation}{concatenation operation}{IN}
** \param{Pmatrix3}{matrix}{4x4 matrix}{OUT}
** \paramend
** \blurb{Forms a matrix to perform the given rotation and concatenates 
** this with 
** \pardesc{matrix} on the basis of \pardesc{operation}.
** This form assumes  $\cos(theta)$ and $\sin(theta)$ are presented.
** No check is made to ensure that the sum of the squares is 1.}
*/
{
  Pmatrix3 temp;

  ptk_unitmatrix3(temp);
  switch (axis) 
  {
  case PTKEXAXIS:
    temp[2][2] = costheta;
    temp[1][1] = costheta;
    temp[1][2] = -sinetheta;
    temp[2][1] = sinetheta;
    break;

  case PTKEYAXIS:
    temp[0][0] = costheta;
    temp[2][2] = costheta;
    temp[0][2] = sinetheta;
    temp[2][0] = -sinetheta;
    break;

  case PTKEZAXIS:
    temp[0][0] = costheta;
    temp[1][1] = costheta;
    temp[0][1] = -sinetheta;
    temp[1][0] = sinetheta;
    break;
  }
  ptk_concatenatematrix3(operation, temp, matrix, matrix);
}  /* ptk_rotatecs3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_rotate3(C(Pfloat) rotation, C(ptkeaxistype) axis, 
                        C(Pcomptype) operation, C(Pmatrix3) matrix)
PreANSI(Pfloat rotation)
PreANSI(ptkeaxistype axis)
PreANSI(Pcomptype operation) 
PreANSI(Pmatrix3 matrix)
/*
** \parambegin
** \param{Pfloat}{rotation}{angle in degrees}{IN}
** \param{ptkeaxistype}{axis}{x, y or z axis}{IN}
** \param{Pcomptype}{operation}{concatenation operation}{IN}
** \param{Pmatrix3}{matrix}{4x4 matrix}{OUT}
** \paramend
** \blurb{Computes a matrix to perform the given rotation and concatenates 
** this with 
** \pardesc{matrix} on the basis of \pardesc{operation}.
** \pardesc{rotation} is expressed in degrees.}
*/
{
  Pfloat costheta;
  Pfloat sinetheta;

  costheta  = cos((rotation * ptkcdtor));
  sinetheta = sin((rotation * ptkcdtor));
  ptk_rotatecs3(costheta, sinetheta, axis, operation, matrix);
}  /* ptk_rotate3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_shear3(C(ptkeaxistype) shearaxis, C(ptkeaxistype) sheardir, 
                       C(Pfloat) shearfactor, C(Pcomptype) operation, 
                       C(Pmatrix3) matrix)
PreANSI(ptkeaxistype shearaxis)
PreANSI(ptkeaxistype sheardir)
PreANSI(Pfloat shearfactor)
PreANSI(Pcomptype operation)
PreANSI(Pmatrix3 matrix)
/*
** \parambegin
** \param{ptkeaxistype}{shearaxis}{x, y or z axis}{IN}
** \param{ptkeaxistype}{sheardir}{x, y or z direction}{IN}
** \param{Pfloat}{shearfactor}{shear factor}{IN}
** \param{Pcomptype}{operation}{concatenation operation}{IN}
** \param{Pmatrix3}{matrix}{4x4 matrix}{OUT}
** \paramend
** \blurb{Computes a matrix to perform the given shear and concatenates 
** this with 
** \pardesc{matrix} on the basis of \pardesc{operation}.
** The shear is specified by amount \pardesc{f} about axis \pardesc{i}
** in  direction \pardesc{j}.}
*/
{
  Pmatrix3 temp;

  ptk_unitmatrix3(temp);
  temp[shearaxis - 1][sheardir - 1] = shearfactor;
  ptk_concatenatematrix3(operation, temp, matrix, matrix);
}  /* ptk_shear3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_rotatevv3(C(Pvector3 *) v1, C(Pvector3 *) v2, 
                          C(Pcomptype) operation, 
                          C(Pmatrix3) matrix, C(Pint *)error)
PreANSI(Pvector3 *v1)
PreANSI(Pvector3 *v2)
PreANSI(Pcomptype operation) 
PreANSI(Pmatrix3 matrix)
PreANSI(Pint *error)
/*
** \parambegin
** \param{Pvector3 *}{v1}{3D vector}{IN}
** \param{Pvector3 *}{v2}{3D vector}{IN}
** \param{Pcomptype}{operation}{concatenation operation}{IN}
** \param{Pmatrix3}{matrix}{4x4 matrix}{OUT}
** \param{Pint *}{error}{error code}{OUT}
** \paramend
** \blurb{Computes a matrix to perform the rotation of \pardesc{v1} to 
** \pardesc{v2} and concatenates this with 
** \pardesc{matrix} on the basis of \pardesc{operation}.}
*/
{
  Pmatrix3 temp;
  Pvector3 dv1, dv2, nvec;
  Pfloat t1, costheta, sinetheta;

  /* Create a rotation matrix (about origin) to rotate from
  ** vector dv1 to vector dv2
  ** Cf ROGERS and ADAMS page 55-59
  */

  *error = ptkcpcok;   /* Assume OK */
  dv1 = ptk_unitv(v1);
  dv2 = ptk_unitv(v2);

  /* Get sine theta (cross product) and cos theta (dot product) */

  costheta = dv1.x * dv2.x + dv1.y * dv2.y + dv1.z * dv2.z;
  nvec = ptk_crossv(&dv1, &dv2);

  /* How do I choose sign of sinetheta?
  ** let A, B and K be unit vectors, then
  ** A X B :=  sin(theta) K
  ** The sign of sinetheta is not important
  ** because of the way it appears in the subsequent formulae
  */

  sinetheta = ptk_modv(&nvec);
  if (ptk_equal(fabs(sinetheta), 0.0)) 
  {
    *error = -1;
    sinetheta = ptkcpceps;
  }
  nvec = ptk_unitv(&nvec);
  ptk_unitmatrix3(temp);   /* get a unit matrix */
  t1 = 1.0 - costheta;
  temp[0][0] = (nvec.x * nvec.x) + (1.0 - (nvec.x * nvec.x)) * costheta;
  temp[1][1] = (nvec.y * nvec.y) + (1.0 - (nvec.y * nvec.y)) * costheta;
  temp[2][2] = (nvec.z * nvec.z) + (1.0 - (nvec.z * nvec.z)) * costheta;
  temp[1][0] = (nvec.x * nvec.y * t1) + (nvec.z * sinetheta);
  temp[2][0] = (nvec.x * nvec.z * t1) - (nvec.y * sinetheta); 
  temp[0][1] = (nvec.x * nvec.y * t1) - (nvec.z * sinetheta);
  temp[2][1] = (nvec.y * nvec.z * t1) + (nvec.x * sinetheta);
  temp[0][2] = (nvec.x * nvec.z * t1) + (nvec.y * sinetheta);
  temp[1][2] = (nvec.y * nvec.z * t1) - (nvec.x * sinetheta);
  ptk_concatenatematrix3(operation, temp, matrix, matrix);
}  /* ptk_rotatevv3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_rotateline3(C(Ppoint3 *) p1, C(Ppoint3 *) p2, 
                            C(Pfloat) theta, C(Pcomptype) operation, 
                            C(Pmatrix3) matrix, C(Pint *)error)
PreANSI(Ppoint3 *p1)
PreANSI(Ppoint3 *p2)
PreANSI(Pfloat theta) 
PreANSI(Pcomptype operation)
PreANSI(Pmatrix3 matrix)
PreANSI(Pint *error)
/*
** \parambegin
** \param{Ppoint3 *}{p1}{3D point}{IN}
** \param{Ppoint3 *}{p2}{3D point}{IN}
** \param{Pfloat}{theta}{angle in degrees}{IN}
** \param{Pcomptype}{operation}{concatenation operation}{IN}
** \param{Pmatrix3}{matrix}{4x4 matrix}{OUT}
** \param{Pint *}{error}{error code}{OUT}
** \paramend
** \blurb{Computes a matrix to perform a rotation and concatenates this with 
** \pardesc{matrix} on the basis of \pardesc{operation}.
** The rotation  is by \pardesc{theta} degrees
** about the line connecting \pardesc{p1} to \pardesc{p2}.}
*/
{
  Pmatrix3 temp;
  Pvector3 nvec, mnvec;
  Pfloat t1, costheta, sinetheta;
  Pvector3 v1, v2;

  *error = ptkcpcok;
  /* make a unit vector, i.e. direction cosines
  ** nvec = p2 - p1.
  */
  v1 = ptk_pt3tovec3(p1);
  v2 = ptk_pt3tovec3(p2);
  nvec = ptk_subv(&v2, &v1);
  if (ptk_nullv(&nvec)) {*error = -1;}
  nvec = ptk_unitv(&nvec);
  costheta = cos(theta * ptkcdtor);
  sinetheta = sin(theta * ptkcdtor);
  t1 = 1.0 - costheta;
  ptk_unitmatrix3(temp); /* get a unit matrix */
  temp[0][0] = (nvec.x * nvec.x) + (1.0 - (nvec.x * nvec.x)) * costheta;
  temp[1][1] = (nvec.y * nvec.y) + (1.0 - (nvec.y * nvec.y)) * costheta;
  temp[2][2] = (nvec.z * nvec.z) + (1.0 - (nvec.z * nvec.z)) * costheta;
  temp[1][0] = (nvec.x * nvec.y * t1) + (nvec.z * sinetheta);
  temp[2][0] = (nvec.x * nvec.z * t1) - (nvec.y * sinetheta); 
  temp[0][1] = (nvec.x * nvec.y * t1) - (nvec.z * sinetheta);
  temp[2][1] = (nvec.y * nvec.z * t1) + (nvec.x * sinetheta);
  temp[0][2] = (nvec.x * nvec.z * t1) + (nvec.y * sinetheta);
  temp[1][2] = (nvec.y * nvec.z * t1) - (nvec.x * sinetheta);
  mnvec = ptk_scalev(&v1, -1.0);
  ptk_shift3(&mnvec, PPRECONCATENATE, temp);
  ptk_shift3(&v1, PPOSTCONCATENATE, temp);
  ptk_concatenatematrix3(operation, temp, matrix, matrix);
}  /* ptk_rotateline3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Ppoint4 ptk_pt3topt4(C(Ppoint3 *) point)
PreANSI(Ppoint3 *point)
/*
** \parambegin
** \param{Ppoint3 *}{point}{3D point}{IN}
** \paramend
** \blurb{Convert 3D point to 4D point.}
*/
{
  Ppoint4 temp;

  temp.x = point->x;
  temp.y = point->y;
  temp.z = point->z;
  temp.w = 1.0;
  return temp;
}  /* ptk_pt3topt4 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Ppoint3 ptk_pt4topt3(C(Ppoint4 *) point)
PreANSI(Ppoint4 *point)
/*
** \parambegin
** \param{Ppoint4 *}{point}{4D point}{IN}
** \paramend
** \blurb{Convert 4D point to 3D point.}
*/
{
  Ppoint3 temp;
  Pfloat w;

  if (ptk_equal(fabs(point->w), 0.0))
    w = 1.0 / ptkcpceps;
  else
    w = 1.0 / point->w;
  temp.x = point->x * w;
  temp.y = point->y * w;
  temp.z = point->z * w;
  return temp;
}  /* ptk_pt4topt3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern Ppoint4 ptk_transform4(C(Pmatrix3) matrix, C(Ppoint4 *) point)
PreANSI(Pmatrix3 matrix)
PreANSI(Ppoint4 *point)
/*
** \parambegin
** \param{Pmatrix3}{matrix}{4x4 matrix}{IN}
** \param{Ppoint4 *}{point}{4D point}{IN}
** \paramend
** \blurb{This routine returns performs a 4D transformation 
** (i.e. {\bf no} division) of
** point \pardesc{pt} by \pardesc{matrix}, and returns the result as
** the value of the function.}
*/
{
  Ppoint4 temp;

  temp.x = matrix[0][0] * point->x + matrix[0][1] * point->y +
	     matrix[0][2] * point->z + matrix[0][3] * point->w;
  temp.y = matrix[1][0] * point->x + matrix[1][1] * point->y +
	     matrix[1][2] * point->z + matrix[1][3] * point->w;
  temp.z = matrix[2][0] * point->x + matrix[2][1] * point->y +
	     matrix[2][2] * point->z + matrix[2][3] * point->w;
  temp.w = matrix[3][0] * point->x + matrix[3][1] * point->y +
	     matrix[3][2] * point->z + matrix[3][3] * point->w;
  return temp;
}  /* ptk_transform4 */

/*----------------------------------------------------------------------------*/

/*function:external*/
extern Ppoint3 ptk_transform3(C(Pmatrix3) matrix, C(Ppoint3 *) point)
PreANSI(Pmatrix3 matrix)
PreANSI(Ppoint3 *point)
/*
** \parambegin
** \param{Pmatrix3}{matrix}{4x4 matrix}{IN}
** \param{Ppoint3 *}{point}{3D point}{IN}
** \paramend
** \blurb{Transforms the point \pardesc{point} by \pardesc{matrix},
** including homogeneous division.}
*/
{
  Ppoint3 temp;
  Pfloat w;

  temp.x = matrix[0][0] * point->x + matrix[0][1] * point->y +
	     matrix[0][2] * point->z + matrix[0][3];
  temp.y = matrix[1][0] * point->x + matrix[1][1] * point->y +
	     matrix[1][2] * point->z + matrix[1][3];
  temp.z = matrix[2][0] * point->x + matrix[2][1] * point->y +
	     matrix[2][2] * point->z + matrix[2][3];
  w = matrix[3][0] * point->x + matrix[3][1] * point->y + 
        matrix[3][2] * point->z + matrix[3][3];
  if (ptk_equal(fabs(w), 0.0))
    w = 1.0 / ptkcpceps;
  else
    w = 1.0 / w;
  temp.x *= w;
  temp.y *= w;
  temp.z *= w;
  return temp;
}  /* ptk_transform3 */

/*-------------------------------------------------------------------*/

/*function:external*/
extern void ptk_matrixtomatrix3(C(Pmatrix) mat, C(Pmatrix3) mat3)
PreANSI(Pmatrix mat)
PreANSI(Pmatrix mat3)
/*
** \parambegin
** \param{Pmatrix}{mat}{3x3 matrix}{IN}
** \param{Pmatrix3}{mat3}{4x4 matrix}{OUT}
** \paramend
** \blurb{Convert 3x3 matrix to 4x4 matrix.
** Uses method given on p.80 of ISO PHIGS 9592.}
*/
{
  ptk_unitmatrix3(mat3);
  mat3[0][0] = mat[0][0]; /* a */
  mat3[0][1] = mat[0][1]; /* b */
  mat3[1][0] = mat[1][0]; /* d */
  mat3[1][1] = mat[1][1]; /* e */
  mat3[0][3] = mat[0][2]; /* c */
  mat3[1][3] = mat[1][2]; /* f */
  mat3[3][0] = mat[2][0]; /* g */
  mat3[3][1] = mat[2][1]; /* h */
  mat3[3][3] = mat[2][2]; /* j */ 
}  /* ptk_matrixtomatrix3 */

/*----------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_outputmatrix3(C(FILE *) fileptr, C(Pmatrix3) matrix, 
                              C(Pchar *) string)
PreANSI(FILE *fileptr)
PreANSI(Pmatrix3 matrix)
PreANSI(Pchar *string)
/*
** \parambegin
** \param{FILE *}{fileptr}{file pointer}{OUT}
** \param{Pmatrix3}{matrix}{4x4 matrix}{IN}
** \param{Pchar *}{string}{character string}{IN}
** \paramend
** \blurb{Outputs \pardesc{matrix} and the message \pardesc{string}
** to the file specified by \pardesc{f}.}
*/
{
  Pint ii;

  /* Output Title */
  fprintf(fileptr,"\n*************************************************\n");
  fprintf(fileptr,"%s\n", string);
  fprintf(fileptr,"           1          2          3          4\n");
  fprintf(fileptr,"-------------------------------------------------\n");
  for (ii = 0; ii <= 3; ii++)
    fprintf(fileptr,"%2ld | %11.4f%11.4f%11.4f%11.4f\n",
	    ii + 1, matrix[ii][0], matrix[ii][1], matrix[ii][2], 
            matrix[ii][3]);
  fprintf(fileptr,"\n*************************************************\n");
}  /* ptk_outputmatrix3 */

/*----------------------------------------------------------------------------*/

static boolean validbounds(C(Plimit3 *) bound)
PreANSI(Plimit3 *bound)
/*
** \parambegin
** \param{Pint}{}{}{IN}
** \paramend
** description: checks if bounding box is sensibly defined (the min value
** is less than max value in each axis).
** input params: bound - 3D bounding box.
** output params: none.
** return value: TRUE if bounding box is valid, otherwise FALSE.
*/
{
  return ((bound->xmin <= bound->xmax) && (bound->ymin <= bound->ymax) &&
	  (bound->zmin <= bound->zmax));
}  /* validbounds */

/*----------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_box3tobox3(C(Plimit3 *) box1, C(Plimit3 *) box2, 
                           C(boolean) preserve, C(Pcomptype) operation, 
                           C(Pmatrix3) matrix, C(Pint *)error)
PreANSI(Plimit3 *box1)
PreANSI(Plimit3 *box2)
PreANSI(boolean preserve)
PreANSI(Pcomptype operation) 
PreANSI(Pmatrix3 matrix)
PreANSI(Pint *error)
/*
** \parambegin
** \param{Plimit3 *}{box1}{3D volume}{IN}
** \param{Plimit3 *}{box2}{3D volume}{IN}
** \param{boolean}{preserve}{preserve aspect ratio}{IN}
** \param{Pcomptype}{operation}{concatenation operation}{IN}
** \param{Pmatrix3}{matrix}{4x4 matrix}{OUT}
** \param{Pint *}{error}{error code}{OUT}
** \paramend
** \blurb{Computes a mapping from one box to another --- a window to viewport
** transformation --- and concatenates it with \pardesc{matrix} on the
** basis of \pardesc{operation}.}
*/
{
  Pvector3 vpscale, winscale;
  Pmatrix3 temp;
  Pvector3 winmin, winmax, vpmin, vpmax, wincentre, vpcentre;
  Pfloat minscale;

  *error = ptkcpcok;   /* assume OK */
  if (!validbounds(box1)) 
  {
    *error = -1;
    return;
  }
  winmin = ptk_vector3(box1->xmin, box1->ymin, box1->zmin);
  winmax = ptk_vector3(box1->xmax, box1->ymax, box1->zmax);
  vpmin = ptk_vector3(box2->xmin, box2->ymin, box2->zmin);
  vpmax = ptk_vector3(box2->xmax, box2->ymax, box2->zmax);

  winscale = ptk_subv(&winmax, &winmin);
  wincentre = ptk_scalev(&winscale, 0.5);
  wincentre = ptk_addv(&winmin, &wincentre);
  wincentre = ptk_scalev(&wincentre, -1.0);
  vpscale = ptk_subv(&vpmax, &vpmin);
  vpcentre = ptk_scalev(&vpscale, 0.5);
  vpcentre = ptk_addv(&vpmin, &vpcentre);
  if (winscale.x == 0.0)
    winscale.x = 1.0;
  else
    winscale.x = vpscale.x / winscale.x;
  if (winscale.y == 0.0)
    winscale.y = 1.0;
  else
    winscale.y = vpscale.y / winscale.y;
  if (winscale.z == 0.0)
    winscale.z = 1.0;
  else
    winscale.z = vpscale.z / winscale.z;
  if (preserve)
  {
    minscale = MIN(winscale.x, winscale.y);
    minscale = MIN(minscale, winscale.z);
    winscale = ptk_vector3(minscale, minscale, minscale);
  }
  ptk_shift3(&vpcentre, PREPLACE, temp);
  ptk_scale3(&winscale, PPRECONCATENATE, temp);
  ptk_shift3(&wincentre, PPRECONCATENATE, temp);
  ptk_concatenatematrix3(operation, temp, matrix, matrix);
}  /* ptk_box3tobox3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_accumulatetran3(C(Ppoint3 *) fixed, C(Pvector3 *) shift, 
                                C(Pfloat) rotx, C(Pfloat) roty, 
                                C(Pfloat) rotz, C(Pvector3 *) scale, 
                                C(Pcomptype) operation, 
                                C(Pmatrix3) matrix)
PreANSI(Ppoint3 *fixed)
PreANSI(Pvector3 *shift) 
PreANSI(Pfloat rotx)
PreANSI(Pfloat roty)
PreANSI(Pfloat rotz) 
PreANSI(Pvector3 *scale)
PreANSI(Pcomptype operation) 
PreANSI(Pmatrix3 matrix)
/*
** \parambegin
** \param{Ppoint3 *}{fixed}{origin}{IN}
** \param{Pvector3 *}{shift}{shift factor}{IN}
** \param{Pfloat}{rotx}{x rotation}{IN}
** \param{Pfloat}{rotx}{y rotation}{IN}
** \param{Pfloat}{rotx}{z rotation}{IN}
** \param{Pvector3 *}{scale}{scale factor}{IN}
** \param{Pcomptype}{operation}{concatenation operation}{IN}
** \param{Pmatrix3}{matrix}{4x4 matrix}{OUT}
** \paramend
** \blurb{Combines shift, scale and rotate into one function call
** and concatenates with given matrix.}
*/
{
  Pmatrix3 temp;
  Pvector3 vfix, mfixed;

  vfix = ptk_pt3tovec3(fixed);
  mfixed = ptk_scalev(&vfix, -1.0);
  ptk_shift3(shift, PREPLACE, temp);
  ptk_shift3(&vfix, PPRECONCATENATE, temp);
  ptk_rotate3(rotx, PTKEXAXIS, PPRECONCATENATE, temp);
  ptk_rotate3(roty, PTKEYAXIS, PPRECONCATENATE, temp);
  ptk_rotate3(rotz, PTKEZAXIS, PPRECONCATENATE, temp);
  ptk_scale3(scale, PPRECONCATENATE, temp);
  ptk_shift3(&mfixed, PPRECONCATENATE, temp);
  ptk_concatenatematrix3(operation, temp, matrix, matrix);
}  /* ptk_accumulatetran3 */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_evalvieworientation3(C(Ppoint3 *) viewrefpoint, 
                                    C(Pvector3 *) viewplanenormal,
				    C(Pvector3 *) viewupvector, 
                                    C(Pcomptype) operation, 
                                    C(Pmatrix3) matrix, C(Pint *)error)
PreANSI(Ppoint3 *viewrefpoint)
PreANSI(Pvector3 *viewplanenormal)
PreANSI(Pvector3 *viewupvector)
PreANSI(Pcomptype operation) 
PreANSI(Pmatrix3 matrix)
PreANSI(Pint *error)
/*
** \parambegin
** \param{Ppoint3 *}{viewrefpoint}{view reference point}{IN}
** \param{Pvector3 *}{viewplanenormal}{view plane normal}{IN}
** \param{Pvector3 *}{viewupvector}{view up vector}{IN}
** \param{Pcomptype}{operation}{concatenation operation}{IN}
** \param{Pmatrix3}{matrix}{4x4 matrix}{OUT}
** \param{Pint *}{error}{error code}{OUT}
** \paramend
** \blurb{Given viewrefpoint, viewplanenormal and viewupvector
** returns the view orientation matrix.
** error == 61 if viewplanenormal is null,
** error == 63 if viewupvector is null,
** error == 58 if cross product of viewupvector and viewplanenormal is null.}
*/
{
  Pvector3 uaxis, vaxis, naxis, vuvxvpn;
  Pmatrix3 temp;

  *error = ptkcpcok;
  if (ptk_nullv(viewplanenormal)) 
  {
    *error = 61;
  } 
  else 
  if (ptk_nullv(viewupvector)) 
  {
    *error = 63;
  } 
  else 
  {
    vuvxvpn = ptk_crossv(viewupvector, viewplanenormal);
    if (ptk_nullv(&vuvxvpn))
      *error = 58;
    else
      *error = ptkcpcok;
  }
  if (*error != ptkcpcok) 
  {
    return;
  }  /* if error = 0 */
  /* generate from viewplanenormal and viewupvector a set
  ** of orthogonal axes denoted by uaxis, vaxis, naxis 
  */
  naxis = ptk_unitv(viewplanenormal);
  uaxis = ptk_unitv(&vuvxvpn);
  vaxis = ptk_crossv(&naxis, &uaxis);
  vaxis = ptk_unitv(&vaxis);
  /* form view orientation matrix */
  ptk_unitmatrix3(temp);
  temp[0][0] = uaxis.x;
  temp[0][1] = uaxis.y;
  temp[0][2] = uaxis.z;
  temp[0][3] = -(uaxis.x * viewrefpoint->x + uaxis.y * 
                   viewrefpoint->y + uaxis.z * viewrefpoint->z);
  temp[1][0] = vaxis.x;
  temp[1][1] = vaxis.y;
  temp[1][2] = vaxis.z;
  temp[1][3] = -(vaxis.x * viewrefpoint->x + vaxis.y * 
                   viewrefpoint->y + vaxis.z * viewrefpoint->z);
  temp[2][0] = naxis.x;
  temp[2][1] = naxis.y;
  temp[2][2] = naxis.z;
  temp[2][3] = -(naxis.x * viewrefpoint->x + naxis.y * 
                   viewrefpoint->y + naxis.z * viewrefpoint->z);
  ptk_concatenatematrix3(operation, temp, matrix, matrix);
}  /* ptk_evalvieworientation3 */

/*----------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_evalviewmapping3(C(Plimit3 *) wlimits, C(Plimit3 *) vlimits, 
                                C(Pprojtype) viewtype, 
                                C(Ppoint3 *) refpoint, C(Pfloat) vplanedist, 
                                C(Pcomptype) operation, C(Pmatrix3) matrix, 
                                C(Pint *)error)
PreANSI(Plimit3 *wlimits)
PreANSI(Plimit3 *vlimits) 
PreANSI(Pprojtype viewtype) 
PreANSI(Ppoint3 *refpoint)
PreANSI(Pfloat vplanedist) 
PreANSI(Pcomptype operation)
PreANSI(Pmatrix3 matrix) 
PreANSI(Pint *error)
/*
** \parambegin
** \param{Plimit3 *}{wlimits}{window limits}{IN}
** \param{Plimit3 *}{vlimits}{viewport limits}{IN}
** \param{Pprojtype}{viewtype}{projection type}{IN}
** \param{Ppoint3 *}{refpoint}{projection reference point}{IN}
** \param{Pfloat}{vplanedist}{view plane distance}{IN}
** \param{Pcomptype}{operation}{concatenation operation}{IN}
** \param{Pmatrix3}{matrix}{4x4 matrix}{OUT}
** \param{Pint *}{error}{error code}{OUT}
** \paramend
** \blurb{Evaluate view mapping matrix.
** error == 329 if window limits not valid in x and y,
** error == 336 if back plane in front of front plane,
** error == 330 if viewport limits are not valid,
** error == 335 if PRP is on the view plane,
** error == 340 if PRP is between front and back planes.}
*/
{
  Pvector3 projectordirn, viewwindowcentre, vref;
  Pmatrix3 temp;

  *error = ptkcpcok;
  /* check for valid window limits */
  if (wlimits->xmin >= wlimits->xmax ||
      wlimits->ymin >= wlimits->ymax) 
  {
    *error = 329;
    return;
  }
  /* check that back plane is not in front of front plane */
  if (wlimits->zmin >= wlimits->zmax) 
  {
    *error = 336;
    return;
  }
  /* check for valid viewport limits */
  if (!validbounds(vlimits)) 
  {
    *error = 330;
    return;
  }
  /* check that PRP not positioned on the view plane */
  if (ptk_equal(refpoint->z, vplanedist)) 
  {
    *error = 335;
    return;
  }
  /* check that PRP not between the front and back planes */
  if (refpoint->z < wlimits->zmax && refpoint->z > wlimits->zmin) 
  {
    *error = 340;
    return;
  }
  /* if everything is OK error is set to zero and a value computed for 
  ** matrix.
  */
  *error = ptkcpcok;
  ptk_unitmatrix3(temp);
  switch (viewtype) 
  {
  case PPERSPECTIVE:
    temp[0][0] = refpoint->z - vplanedist;
    temp[0][2] = -refpoint->x;
    temp[0][3] = refpoint->x * vplanedist;
    temp[1][0] = 0.0;
    temp[1][1] = refpoint->z - vplanedist;
    temp[1][2] = -refpoint->y;
    temp[1][3] = refpoint->y * vplanedist;
    temp[2][2] = 0.0;
    temp[2][3] = 1.0;
    temp[3][2] = -1.0;
    temp[3][3] = refpoint->z;
    break;

  case PPARALLEL:
    viewwindowcentre.x = (wlimits->xmin + wlimits->xmax) / 2;
    viewwindowcentre.y = (wlimits->ymin + wlimits->ymax) / 2;
    viewwindowcentre.z = vplanedist;
    vref = ptk_pt3tovec3(refpoint);
    projectordirn = ptk_subv(&vref, &viewwindowcentre);
    projectordirn = ptk_scalev(&projectordirn, 
                               1.0 / (refpoint->z - vplanedist));
    temp[0][2] = -projectordirn.x;
    temp[0][3] = vplanedist * projectordirn.x;
    temp[1][2] = -projectordirn.y;
    temp[1][3] = vplanedist * projectordirn.y;
    break;
  }
  /* The z coordinate is unchanged by the transformation matrix above for
  ** parallel projection types and changed for perspective projection
  ** types. Different action is therefore required in each case so that
  ** points at the front and back planes are always mapped onto points at
  ** the front and back of the projection viewport 
  */
  if (viewtype == PPERSPECTIVE) 
  {
    wlimits->zmin = 1 / (refpoint->z - wlimits->zmin);
    wlimits->zmax = 1 / (refpoint->z - wlimits->zmax);
  }
  ptk_box3tobox3(wlimits, vlimits, FALSE, PPOSTCONCATENATE, temp, error);
  ptk_concatenatematrix3(operation, temp, matrix, matrix);
}  /* of ptk_evalviewmapping3 */

/*----------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_stackmatrix3(C(Pmatrix3) matrix)
PreANSI(Pmatrix3 matrix)
/*
** \parambegin
** \param{Pmatrix3}{matrix}{4x4 matrix}{IN}
** \paramend
** \blurb{Pushes \pardesc{matrix} onto the transformation stack.}
*/
{
  newone = (ptksstack *)malloc(sizeof(ptksstack));
  newone->ptkpnext = listhead;
  memcpy(newone->ptkamat, matrix, sizeof(Pmatrix3));
  listhead = newone;
}  /* ptk_stackmatrix3 */

/*----------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_unstackmatrix3(C(Pmatrix3) matrix)
PreANSI(Pmatrix3 matrix)
/*
** \parambegin
** \param{Pmatrix3}{matrix}{4x4 matrix}{OUT}
** \paramend
** \blurb{Pops a matrix  from the transformation stack and returns it in
** \pardesc{matrix}.}
*/
{
  if (listhead == NULL)
    return;
  memcpy(matrix, listhead->ptkamat, sizeof(Pmatrix3));
  newone = listhead->ptkpnext;
  free(listhead);
  listhead = newone;
}  /* ptk_unstackmatrix3 */

/*---------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_examinestackmatrix3(C(Pmatrix3) matrix)
PreANSI(Pmatrix3 matrix)
/*
** \parambegin
** \param{Pmatrix3}{matrix}{4x4 matrix}{IN}
** \paramend
** \blurb{This routine returns the top element of the transformation stack.
** The stack is not disturbed.}
*/
{
  if (listhead != NULL)
    memcpy(matrix, listhead->ptkamat, sizeof(Pmatrix3));
}  /* ptk_examinestackmatrix3 */

/*---------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_3ptto3pt(C(Ppoint3 *) p1, C(Ppoint3 *) p2, C(Ppoint3 *) p3, 
                         C(Ppoint3 *) q1, C(Ppoint3 *) q2, C(Ppoint3 *) q3, 
                         C(Pcomptype) operation, C(Pmatrix3) matrix, 
                         C(Pint *)error)
PreANSI(Ppoint3 *p1)
PreANSI(Ppoint3 *p2)
PreANSI(Ppoint3 *p3)
PreANSI(Ppoint3 *q1) 
PreANSI(Ppoint3 *q2)
PreANSI(Ppoint3 *q3)
PreANSI(Pcomptype operation) 
PreANSI(Pmatrix3 matrix)
PreANSI(Pint *error)
/*
** \parambegin
** \param{Ppoint3 *}{p1}{3D point}{IN}
** \param{Ppoint3 *}{p2}{3D point}{IN}
** \param{Ppoint3 *}{p3}{3D point}{IN}
** \param{Ppoint3 *}{q1}{3D point}{IN}
** \param{Ppoint3 *}{q2}{3D point}{IN}
** \param{Ppoint3 *}{q3}{3D point}{IN}
** \param{Pcomptype}{operation}{concatenation operation}{IN}
** \param{Pmatrix3}{matrix}{4x4 matrix}{OUT}
** \param{Pint *}{error}{error code}{OUT}
** \paramend
** \blurb{This function returns the 3 point to 3 point transformation as
** described in \cite{mort:geom}.
** The transformation has the following properties:
** \pardesc{p1} is transformed onto \pardesc{q1};
** the vector \pardesc{(p2-p1)} is transformed to be parallel with the vector 
** \pardesc{(q2-q1)};
** the plane containing the three points \pardesc{p1, p2, p3} is
** transformed into  the plane containing \pardesc{q1, q2, q3}.}
*/
{
  Pmatrix3 temp, invvmatrix, wmatrix;
  Pvector3 v1, v2, v3, w1, w2, w3;
  Pvector3 vp1, vp2, vp3, vq1, vq2, vq3;
  Pvector3 t1, t2;
  Ppoint3 t3;

  /* form 2 right-hand orthogonal systems from the given points */
  *error = ptkcpcok;
  vp1 = ptk_pt3tovec3(p1);
  vp2 = ptk_pt3tovec3(p2);
  vp3 = ptk_pt3tovec3(p3);
  vq1 = ptk_pt3tovec3(q1);
  vq2 = ptk_pt3tovec3(q2);
  vq3 = ptk_pt3tovec3(q3);
  v1 = ptk_subv(&vp2, &vp1);
  v1 = ptk_unitv(&v1);
  v3 = ptk_subv(&vp3, &vp1);
  v3 = ptk_crossv(&v1, &v3);
  v3 = ptk_unitv(&v3);
  v2 = ptk_crossv(&v3, &v1);
  v2 = ptk_unitv(&v2);

  // Following code has been adjusted for WATCOM C
  // The compiler was detecting the error :
  // E1071: Type of parameter 1 does not agree with previous definition

  // The line :
  // w1 = ptk_unitv(ptk_subv(&vq2, &vq1));
  // Replaced with :
  t1 = ptk_subv(&vq2, &vq1);
  w1 = ptk_unitv(&t1);

  // The line :
  // w3 = ptk_unitv(ptk_crossv(w1, ptk_subv(&vq3, &vq1)));
  // Replaced with :
  t1 = ptk_subv(&vq3, &vq1);
  t2 = ptk_crossv(&w1, &t1);
  w3 = ptk_unitv(&t2);

  // The line :
  // w2 = ptk_unitv(ptk_crossv(w3, w1));
  // Replaced with :
  t1 = ptk_crossv(&w3, &w1);
  w2 = ptk_unitv(&t1);

  /* if any of v1..v3, w1..w3 are null vectors then the parameters are 
  ** invalid.
  */
  if (ptk_nullv(&v1) || ptk_nullv(&v2) || ptk_nullv(&v3) || ptk_nullv(&w1) ||
      ptk_nullv(&w2) || ptk_nullv(&w3)) 
  {
    *error = -1;
    return;
  }
  *error = ptkcpcok;
  ptk_unitmatrix3(invvmatrix);
  invvmatrix[0][0] = v1.x;
  invvmatrix[0][1] = v1.y;
  invvmatrix[0][2] = v1.z;
  invvmatrix[1][0] = v2.x;
  invvmatrix[1][1] = v2.y;
  invvmatrix[1][2] = v2.z;
  invvmatrix[2][0] = v3.x;
  invvmatrix[2][1] = v3.y;
  invvmatrix[2][2] = v3.z;
  ptk_unitmatrix3(wmatrix);
  wmatrix[0][0] = w1.x;
  wmatrix[1][0] = w1.y;
  wmatrix[2][0] = w1.z;
  wmatrix[0][1] = w2.x;
  wmatrix[1][1] = w2.y;
  wmatrix[2][1] = w2.z;
  wmatrix[0][2] = w3.x;
  wmatrix[1][2] = w3.y;
  wmatrix[2][2] = w3.z;

  /* shift points at origin onto q1 */

  ptk_shift3(q1, PREPLACE, temp);
  ptk_concatenatematrix3(PPRECONCATENATE, wmatrix, wmatrix, temp);
  ptk_concatenatematrix3(PPRECONCATENATE, invvmatrix, temp, temp);

  /* shift p1 to origin */

  // Following code has been adjusted for WATCOM C
  // The compiler was detecting the error :
  // E1071: Type of parameter 1 does not agree with previous definition

  // The line :
  // ptk_shift3(ptk_scalev(p1, -1.0), PPRECONCATENATE, temp);
  // Replaced with :
  t3 = ptk_scalev(p1, -1.0);
  ptk_shift3(&t3, PPRECONCATENATE, temp);

  ptk_concatenatematrix3(operation, temp, matrix, matrix);
}  /* ptk_3ptto3pt */

/*--------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_0to3pt(C(Ppoint3 *) origin, C(Pvector3 *) xdirn, 
              C(Pvector3 *) ydirn, C(Pcomptype) operation, C(Pmatrix3) matrix)
PreANSI(Ppoint3 *origin)
PreANSI(Pvector3 *xdirn)
PreANSI(Pvector3 *ydirn) 
PreANSI(Pcomptype operation)
PreANSI(Pmatrix3 matrix)
/*
** \parambegin
** \param{Ppoint3 *}{origin}{origin of axes}{IN}
** \param{Pvector3 *}{xdirn}{x direction}{IN}
** \param{Pvector3 *}{y dirn}{y direction}{IN}
** \param{Pcomptype}{operation}{concatenation operation}{IN}
** \param{Pmatrix3}{matrix}{4x4 matrix}{OUT}
** \paramend
** \blurb{Computes an object transformation which would map unit vectors 
** along the x, y and z axes onto unit vectors along the corresponding axes 
** of the new coordinate system.}
*/
{
  Pvector3 xaxis, yaxis, zaxis;
  Pmatrix3 objectxform;

  /* work out unit vectors along the axes of new coordinate system */

  xaxis = ptk_unitv(xdirn);
  zaxis = ptk_crossv(&xaxis, ydirn);
  zaxis = ptk_unitv(&zaxis);
  yaxis = ptk_crossv(&zaxis, &xaxis);
  yaxis = ptk_unitv(&yaxis);

  /* form matrices - see "An Implementation of the GKS-3D/PHIGS Viewing
  ** Pipeline" for the maths. 
  */

  ptk_unitmatrix3(objectxform);
  objectxform[0][0] = xaxis.x;
  objectxform[1][0] = xaxis.y;
  objectxform[2][0] = xaxis.z;
  objectxform[0][1] = yaxis.x;
  objectxform[1][1] = yaxis.y;
  objectxform[2][1] = yaxis.z;
  objectxform[0][2] = zaxis.x;
  objectxform[1][2] = zaxis.y;
  objectxform[2][2] = zaxis.z;
  objectxform[0][3] = origin->x;
  objectxform[1][3] = origin->y;
  objectxform[2][3] = origin->z;
  ptk_concatenatematrix3(operation, objectxform, matrix, matrix);

  /* To output text, call above procedure to get the character plane 
  ** transformation:
  **
  **  ptk_0to3pt( text_point, text_direction_vector_1,
  **             text_direction_vector_2, PVReplacematrix,char_plane_xform)
  **
  ** As each point of the text is output, transform it by:
  **
  **  output_pipeline . char_plane_xform 
  */
}  /* ptk_0to3pt */

/*---------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_oto3pt(C(Ppoint3 *) origin, C(Pvector3 *) xdirn, 
             C(Pvector3 *) ydirn, C(Pcomptype) operation, C(Pmatrix3) matrix)
PreANSI(Ppoint3 *origin)
PreANSI(Pvector3 *xdirn)
PreANSI(Pvector3 *ydirn) 
PreANSI(Pcomptype operation)
PreANSI(Pmatrix3 matrix)
/*
** \parambegin
** \param{Ppoint3 *}{origin}{origin of axes}{IN}
** \param{Pvector3 *}{xdirn}{x direction}{IN}
** \param{Pvector3 *}{y dirn}{y direction}{IN}
** \param{Pcomptype}{operation}{concatenation operation}{IN}
** \param{Pmatrix3}{matrix}{4x4 matrix}{OUT}
** \paramend
** \blurb{This is the same as \pardesc{ptk\_0to3pt}, except the \pardesc{o}\ 
** (oh) instead of \pardesc{0}\ (zero).}
*/
{
  ptk_0to3pt(origin, xdirn, ydirn, operation, matrix);
}  /* ptk_oto3pt */

/*---------------------------------------------------------------------------*/

/*function:external*/
extern void ptk_invertmatrix3(C(Pmatrix3) a, C(Pmatrix3) ainverse, 
                              C(Pint *)error)
PreANSI(Pmatrix3 a)
PreANSI(Pmatrix3 ainverse)
PreANSI(Pint *error)
/*
** \parambegin
** \param{Pmatrix3}{a}{4x4 matrix}{IN}
** \param{Pmatrix3}{ainverse}{4x4 matrix}{OUT}
** \param{Pint *}{error}{error code}{OUT}
** \paramend
** \blurb{This procedure computes the inverse of matrix \pardesc{a}, 
** returning the result in \pardesc{ainverse}. 
** If matrix \pardesc{a} is singular, then 
** \pardesc{error} is set \pardesc{true} and \pardesc{ainverse} is undefined,
** otherwise \pardesc{error} is set to \pardesc{false}.}
*/
{
  Pfloat b[4], c[4];
  Pfloat w, y;
  Pint z[4];
  Pint i, j, k, l, p;

  *error = -1;
  memcpy(ainverse, a, sizeof(Pmatrix3));
  for (j = 1; j <= 4; j++)
    z[j - 1] = j;
  for (i = 1; i <= 4; i++) 
  {
    k = i;
    y = ainverse[i - 1][i - 1];
    l = i - 1;
    p = i + 1;
    for (j = p; j <= 4; j++) 
    {
      w = ainverse[i - 1][j - 1];
      if (fabs(w) > fabs(y)) 
      {
	k = j;
	y = w;
      }
    }
    if (ptk_equal(y, 0.0))   /* matrix has no inverse */
      return;
    y = 1.0 / y;
    for (j = 0; j <= 3; j++) 
    {
      c[j] = ainverse[j][k - 1];
      ainverse[j][k - 1] = ainverse[j][i - 1];
      ainverse[j][i - 1] = -c[j] * y;
      ainverse[i - 1][j] *= y;
      b[j] = ainverse[i - 1][j];
    }
    ainverse[i - 1][i - 1] = y;
    j = z[i - 1];
    z[i - 1] = z[k - 1];
    z[k - 1] = j;
    for (k = 0; k < l; k++) 
    {
      for (j = 0; j < l; j++)
	ainverse[k][j] -= b[j] * c[k];
      for (j = p - 1; j <= 3; j++)
	ainverse[k][j] -= b[j] * c[k];
    }
    for (k = p - 1; k <= 3; k++) 
    {
      for (j = 0; j < l; j++)
	ainverse[k][j] -= b[j] * c[k];
      for (j = p - 1; j <= 3; j++)
	ainverse[k][j] -= b[j] * c[k];
    }
  }
  for (i = 0; i <= 3; i++) 
  {
    do 
    {
      k = z[i];
      if (k != i + 1) 
      {
	for (j = 0; j <= 3; j++) 
        {
	  w = ainverse[i][j];
	  ainverse[i][j] = ainverse[k - 1][j];
	  ainverse[k - 1][j] = w;
	}
	p = z[i];
	z[i] = z[k - 1];
	z[k - 1] = p;
      }
    } while (k != i + 1);
  }
  *error = ptkcpcok;
}  /* ptk_invertmatrix3 */

/*--------------------------------------------------------------------------*/

/* End. */
