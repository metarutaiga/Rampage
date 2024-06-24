/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** $Log: 
**  4    Sage      1.3         12/6/00  Tim Little      Updated to fix lighting 
**       with SAGE
**  3    Sage      1.2         10/31/00 Brent Burton    Added ddrawi.h to list 
**       of includes.
**  2    Sage      1.1         10/27/00 Tim Little      Removed an old un-needed
**       include.
**  1    Sage      1.0         10/26/00 Tim Little      
** $
*/


#include <ddrawi.h>
#include <d3dhal.h>
#include "d6fvf.h"
#include "fxglobal.h"
#include "d3global.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "fifomgr.h"
#include "d3tri.h"
#include "d6fvf.h"
#include "geglobal.h"


//---------------------------------------------------------------------
// This function uses Cramer's Rule to calculate the matrix inverse.
// See nt\private\windows\opengl\serever\soft\so_math.c
//
// Returns:
//    0 - if success
//   -1 - if input matrix is singular
//
int Inverse4x4Transpose(D3DMATRIX *src,D3DMATRIX *inverseTranspose)
{
    int retval = 0;
    D3DMATRIX   t;
    t._11 = src->_11;
    t._21 = src->_12;
    t._31 = src->_13;
    t._41 = src->_14;

    t._12 = src->_21;
    t._22 = src->_22;
    t._32 = src->_23;
    t._42 = src->_24;

    t._13 = src->_31;
    t._23 = src->_32;
    t._33 = src->_33;
    t._43 = src->_34;

    t._14 = src->_41;
    t._24 = src->_42;
    t._34 = src->_43;
    t._44 = src->_44;
    retval = Inverse4x4(&t,inverseTranspose);

    return retval;
}
int Inverse4x4(D3DMATRIX *src, D3DMATRIX *inverse)
{
   double x00, x01, x02;
   double x10, x11, x12;
   double x20, x21, x22;
   double rcp;
   double x30, x31, x32;
   double y01, y02, y03, y12, y13, y23;
   double z02, z03, z12, z13, z22, z23, z32, z33;

#define x03 x01
#define x13 x11
#define x23 x21
#define x33 x31
#define z00 x02
#define z10 x12
#define z20 x22
#define z30 x32
#define z01 x03
#define z11 x13
#define z21 x23
#define z31 x33

   /* read 1st two columns of matrix into registers */
   x00 = src->_11;
   x01 = src->_12;
   x10 = src->_21;
   x11 = src->_22;
   x20 = src->_31;
   x21 = src->_32;
   x30 = src->_41;
   x31 = src->_42;

   /* compute all six 2x2 determinants of 1st two columns */
   y01 = x00*x11 - x10*x01;
   y02 = x00*x21 - x20*x01;
   y03 = x00*x31 - x30*x01;
   y12 = x10*x21 - x20*x11;
   y13 = x10*x31 - x30*x11;
   y23 = x20*x31 - x30*x21;

   /* read 2nd two columns of matrix into registers */
   x02 = src->_13;
   x03 = src->_14;
   x12 = src->_23;
   x13 = src->_24;
   x22 = src->_33;
   x23 = src->_34;
   x32 = src->_43;
   x33 = src->_44;

   /* compute all 3x3 cofactors for 2nd two columns */
   z33 = x02*y12 - x12*y02 + x22*y01;
   z23 = x12*y03 - x32*y01 - x02*y13;
   z13 = x02*y23 - x22*y03 + x32*y02;
   z03 = x22*y13 - x32*y12 - x12*y23;
   z32 = x13*y02 - x23*y01 - x03*y12;
   z22 = x03*y13 - x13*y03 + x33*y01;
   z12 = x23*y03 - x33*y02 - x03*y23;
   z02 = x13*y23 - x23*y13 + x33*y12;

   /* compute all six 2x2 determinants of 2nd two columns */
   y01 = x02*x13 - x12*x03;
   y02 = x02*x23 - x22*x03;
   y03 = x02*x33 - x32*x03;
   y12 = x12*x23 - x22*x13;
   y13 = x12*x33 - x32*x13;
   y23 = x22*x33 - x32*x23;

   /* read 1st two columns of matrix into registers */
   x00 = src->_11;
   x01 = src->_12;
   x10 = src->_21;
   x11 = src->_22;
   x20 = src->_31;
   x21 = src->_32;
   x30 = src->_41;
   x31 = src->_42;

   /* compute all 3x3 cofactors for 1st column */
   z30 = x11*y02 - x21*y01 - x01*y12;
   z20 = x01*y13 - x11*y03 + x31*y01;
   z10 = x21*y03 - x31*y02 - x01*y23;
   z00 = x11*y23 - x21*y13 + x31*y12;

   /* compute 4x4 determinant & its reciprocal */
   rcp = x30*z30 + x20*z20 + x10*z10 + x00*z00;
   if (rcp == (float)0)
   return -1;
   rcp = (float)1/rcp;

   /* compute all 3x3 cofactors for 2nd column */
   z31 = x00*y12 - x10*y02 + x20*y01;
   z21 = x10*y03 - x30*y01 - x00*y13;
   z11 = x00*y23 - x20*y03 + x30*y02;
   z01 = x20*y13 - x30*y12 - x10*y23;

   /* multiply all 3x3 cofactors by reciprocal */
   inverse->_11 = (float)(z00*rcp);
   inverse->_21 = (float)(z01*rcp);
   inverse->_12 = (float)(z10*rcp);
   inverse->_31 = (float)(z02*rcp);
   inverse->_13 = (float)(z20*rcp);
   inverse->_41 = (float)(z03*rcp);
   inverse->_14 = (float)(z30*rcp);
   inverse->_22 = (float)(z11*rcp);
   inverse->_32 = (float)(z12*rcp);
   inverse->_23 = (float)(z21*rcp);
   inverse->_42 = (float)(z13*rcp);
   inverse->_24 = (float)(z31*rcp);
   inverse->_33 = (float)(z22*rcp);
   inverse->_43 = (float)(z23*rcp);
   inverse->_34 = (float)(z32*rcp);
   inverse->_44 = (float)(z33*rcp);
   return 0;
}

