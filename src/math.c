/*
 *  This program is free software; you can redistribute it and/or modify it under the terms of the
 *  GNU General Public License as published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 * Copyright © 2007 Hans Nieser
 */

#include <string.h>

#include "math.h"

float identity_4[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
                         0.0f, 1.0f, 0.0f, 0.0f,
                         0.0f, 0.0f, 1.0f, 0.0f,
                         0.0f, 0.0f, 0.0f, 1.0f };


/* Multiply matrix A with B, and store result AB in A. */
void math_mult_m44_2(float *a, float *b)
{
    float r[16];

    r[0]  = a[0]*b[0]  + a[4]*b[1]  + a[8] *b[2]  + a[12]*b[3];
    r[4]  = a[0]*b[4]  + a[4]*b[5]  + a[8] *b[6]  + a[12]*b[7];
    r[8]  = a[0]*b[8]  + a[4]*b[9]  + a[8] *b[10] + a[12]*b[11];
    r[12] = a[0]*b[12] + a[4]*b[13] + a[8] *b[14] + a[12]*b[15]; 
    r[1]  = a[1]*b[0]  + a[5]*b[1]  + a[9] *b[2]  + a[13]*b[3];
    r[5]  = a[1]*b[4]  + a[5]*b[5]  + a[9] *b[6]  + a[13]*b[7];
    r[9]  = a[1]*b[8]  + a[5]*b[9]  + a[9] *b[10] + a[13]*b[11];
    r[13] = a[1]*b[12] + a[5]*b[13] + a[9] *b[14] + a[13]*b[15]; 
    r[2]  = a[2]*b[0]  + a[6]*b[1]  + a[10]*b[2]  + a[14]*b[3];
    r[6]  = a[2]*b[4]  + a[6]*b[5]  + a[10]*b[6]  + a[14]*b[7];
    r[10] = a[2]*b[8]  + a[6]*b[9]  + a[10]*b[10] + a[14]*b[11];
    r[14] = a[2]*b[12] + a[6]*b[13] + a[10]*b[14] + a[14]*b[15]; 
    r[3]  = a[3]*b[0]  + a[7]*b[1]  + a[11]*b[2]  + a[15]*b[3];
    r[7]  = a[3]*b[4]  + a[7]*b[5]  + a[11]*b[6]  + a[15]*b[7];
    r[11] = a[3]*b[8]  + a[7]*b[9]  + a[11]*b[10] + a[15]*b[11];
    r[15] = a[3]*b[12] + a[7]*b[13] + a[11]*b[14] + a[15]*b[15];

    memcpy(a, r, sizeof(float)*16);
}


/* Multiply matrix A with B, and store result AB in dest. */
void math_mult_m44_3(float *dest, float *a, float *b)
{
    dest[0]  = a[0]*b[0]  + a[4]*b[1]  + a[8] *b[2]  + a[12]*b[3];
    dest[4]  = a[0]*b[4]  + a[4]*b[5]  + a[8] *b[6]  + a[12]*b[7];
    dest[8]  = a[0]*b[8]  + a[4]*b[9]  + a[8] *b[10] + a[12]*b[11];
    dest[12] = a[0]*b[12] + a[4]*b[13] + a[8] *b[14] + a[12]*b[15]; 
    dest[1]  = a[1]*b[0]  + a[5]*b[1]  + a[9] *b[2]  + a[13]*b[3];
    dest[5]  = a[1]*b[4]  + a[5]*b[5]  + a[9] *b[6]  + a[13]*b[7];
    dest[9]  = a[1]*b[8]  + a[5]*b[9]  + a[9] *b[10] + a[13]*b[11];
    dest[13] = a[1]*b[12] + a[5]*b[13] + a[9] *b[14] + a[13]*b[15]; 
    dest[2]  = a[2]*b[0]  + a[6]*b[1]  + a[10]*b[2]  + a[14]*b[3];
    dest[6]  = a[2]*b[4]  + a[6]*b[5]  + a[10]*b[6]  + a[14]*b[7];
    dest[10] = a[2]*b[8]  + a[6]*b[9]  + a[10]*b[10] + a[14]*b[11];
    dest[14] = a[2]*b[12] + a[6]*b[13] + a[10]*b[14] + a[14]*b[15]; 
    dest[3]  = a[3]*b[0]  + a[7]*b[1]  + a[11]*b[2]  + a[15]*b[3];
    dest[7]  = a[3]*b[4]  + a[7]*b[5]  + a[11]*b[6]  + a[15]*b[7];
    dest[11] = a[3]*b[8]  + a[7]*b[9]  + a[11]*b[10] + a[15]*b[11];
    dest[15] = a[3]*b[12] + a[7]*b[13] + a[11]*b[14] + a[15]*b[15];
}


/* Transform vector v by M and store result Mv in v. */
void math_transform_v4(float *m, float *v)
{
    float vt[4];

    vt[0] = m[0]*v[0] + m[4]*v[1] + m[8]*v[2]  + m[12]*v[3];
    vt[1] = m[1]*v[0] + m[5]*v[1] + m[9]*v[2]  + m[13]*v[3];
    vt[2] = m[2]*v[0] + m[6]*v[1] + m[10]*v[2] + m[14]*v[3];
    vt[3] = m[3]*v[0] + m[7]*v[1] + m[11]*v[2] + m[15]*v[3];

    memcpy(v, vt, sizeof(float)*4);
}

