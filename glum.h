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

#ifndef __glum_h__
#define __glum_h__


#define GLUM_MODELVIEW  1
#define GLUM_PROJECTION 2
#define GLUM_VIEWPORT   3

#define GLUM_MODELVIEW_STACK_SIZE 16
#define GLUM_PROJECTION_STACK_SIZE 2

// Camera directions
#define GLUM_UP         1
#define GLUM_DOWN       2
#define GLUM_LEFT       3
#define GLUM_RIGHT      4
#define GLUM_FORWARD    5
#define GLUM_BACK       6

// Camera movement modes
#define GLUM_CAMERA_RELATIVE 1
#define GLUM_CAMERA_ABSOLUTE 2

// Polygon filling modes
#define GLUM_FILLMODE_COLOR       1
#define GLUM_FILLMODE_TEXNEAREST  2
#define GLUM_FILLMODE_TEXBILINEAR 3

#define GLUM_COLOR(r,g,b,a) ((r)+((g)<<8)+((b)<<16)+((a)<<24))

#include "texture.h"
//#include "math.h" // Private, only used internally
#include "context.h"
#include "draw.h"
#include "matrix.h"
#include "camera.h"


#endif

