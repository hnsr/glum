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

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include "glum.h"
#include "math.h"


void glum_load_matrix(Context *context, int matrix_type, float *m)
{
    if (matrix_type == GLUM_MODELVIEW)
        memcpy(context->modelview[context->modelview_stackpos], m, sizeof(float)*16);
    else if (matrix_type == GLUM_PROJECTION)
        memcpy(context->projection[context->projection_stackpos], m, sizeof(float)*16);
    else
        assert(0 && "Incorrect matrix_type.");
}


void glum_load_identity(Context *context, int matrix_type)
{
    if (matrix_type == GLUM_MODELVIEW)
        memcpy(context->modelview[context->modelview_stackpos], identity_4, sizeof(float)*16);
    else if (matrix_type == GLUM_PROJECTION)
        memcpy(context->projection[context->projection_stackpos], identity_4, sizeof(float)*16);
    else
        assert(0 && "Incorrect matrix_type.");
}


void glum_mult_matrix(Context *context, int matrix_type, float *m)
{
    // R = result, C = current, M = supplied 
    // R = CM
    float *c;
   
    if (matrix_type == GLUM_MODELVIEW) {
        c = context->modelview[context->modelview_stackpos];
    } else if (matrix_type == GLUM_PROJECTION) {
        c = context->projection[context->projection_stackpos];
    } else {
        assert(0 && "incorrect matrix_type.");
    }

    math_mult_m44_2(c, m);
}


void glum_push_matrix(Context *context, int matrix_type)
{
    if (matrix_type == GLUM_MODELVIEW) {

        if (context->modelview_stackpos < GLUM_MODELVIEW_STACK_SIZE-1) {

            memcpy(context->modelview[context->modelview_stackpos+1],
                   context->modelview[context->modelview_stackpos], sizeof(float)*16);
            context->modelview_stackpos++;
        } else {
            context->error_set = 1;
            context->error_str = "Pushed off of modelview matrix stack.";
        }

    } else if (matrix_type == GLUM_PROJECTION) {

        if (context->projection_stackpos < GLUM_PROJECTION_STACK_SIZE-1) {

            memcpy(context->projection[context->projection_stackpos+1],
                   context->projection[context->projection_stackpos], sizeof(float)*16);
            context->projection_stackpos++;
        } else {
            context->error_set = 1;
            context->error_str = "Pushed off of projection matrix stack.";
        }

    } else {

        assert(0 && "Incorrect matrix_type.");
    }
}


void glum_pop_matrix(Context *context, int matrix_type)
{
    if (matrix_type == GLUM_MODELVIEW) {

        if (context->modelview_stackpos > 0) {

            context->modelview_stackpos--;

        } else {
            context->error_set = 1;
            context->error_str = "Tried to pop modelview matrix stack while already at bottom.";
        }

    } else if (matrix_type == GLUM_PROJECTION) {

        if (context->projection_stackpos > 0) {

            context->projection_stackpos--;
        } else {
            context->error_set = 1;
            context->error_str = "Tried to pop projection matrix stack while already at bottom.";
        }

    } else {

        assert(0 && "Incorrect matrix_type.");
    }
}


void glum_translate(Context *context, float x, float y, float z)
{
    float *m;

    m = context->modelview[context->modelview_stackpos];

    // Simplfified translation
    m[12] += m[0]*x + m[4]*y +  m[8]*z;
    m[13] += m[1]*x + m[5]*y +  m[9]*z;
    m[14] += m[2]*x + m[6]*y + m[10]*z;
    m[15] += m[3]*x + m[7]*y + m[11]*z;
}


