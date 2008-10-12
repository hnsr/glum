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

#ifndef __context_h__
#define __context_h__

struct Texture;

typedef struct Context Context;

struct Context
{
    unsigned int viewport_width;
    unsigned int viewport_height;

    unsigned char *framebuffer;

    unsigned int clear_color;
    unsigned int draw_color;

    unsigned int fill_mode;
    struct Texture *texture; // Currently active texture.

    float modelview[GLUM_MODELVIEW_STACK_SIZE][16];
    float projection[GLUM_PROJECTION_STACK_SIZE][16];
    float viewport[16];

    int modelview_stackpos;
    int projection_stackpos;

    int error_set;
    char *error_str;
};

Context *glum_context_new(unsigned int width, unsigned int height);
void glum_context_resize(Context *context, int new_width, int new_height);
void glum_context_free(Context *context);

#endif
