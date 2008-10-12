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

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "glum.h"
#include "math.h"

#define DEFAULT_MAX_PITCH M_PI/2.0f


Camera *glum_camera_new(float pos_x, float pos_y, float pos_z, float pitch, float yaw)
{
    Camera *camera = malloc(sizeof(Camera));

    assert( NULL != camera );

    memset((void *) camera, 0, sizeof(Camera));

    camera->position[0] = pos_x;
    camera->position[1] = pos_y;
    camera->position[2] = pos_z;

    camera->pitch = pitch;
    camera->yaw = yaw;

    camera->max_pitch = DEFAULT_MAX_PITCH;

    return camera;
}


void glum_camera_destroy(Camera *camera)
{
    assert( NULL != camera );

    free(camera);
}


void glum_camera_lookat(Camera *camera, float x, float y, float z)
{
    // TODO, set angle/pitch based on (lookat - position).
}


void glum_camera_position(Camera *camera, float x, float y, float z)
{
    camera->position[0] = x;
    camera->position[1] = y;
    camera->position[2] = z;
}


void glum_camera_move(Camera *camera, int direction, int mode, float dist)
{
    if (mode == GLUM_CAMERA_RELATIVE) {

        float cosy, cosp, siny, sinp;
        float d[3];

        // Pick direction vector based on direction, then transform by pitch, yaw.

        // Setup displacement vector
        d[0] = d[1] = d[2] = 0.0f;

        switch (direction) {

            case GLUM_UP:
                d[1] = dist;
                break;
            case GLUM_DOWN:
                d[1] = -dist;
                break;
            case GLUM_LEFT:
                d[0] = -dist;
                break;
            case GLUM_RIGHT:
                d[0] = dist;
                break;
            case GLUM_FORWARD:
                d[2] = -dist;
                break;
            case GLUM_BACK:
                d[2] = dist;
                break;
            default:
                assert(0 && "Invalid direction given.");
        }

        // Transform displacement vector and add to position
        cosy = cosf(camera->yaw);
        siny = sinf(camera->yaw);
        cosp = cosf(camera->pitch);
        sinp = sinf(camera->pitch);

        camera->position[0] +=    cosy*d[0] + siny*sinp*d[1] + siny*cosp*d[2];
        camera->position[1] +=                     cosp*d[1] +   (-sinp)*d[2];
        camera->position[2] += (-siny)*d[0] + cosy*sinp*d[1] + cosy*cosp*d[2];


    } else if (mode == GLUM_CAMERA_ABSOLUTE) {

        switch (direction) {

            case GLUM_UP:
                camera->position[1] += dist;
                break;
            case GLUM_DOWN:
                camera->position[1] -= dist;
                break;
            case GLUM_LEFT:
                camera->position[0] -= dist;
                break;
            case GLUM_RIGHT:
                camera->position[0] += dist;
                break;
            case GLUM_FORWARD:
                camera->position[2] -= dist;
                break;
            case GLUM_BACK:
                camera->position[2] += dist;
                break;
            default:
                assert(0 && "Invalid direction given.");
        }

    } else {

        assert( 0 && "Invalid camera mode specified.");
    }
}


void glum_camera_turn(Camera *camera, int direction, float angle)
{
    switch (direction) {

        case GLUM_UP:
            camera->pitch += angle;
            break;
        case GLUM_DOWN:
            camera->pitch -= angle;
            break;
        case GLUM_LEFT:
            camera->yaw += angle;
            break;
        case GLUM_RIGHT:
            camera->yaw -= angle;
            break;

        default:
            assert(0 && "Invalid direction given.");
    }

    // Clamp to max_pitch
    if (camera->pitch > camera->max_pitch)
        camera->pitch = camera->max_pitch;
    else if ( camera->pitch < -(camera->max_pitch) )
        camera->pitch = -(camera->max_pitch);
}


void glum_camera_apply(struct Context *context, Camera *camera)
{
    float cosy, cosp, siny, sinp;
    float xt, yt, zt;
    float m[16];

    // Construct world->view matrix from position/pitch/yaw
    cosy = cosf(-(camera->yaw));
    siny = sinf(-(camera->yaw));
    cosp = cosf(-(camera->pitch));
    sinp = sinf(-(camera->pitch));

    xt = -(camera->position[0]);
    yt = -(camera->position[1]);
    zt = -(camera->position[2]);

    m[0] = cosy;            m[4] = 0.0f;  m[8] = siny;         m[12] = cosy*xt + siny*zt;
    m[1] = (-sinp)*(-siny); m[5] = cosp;  m[9] = (-sinp)*cosy; m[13] = (-sinp)*(-siny)*xt + cosp*yt + (-sinp)*cosy*zt;
    m[2] = cosp*(-siny);    m[6] = sinp; m[10] = cosp*cosy;    m[14] = cosp*(-siny)*xt + sinp*yt + cosp*cosy*zt;
    m[3] = 0.0f;            m[7] = 0.0f; m[11] = 0.0f;         m[15] = 1.0f;

    // Multiply with current modelview matrix
    glum_mult_matrix(context, GLUM_MODELVIEW, m);
}