void glum_rotate(Context *context, float x, float y, float z, float angle)
{
    float *m;
    float r[9]; // Upper 3x3 matrix of the rotation

    float c, s, t, tx;
    float t1, t2, t3; // Temp storage

    m = context->modelview[context->modelview_stackpos];

    // Calculate upper 3x3 rotation matrix.
    c = cosf(angle);
    s = sinf(angle);
    t = 1.0f - c;
    tx = t*x;

    r[0] = (tx*x)+c;     r[3] = (tx*y)-(s*z);  r[6] = (tx*z)+(s*y);
    r[1] = (tx*y)+(s*z); r[4] = (t*y*y)+c;     r[7] = (t*y*z)-(s*x);
    r[2] = (tx*z)-(s*y); r[5] = (t*y*z)+(s*x); r[8] = (t*z*z)+c;

    // --
    t1 = m[0]*r[0] + m[4]*r[1] + m[8]*r[2];
    t2 = m[0]*r[3] + m[4]*r[4] + m[8]*r[5];
    t3 = m[0]*r[6] + m[4]*r[7] + m[8]*r[8];
    m[0] = t1; m[4] = t2; m[8] = t3;

    t1 = m[1]*r[0] + m[5]*r[1] + m[9]*r[2];
    t2 = m[1]*r[3] + m[5]*r[4] + m[9]*r[5];
    t3 = m[1]*r[6] + m[5]*r[7] + m[9]*r[8];
    m[1] = t1; m[5] = t2; m[9] = t3;

    t1 = m[2]*r[0] + m[6]*r[1] + m[10]*r[2];
    t2 = m[2]*r[3] + m[6]*r[4] + m[10]*r[5];
    t3 = m[2]*r[6] + m[6]*r[7] + m[10]*r[8];
    m[2] = t1; m[6] = t2; m[10] = t3;

    t1 = m[3]*r[0] + m[7]*r[1] + m[11]*r[2];
    t2 = m[3]*r[3] + m[7]*r[4] + m[11]*r[5];
    t3 = m[3]*r[6] + m[7]*r[7] + m[11]*r[8];
    m[3] = t1; m[7] = t2; m[11] = t3;
}


void glum_scale(Context *context, float x, float y, float z)
{
    float *m;

    m = context->modelview[context->modelview_stackpos];

    m[0] *= x; m[4] *= y; m[8]  *= z;
    m[1] *= x; m[5] *= y; m[9]  *= z;
    m[2] *= x; m[6] *= y; m[10] *= z;
    m[3] *= x; m[7] *= y; m[11] *= z; // TODO: Should be able to skip 3/7/11
}


void glum_perspective(struct Context *context, float fovy, float aspect, float near, float far)
{
    float *m;

    float d, da, A, B, tmp;

    // Convert degrees to radians.
    fovy *= DEG_TO_RAD;
    m = context->projection[context->projection_stackpos];
    
    d = 1.0f/tanf(fovy/2.0f);
    da = d/aspect;
    A = (near+far)/(near-far);
    B = (2.0f*near*far)/(near-far);

    m[0] *= da;
    m[4] *= d;
    tmp = (m[8]*A) + m[12]*-1.0f;
    m[12] = m[8]*B;
    m[8] = tmp;

    m[1] *= da;
    m[5] *= d;
    tmp = (m[9]*A) + m[13]*-1.0f;
    m[13] = m[9]*B;
    m[9] = tmp;

    m[2] *= da;
    m[6] *= d;
    tmp = (m[10]*A) + m[14]*-1.0f;
    m[14] = m[10]*B;
    m[10] = tmp;

    m[3] *= da;
    m[7] *= d;
    tmp = (m[11]*A) + m[15]*-1.0f;
    m[15] = m[11]*B;
    m[11] = tmp;
}


static void dump_matrix(float *m)
{    
    printf("  %f %f %f %f\n", m[0], m[4], m[8],  m[12]);
    printf("  %f %f %f %f\n", m[1], m[5], m[9],  m[13]);
    printf("  %f %f %f %f\n", m[2], m[6], m[10], m[14]);
    printf("  %f %f %f %f\n\n", m[3], m[7], m[11], m[15]);
}


void glum_dump_matrix(struct Context *context, int matrix_type)
{
    if (matrix_type == GLUM_MODELVIEW) {

        printf("Current modelview matrix:\n");
        dump_matrix(context->modelview[context->modelview_stackpos]);

    } else if (matrix_type == GLUM_PROJECTION) {

        printf("Current projection matrix:\n");
        dump_matrix(context->projection[context->projection_stackpos]);

    } else if (matrix_type == GLUM_VIEWPORT) {

        printf("Current viewport matrix:\n");
        dump_matrix(context->viewport);

    } else {

        assert(0 && "Incorrect matrix_type.");
    }
}

