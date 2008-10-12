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

#ifndef __draw_h__
#define __draw_h__

struct Context;

void glum_set_clear_color(struct Context *context, unsigned int color);
void glum_set_draw_color(struct Context *context, unsigned int color);
void glum_clear(struct Context *context);
void glum_draw_triangle(struct Context *context, float *triangle);
void glum_draw_line(struct Context *context, int x1, int y1, int x2, int y2);

#endif
