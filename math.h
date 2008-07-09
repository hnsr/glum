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

#ifndef __math_h__
#define __math_h__

// Apparently M_PI is not part of standard but already defined on Windows anyway
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif
#define DEG_TO_RAD 0.017453293 // (1/360)2pi

#ifndef MIN
#define MIN(a, b) ( (a) < (b) ? (a) : (b) ) 
#endif

extern float identity_4[16];

void math_mult_m44_2(float *a, float *b);
void math_mult_m44_3(float *dest, float *a, float *b);
void math_transform_v4(float *m, float *v);

#endif
