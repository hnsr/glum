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

#ifndef __texture_h__
#define __texture_h__

typedef struct Texture
{
    unsigned int width;
    unsigned int height;

    float width_f;
    float height_f;

    // Optimised power-of-two modulus mask
    unsigned int width_mod_mask;
    unsigned int height_mod_mask;

    unsigned char *image;

} Texture;

Texture *glum_texture_from_file(char *filename);
void glum_texture_free(Texture *texture);

#endif