//---------------------------------------------------------------------
#define MATRIX_PRODUCT(res, a, b)                                           \
   res->_11 = a->_11*b->_11 + a->_12*b->_21 + a->_13*b->_31 + a->_14*b->_41;   \
   res->_12 = a->_11*b->_12 + a->_12*b->_22 + a->_13*b->_32 + a->_14*b->_42;   \
   res->_13 = a->_11*b->_13 + a->_12*b->_23 + a->_13*b->_33 + a->_14*b->_43;   \
   res->_14 = a->_11*b->_14 + a->_12*b->_24 + a->_13*b->_34 + a->_14*b->_44;   \
                                                                               \
   res->_21 = a->_21*b->_11 + a->_22*b->_21 + a->_23*b->_31 + a->_24*b->_41;   \
   res->_22 = a->_21*b->_12 + a->_22*b->_22 + a->_23*b->_32 + a->_24*b->_42;   \
   res->_23 = a->_21*b->_13 + a->_22*b->_23 + a->_23*b->_33 + a->_24*b->_43;   \
   res->_24 = a->_21*b->_14 + a->_22*b->_24 + a->_23*b->_34 + a->_24*b->_44;   \
                                                                               \
   res->_31 = a->_31*b->_11 + a->_32*b->_21 + a->_33*b->_31 + a->_34*b->_41;   \
   res->_32 = a->_31*b->_12 + a->_32*b->_22 + a->_33*b->_32 + a->_34*b->_42;   \
   res->_33 = a->_31*b->_13 + a->_32*b->_23 + a->_33*b->_33 + a->_34*b->_43;   \
   res->_34 = a->_31*b->_14 + a->_32*b->_24 + a->_33*b->_34 + a->_34*b->_44;   \
                                                                               \
   res->_41 = a->_41*b->_11 + a->_42*b->_21 + a->_43*b->_31 + a->_44*b->_41;   \
   res->_42 = a->_41*b->_12 + a->_42*b->_22 + a->_43*b->_32 + a->_44*b->_42;   \
   res->_43 = a->_41*b->_13 + a->_42*b->_23 + a->_43*b->_33 + a->_44*b->_43;   \
   res->_44 = a->_41*b->_14 + a->_42*b->_24 + a->_43*b->_34 + a->_44*b->_44;
