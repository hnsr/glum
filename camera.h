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

#ifndef __camera_h__
#define __camera_h__

struct Context;

typedef struct Camera
{
    float position[3];
    float pitch;
    float yaw;

    float max_pitch;

} Camera;

Camera *glum_camera_new(float pos_x, float pos_y, float pos_z, float pitch, float yaw);
void glum_camera_destroy(Camera *camera);

void glum_camera_lookat(Camera *camera, float x, float y, float z);
void glum_camera_position(Camera *camera, float x, float y, float z);

void glum_camera_move(Camera *camera, int direction, int mode, float dist);
void glum_camera_turn(Camera *camera, int direction, float angle);

void glum_camera_apply(struct Context *context, Camera *camera);

#endif
