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
#include <limits.h>

#include <IL/il.h>
#include <IL/ilu.h>

#include "texture.h"

Texture *glum_texture_from_file(char *filename) {

    Texture  *tex = NULL;

    ILuint image_name;
    ILenum error;
    char   *error_str;
    ILint cur_format;
    unsigned int width, height;

    ilGenImages(1, &image_name);
    ilBindImage(image_name);

    //ilEnable(IL_ORIGIN_SET);
    //ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

    if (ilLoadImage(filename)) {

        cur_format = ilGetInteger(IL_IMAGE_FORMAT);

        if ( cur_format == IL_COLOR_INDEX ) {

            printf("DEBUG: %s: Image format for \"%s\" is COLOR_INDEX, converting to RGBA\n",
                __func__, filename);

            if ( !ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE) ) {

                printf("WARNING: %s: Failed to convert color indexed image \"%s\" to RGBA.\n",
                    __func__, filename);
                ilDeleteImages(1, &image_name);
                return NULL;
            }
        }

        width = ilGetInteger(IL_IMAGE_WIDTH);
        height = ilGetInteger(IL_IMAGE_WIDTH);

#ifndef IS_POT
#define IS_POT(x) ( ( (x) & ( (x)-1 ) ) == 0 )
#endif
        // Make sure width and height are > 0 and power-of-two.
        if ( !(width > 0 && height > 0 && IS_POT(width) && IS_POT(height)) ) {

            printf("WARNING: %s: Failed to load image \"%s\", invalid dimensions (zero or non-"
                "power-of-two).\n", __func__, filename);
            ilDeleteImages(1, &image_name);
            return NULL;
        }

        tex = (Texture *) malloc(sizeof(Texture));
        tex->width  = ilGetInteger(IL_IMAGE_WIDTH);
        tex->height = ilGetInteger(IL_IMAGE_HEIGHT);
        tex->image  = (unsigned char *) malloc(tex->width * tex->height * 4);

        if( !ilCopyPixels(0, 0, 0, tex->width, tex->height, 1, IL_RGBA, IL_UNSIGNED_BYTE,
                          tex->image) ) {

            error = ilGetError();
            error_str = iluErrorString(error);
            printf("WARNING: %s: Failed to read image data for \"%s\". (Last error given by DevIL"
                " was \"%s (code %d)\".)\n", __func__, filename, error_str, error);
            free( (void *) tex->image );
            free(tex);
            ilDeleteImages(1, &image_name);
            return NULL;
        }

    } else {

        printf("WARNING: %s: Failed to load image \"%s\".\n", __func__, filename);
        ilDeleteImages(1, &image_name);
        return NULL;
    }

    ilDeleteImages(1, &image_name);

    tex->width_f  = (float) (tex->width);
    tex->height_f = (float) (tex->height);
    tex->width_mod_mask  = ~(UINT_MAX << (unsigned int) log2((double) tex->width));
    tex->height_mod_mask = ~(UINT_MAX << (unsigned int) log2((double) tex->height));

    return tex;
}

void glum_texture_free(Texture *texture)
{
    assert(texture != NULL);
    assert(texture->image != NULL);

    free(texture->image);
    free(texture);
}