//---------------------------------------------------------------------
// result = a*b
// result is the same as a or b
//
void MatrixProduct2(D3DMATRIX *a, D3DMATRIX *b,D3DMATRIX *result)
{
   D3DMATRIX res;
   MATRIX_PRODUCT((&res), a, b);
   *(D3DMATRIX*)result = res;
}
//---------------------------------------------------------------------
// result = a*b.
// "result" pointer  could be equal to "a" or "b"
//
void MatrixProduct(D3DMATRIX *a, D3DMATRIX *b, D3DMATRIX *result)
{
   if (result == a || result == b) {
      MatrixProduct2(result, a, b);
      return;
   }
   MATRIX_PRODUCT(result, a, b);
}

void Normalize(D3DVECTOR *vIn, D3DVECTOR *vOut)
{
   // float mag = (float)sqrt((double)vIn->x * (double)vIn->x 
   // + (double)vIn->y * (double)vIn->y + (double)vIn->z * (double)vIn->z);
   // use a psedo magnitude since we can't use the math library

   float mag = 0.0f;

   if(vIn->x > 0.0f)
      mag += vIn->x;
   else
      mag -= vIn->x;

   if(vIn->y > 0.0f)
      mag += vIn->y;
   else
      mag -= vIn->y;

   if(vIn->z > 0.0f)
      mag += vIn->z;
   else
      mag -= vIn->z;

   vOut->x = vIn->x / mag;
   vOut->y = vIn->y / mag;
   vOut->z = vIn->z / mag;
}

//---------------------------------------------------------------------
// xform
// 
//
void VectorXForm (D3DMATRIX *mat, GEVECTOR4 *vIn, GEVECTOR4 *vOut)
{
    D3DVALUE   newx,newy,newz,neww;

    newx = (vIn->x * mat->_11) + 
           (vIn->y * mat->_21) + 
           (vIn->z * mat->_31) +
           (vIn->w * mat->_41);
    newy = (vIn->x * mat->_12) +  
           (vIn->y * mat->_22) +  
           (vIn->z * mat->_32) +
           (vIn->w * mat->_42);
    newz = (vIn->x * mat->_13) +  
           (vIn->y * mat->_23) +  
           (vIn->z * mat->_33) +
           (vIn->w * mat->_43);
    neww = (vIn->x * mat->_14) +  
           (vIn->y * mat->_24) +  
           (vIn->z * mat->_34) +
           (vIn->w * mat->_44);

    vOut->x = newx;
    vOut->y = newy;
    vOut->z = newz;
    vOut->w = neww;
}

#define PI              (3.1415926535897932384f)
#define HALFPI          (PI/2.f)
#define TWOPI           (PI*2.f)
#define TABLE_DEPTH     512
#define TABLE_SCALE     ((1.f/HALFPI) * (TABLE_DEPTH-1.f))
#include "gesintab.dat"

//---------------------------------------------------------------------
// We use table based sine and cosine functions rather than the FPU.
// We are acurate to about .003 radians

float fxSin(float _rad)
{
    float neg;
    int index;
    float rad = _rad;

    while (rad < 0.f || rad >= TWOPI)
    {
        if (rad < 0.f)
            rad += TWOPI;
        else
            rad -= TWOPI;
    }    
    // Determine the quadrant
    if (rad <= HALFPI) {
        neg = 1.f;
    }
    else if (rad <= PI) {
        neg = 1.f;
        rad = PI - rad;
    }
    else if (rad <= (PI+HALFPI)) {
        neg = -1.f;
        rad = rad - PI;
    }
    else {
        neg = -1.f;
        rad = PI - (rad - PI);
    }
    index = float2int(rad * TABLE_SCALE);

    return (neg * SineTable[index]);
}
float fxCos(float rad)
{
    return (fxSin(rad+HALFPI));
}

