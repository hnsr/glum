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

#ifndef __matrix_h__
#define __matrix_h__

struct Context;

void glum_load_matrix(struct Context *context, int matrix_type, float *m);
void glum_load_identity(struct Context *context, int matrix_type);
void glum_mult_matrix(struct Context *context, int matrix_type, float *m);

void glum_push_matrix(struct Context *context, int matrix_type);
void glum_pop_matrix(struct Context *context, int matrix_type);

void glum_translate(struct Context *context, float x, float y, float z);
void glum_rotate(struct Context *context, float x, float y, float z, float angle);
void glum_scale(struct Context *context, float x, float y, float z);

void glum_perspective(struct Context *context, float fovy, float aspect, float near, float far);

void glum_dump_matrix(struct Context *context, int matrix_type);

#endif
