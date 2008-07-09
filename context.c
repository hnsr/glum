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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "glum.h"
#include "math.h"


#define DEFAULT_DRAW_COLOR  0xFFFFFFFF
#define DEFAULT_CLEAR_COLOR 0xFF000000


#define MAX_DEPTH 1.0f // Max depth value.
static void set_viewport_transform(float *m, unsigned int width, unsigned int height)
{
    float hwidth, hheight, hdepth;

    hwidth = (float) width/2.0f;
    hheight = (float) height/2.0f;
    hdepth = (float) MAX_DEPTH/2.0f;

    m[0]  = hwidth;
    m[1]  = 0.0f;
    m[2]  = 0.0f;
    m[3]  = 0.0f;

    m[4]  = 0.0f;
    m[5]  = -hheight;
    m[6]  = 0.0f;
    m[7]  = 0.0f;

    m[8]  = 0.0f;
    m[9]  = 0.0f;
    m[10] = hdepth;
    m[11] = 0.0f;

    m[12] = hwidth;
    m[13] = hheight;
    m[14] = hdepth;
    m[15] = 1.0f;
}


Context *glum_context_new(unsigned int width, unsigned int height)
{
    Context *new = malloc(sizeof(Context));

    assert(new != NULL);

    memset((void *) new, 0, sizeof(Context));

    assert(width > 0);
    assert(height > 0);

    new->viewport_width = width;
    new->viewport_height = height;

    new->fill_mode = GLUM_FILLMODE_TEXBILINEAR;

    new->draw_color = DEFAULT_DRAW_COLOR;
    new->clear_color = DEFAULT_CLEAR_COLOR;

    // Allocate framebuffer
    new->framebuffer = malloc(width*height*4);
    assert(new->framebuffer != NULL);


    // Setup modelview/projection/viewport matrices.
    memcpy((void *) new->modelview[0], (void *) identity_4, sizeof(float)*16);
    memcpy((void *) new->projection[0], (void *) identity_4, sizeof(float)*16);
    set_viewport_transform(new->viewport, width, height);

    return new;
}


void glum_context_resize(Context *context, int new_width, int new_height)
{
    assert(new_width > 0);
    assert(new_height > 0);

    free(context->framebuffer);

    context->framebuffer = malloc(new_width*new_height*4);
    assert(context->framebuffer != NULL);

    set_viewport_transform(context->viewport, new_width, new_height);

    context->viewport_width = new_width;
    context->viewport_height = new_height;
}


void glum_context_free(Context *context)
{
    assert(context != NULL);
    assert(context->framebuffer != NULL);

    free(context->framebuffer);
    free(context);
}

