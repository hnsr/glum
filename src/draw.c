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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "glum.h"
#include "math.h"

#ifdef WIN32
    #define NO_ASM
#endif



// Types -------------------------------------------------------------------------------------------

// Transformed/processed triangle.
typedef struct Triangle
{
    float coords[3][4];
    float uv_coords[3][2];
    float colors[3][4];

    // Parameters for each vertex that need to be interpolated
    float zi[3];    // 1/z
    float uz[3];    // u/z
    float vz[3];    // v/z
    float cz[3][4]; // c/z

    // Gradients (change in a parameter as we step by 1 in screen space)
    float dzi_dx;    // d(1/z)/dx
    float duz_dx;    // d(u/z)/dx
    float dvz_dx;    // d(v/z)/dx
    float dcz_dx[4]; // d(c/z)/dx

    float dzi_dy;    // d(1/z)/dy
    float duz_dy;    // d(u/z)/dy
    float dvz_dy;    // d(v/z)/dy
    float dcz_dy[4]; // d(c/z)/dy

} Triangle;


// Holds some starting and stepping values for an edge as we step along it while scan-converting.
typedef struct Edge
{
    float x;      // Current x position
    float x_step; // How x changes everytime we move to the next scanline (dx/dy)

    int y;        // Current y position
    int height;   // Height of this edge (when height becomes 0 we stop stepping along it)

    float zi;
    float zi_step;

    float uz;
    float uz_step;

    float vz;
    float vz_step;

    float cz[4];
    float cz_step[4];

} Edge;




// Util functions ----------------------------------------------------------------------------------

static inline unsigned int pack_color(float r, float g, float b, float a)
{
    unsigned int color = 0;

    // TODO: Measure performance impact of MIN() here
    //color += ((unsigned int) (MIN(a, 1.0f) * 255.0f)) << 24;
    //color += ((unsigned int) (MIN(b, 1.0f) * 255.0f)) << 16;
    //color += ((unsigned int) (MIN(g, 1.0f) * 255.0f)) << 8;
    //color +=  (unsigned int) (MIN(r, 1.0f) * 255.0f);

    color += ((unsigned int) (a * 255.0f)) << 24;
    color += ((unsigned int) (b * 255.0f)) << 16;
    color += ((unsigned int) (g * 255.0f)) << 8;
    color +=  (unsigned int) (r * 255.0f);


    return color;
}


static inline unsigned int get_texel_nearest(Texture *texture, float u, float v)
{
    int x, y;
    
    // Optimised power-of-two modulus, 50% faster than normal modulus and only marginally slower
    // than no modulus at all. The unsigned int mask with a signed integer seems to work because
    // of two's complement (consider that u/v coords can be negative)
    // TODO: Optimise floor/cast (removing the floor doubles FPS).
    x = ( (int) floorf(u * texture->width_f) ) & texture->width_mod_mask;
    y = ( (int) floorf(v * texture->height_f) ) & texture->height_mod_mask;

    return ((unsigned int *)texture->image)[y*texture->width + x];
}


// Fetches 4 texels, (u,v), (u+1,v), (u,v+1) and (u+1,v+1) and stores them at samples.
static inline void get_texels_bilinear(Texture *texture, unsigned int *samples, float u, float v)
{
    int x, y, x1, y1;
    
    x = ((int) u) & texture->width_mod_mask;
    y = ((int) v) & texture->height_mod_mask;
    x1 = (x+1) & texture->width_mod_mask;
    y1 = (y+1) & texture->height_mod_mask;

    samples[0] = *(((unsigned int *)texture->image) + y*texture->width + x);
    samples[1] = *(((unsigned int *)texture->image) + y*texture->width + x1);
    samples[2] = *(((unsigned int *)texture->image) + y1*texture->width + x);
    samples[3] = *(((unsigned int *)texture->image) + y1*texture->width + x1);
}


// Scales packed RGBA colors.
static inline unsigned int scale_color(unsigned int scale, unsigned int color)
{
    unsigned int ag = (color & 0xFF00FF00) >> 8;
    unsigned int br = color & 0x00FF00FF;

    ag *= scale;
    br *= scale;

    br = br & 0xFF00FF00;
    ag = (ag >> 8) & 0x00FF00FF;

    return br | ag;
}


// Linearly interpolate dest/src using some two's complement trickery, store result in dest. This
// is actually slightly slower than just doing two scale_color()s.
static inline void lerp_colors(unsigned int scale, unsigned int *dst, unsigned int src)
{
    unsigned int dst_ag, dst_br,
                 src_ag, src_br,
                 tmp_ag, tmp_br;

    dst_ag = (*dst >> 8) & 0x00FF00FF;
    dst_br =        *dst & 0x00FF00FF;

    src_ag = (src >> 8) & 0x00FF00FF;
    src_br =        src & 0x00FF00FF;

    tmp_ag = ((src_ag - dst_ag) * scale) >> 8;
    tmp_br = ((src_br - dst_br) * scale) >> 8;
    
    tmp_ag = ((tmp_ag + dst_ag) << 8) & 0xFF00FF00;
    tmp_br = (tmp_br + dst_br) & 0x00FF00FF;

    *dst = tmp_ag | tmp_br;
}


static inline unsigned int get_texel_bilinear(Texture *texture, float u, float v)
{
    float u_floor, v_floor;
    unsigned int dist_h, dist_v;
    unsigned int s0,s1,s2,s3;
    int x, y, x1, y1;

    u = (u * texture->width_f) - 0.5f;
    v = (v * texture->height_f) - 0.5f;

    // TODO: Optimize floor (simply removing ceilf has made this code 3/4 times as fast, so should
    // see if I can make floor faster as well.
    u_floor = floorf(u);
    v_floor = floorf(v);

    dist_h = (unsigned int) ((u-u_floor) * 256.0f);
    dist_v = (unsigned int) ((v-v_floor) * 256.0f);

    x = ((int) u_floor) & texture->width_mod_mask;
    y = ((int) v_floor) & texture->height_mod_mask;
    x1 = (x+1)          & texture->width_mod_mask;
    y1 = (y+1)          & texture->height_mod_mask;

    s0 = *(((unsigned int *)texture->image) + y*texture->width + x);
    s1 = *(((unsigned int *)texture->image) + y*texture->width + x1);
    s2 = *(((unsigned int *)texture->image) + y1*texture->width + x);
    s3 = *(((unsigned int *)texture->image) + y1*texture->width + x1);

#if 0 // lerp_colors isn't actually faster...
    lerp_colors(dist_h, &samples[0], samples[1]);
    lerp_colors(dist_h, &samples[2], samples[3]);
    lerp_colors(dist_v, &samples[0], samples[2]);

    return samples[0];
#endif

    s0 = scale_color(256-dist_h, s0) + scale_color(dist_h, s1);
    s1 = scale_color(256-dist_h, s2) + scale_color(dist_h, s3);

    return scale_color(256-dist_v, s0) + scale_color(dist_v, s1);
}


#if 0 // Unused util funcs
static float clampf(float v, float min, float max)
{
    if (v < min) {
        v = min;
    } else if (v > max) {
        v = max;
    }

    return v;
}


static void dump_v(float *v)
{
    printf("vertex = (%f, %f, %f, %f)\n", v[0], v[1], v[2], v[3]);
}
#endif


// Projects a point in homogeneous space to the plane w=1
static inline int w_divide(float *v)
{
    if ( v[3] <= 0.001f ) return 0; // XXX Remove this once I have proper culling/clipping
    v[0] /= v[3];
    v[1] /= v[3];
    v[2] /= v[3];
    v[3] = 1.0f;
    return 1;
}


// Calculates initial and step values for x/y (and interpolants if its a left edge).
static void init_edge(Triangle *t, Edge *edge, int p0, int p1, int is_left)
{
    float y_offset; // Difference between top vertex Y and ceil(top vertex Y).

    // Calculate initial scanline (y) and number of scanlines (height) for this edge.
    edge->y      =  (int) ceilf(t->coords[p0][1]);
    edge->height = ((int) ceilf(t->coords[p1][1])) - edge->y;

    // Slope of this edge (change in x as we step one in y).
    edge->x_step  = (t->coords[p1][0] - t->coords[p0][0]) /
                    (t->coords[p1][1] - t->coords[p0][1]);

    // Prestep x and interpolants (since edge->y could be larger than coords[p0][1]).
    y_offset = edge->y - t->coords[p0][1];
    edge->x = t->coords[p0][0] + (edge->x_step * y_offset);

    // Since during span drawing interpolants are calculated from the left edge to the right edge
    // incrementally, we only need to initialise the starting/step values of the interpolants for
    // left edges.
    if (is_left) {

        edge->zi_step    = t->dzi_dy    + t->dzi_dx    * edge->x_step;
        edge->uz_step    = t->duz_dy    + t->duz_dx    * edge->x_step;
        edge->vz_step    = t->dvz_dy    + t->dvz_dx    * edge->x_step;
        edge->cz_step[0] = t->dcz_dy[0] + t->dcz_dx[0] * edge->x_step;
        edge->cz_step[1] = t->dcz_dy[1] + t->dcz_dx[1] * edge->x_step;
        edge->cz_step[2] = t->dcz_dy[2] + t->dcz_dx[2] * edge->x_step;
        edge->cz_step[3] = t->dcz_dy[3] + t->dcz_dx[3] * edge->x_step;

        edge->zi    = t->zi[p0]    + (edge->zi_step    * y_offset);
        edge->uz    = t->uz[p0]    + (edge->uz_step    * y_offset);
        edge->vz    = t->vz[p0]    + (edge->vz_step    * y_offset);
        edge->cz[0] = t->cz[p0][0] + (edge->cz_step[0] * y_offset);
        edge->cz[1] = t->cz[p0][1] + (edge->cz_step[1] * y_offset);
        edge->cz[2] = t->cz[p0][2] + (edge->cz_step[2] * y_offset);
        edge->cz[3] = t->cz[p0][3] + (edge->cz_step[3] * y_offset);
    }
}


// Figures out the triangle configuration by sorting vertices in y. Fills in the top_bottom,
// top_middle, middle_bottom edges and indicates which side the middle vertex is at through
// middle_is_left.
static int setup_edges(Triangle *t, Edge *top_bottom, Edge *top_middle,
                       Edge *middle_bottom)
{
    int top, middle, bottom, middle_is_left;

    float y0 = t->coords[0][1];
    float y1 = t->coords[1][1];
    float y2 = t->coords[2][1];

    // Because triangle vertices are ccw, sorting vertices by y gives the triangle layout \o/
    if (y0 < y1) {
        if (y0 < y2) {
            top = 0;
            if (y1 < y2) {
                // y0 < y1 < y2
                middle = 1;
                bottom = 2;
                middle_is_left = 1;
            } else {
                // y0 < y2 < y1
                middle = 2;
                bottom = 1;
                middle_is_left = 0;
            }
        } else {
            // y2 < y0 < y1
            top = 2;
            middle = 0;
            bottom = 1;
            middle_is_left = 1;
        }
    } else {
        if (y2 < y1) {
            // y2 < y1 < y0
            top = 2;
            middle = 1;
            bottom = 0;
            middle_is_left = 0;
        } else {
            top = 1;
            if (y0 < y2) {
                // y1 < y0 < y2
                middle = 0;
                bottom = 2;
                middle_is_left = 0;
            } else {
                // y1 < y2 < y0
                middle = 2;
                bottom = 0;
                middle_is_left = 1;
            }
        }
    }

    if (middle_is_left) {

        init_edge(t, top_bottom, top, bottom, 0);
        init_edge(t, top_middle, top, middle, 1);
        init_edge(t, middle_bottom, middle, bottom, 1);

    } else {

        init_edge(t, top_bottom, top, bottom, 1);
        init_edge(t, top_middle, top, middle, 0);
        init_edge(t, middle_bottom, middle, bottom, 0);
    }

    return middle_is_left;
}


// Fills part of a scanline that falls between the left edge and the right edge.
static void draw_scan(Context *context, Triangle *triangle, Edge *left, Edge *right)
{
    int count, start;
    float x_offset, left_x, right_x;
    float x_cut = 0.0f; // Length of scanline segment that cutoff if it fell outside the viewport
    float z, zi, uz, vz, cz[4];
    unsigned int *fb;

    left_x = left->x;
    right_x = right->x;

    // TODO: Some temporary bounds checking/clamping to make sure we dont write outside framebuffer
    // and segfault. This can be removed once clipping/culling is implemented.
    if (left_x < 0) {
        x_cut = -left_x;
        left_x = 0.0f;
    }
    if (right_x > ((float) context->viewport_width)) right_x = (float) context->viewport_width;
    if (left->y > ((int) context->viewport_height-1)) return;
    if (left->y < 0) return;

    start = (int) ceilf(left_x);
    count = ((int) ceilf(right_x)) - start;

    // Prestep our interpolants to account for the distance (x_offset) between the first pixel of
    // the scan we draw and the real intersection of the left edge with the scanline.
    x_offset = ((float) start) - left_x + x_cut;
    zi    = left->zi    + (triangle->dzi_dx    * x_offset);
    uz    = left->uz    + (triangle->duz_dx    * x_offset);
    vz    = left->vz    + (triangle->dvz_dx    * x_offset);
    cz[0] = left->cz[0] + (triangle->dcz_dx[0] * x_offset);
    cz[1] = left->cz[1] + (triangle->dcz_dx[1] * x_offset);
    cz[2] = left->cz[2] + (triangle->dcz_dx[2] * x_offset);
    cz[3] = left->cz[3] + (triangle->dcz_dx[3] * x_offset);

    fb = (unsigned int *) context->framebuffer;
    fb += ((int) context->viewport_width) * left->y;
    fb += start;

    if (count > 0) {

        // TODO: This code duplication is a bit inconvenient, but using a function pointer for
        // get_texel is too costly and the overhead of branching around get_texel is too big...
        if (context->fill_mode == GLUM_FILLMODE_TEXBILINEAR) {
            
            while (count--) {

                z = 1.0f/zi;
                *fb++ = get_texel_bilinear(context->texture, uz*z, vz*z);
                //*fb++ = *(context->texture->image);

                zi += triangle->dzi_dx;
                uz += triangle->duz_dx;
                vz += triangle->dvz_dx;
            }

        } else if (context->fill_mode == GLUM_FILLMODE_TEXNEAREST) {

            while (count--) {

                z = 1.0f/zi;
                *fb++ = get_texel_nearest(context->texture, uz*z, vz*z);
                //*fb++ = *(context->texture->image);

                zi += triangle->dzi_dx;
                uz += triangle->duz_dx;
                vz += triangle->dvz_dx;
            }
        } else if (context->fill_mode == GLUM_FILLMODE_COLOR) {

            while (count--) {

                z = 1.0f/zi;
                *fb++ = pack_color(cz[0]*z, cz[1]*z, cz[2]*z, cz[3]*z);

                zi    += triangle->dzi_dx;
                cz[0] += triangle->dcz_dx[0];
                cz[1] += triangle->dcz_dx[1];
                cz[2] += triangle->dcz_dx[2];
                cz[3] += triangle->dcz_dx[3];
            }
        }
    }
}


// Steps down an edge in screens-space (y=y+1), updating x and interpolants accordingly.
static inline void edge_step(Edge *edge)
{
    edge->x += edge->x_step;
    edge->y++;

    edge->zi    += edge->zi_step;
    edge->uz    += edge->uz_step;
    edge->vz    += edge->vz_step;
    edge->cz[0] += edge->cz_step[0];
    edge->cz[1] += edge->cz_step[1];
    edge->cz[2] += edge->cz_step[2];
    edge->cz[3] += edge->cz_step[3];
}


static void rasterize_triangle(Context *context, Triangle *t)
{
    Edge top_bottom,    // The 3 edges that form the triangle. While scan-converting, these are
         top_middle,    // stepped along through the left/right pointers.
         middle_bottom;

    Edge *left_edge,  // The edges these point to depend on triangle configuration and on which
         *right_edge; // half (upper or lower) of the triangle we are currently scan-converting.

    int height,
        middle_is_left; // On which side the top_middle and middle_bottom edges connect.


    // Figure out triangle configuration and setup edges.
    middle_is_left = setup_edges(t, &top_bottom, &top_middle, &middle_bottom);

    // Draw upper half
    height = top_middle.height;

    // Set up left/right pointers for upper half
    if (middle_is_left) {
        left_edge = &top_middle;
        right_edge = &top_bottom;
    } else {
        left_edge = &top_bottom;
        right_edge = &top_middle;
    }

    // Step each scanline along left and right edges.
    while (height--) {

        draw_scan(context, t, left_edge, right_edge);
        edge_step(left_edge);
        edge_step(right_edge);
    }

    // Same for lower half
    height = middle_bottom.height;

    // Change left/right pointers for lower half
    if (middle_is_left)
        left_edge  = &middle_bottom;
    else
        right_edge = &middle_bottom;

    while (height--) {

        draw_scan(context, t, left_edge, right_edge);
        edge_step(left_edge);
        edge_step(right_edge);
    }
}


// Initializes interpolants (zi/uz/vz for perspective texture mapping, cz for perspective colors)
// at each vertex.
static void setup_interpolants(Triangle *t)
{
    int i;

    for ( i = 0; i < 3; i++ ) {

        t->zi[i] = 1.0f/t->coords[i][2];
        t->uz[i] = t->uv_coords[i][0] * t->zi[i];
        t->vz[i] = t->uv_coords[i][1] * t->zi[i];

        t->cz[i][0] = t->colors[i][0] * t->zi[i];
        t->cz[i][1] = t->colors[i][1] * t->zi[i];
        t->cz[i][2] = t->colors[i][2] * t->zi[i];
        t->cz[i][3] = t->colors[i][3] * t->zi[i];
    }
}


// Calculate gradients for the triangle (gradients = the change in the various interpolants as we
// step one in y or x) for 1/z, u/z, v/z.
static void calculate_gradients(Triangle *t)
{
    float dxi, dyi;

    float x0_min_x2 = t->coords[0][0] - t->coords[2][0]; // x0-x2
    float x1_min_x2 = t->coords[1][0] - t->coords[2][0]; // x1-x2

    float y0_min_y2 = t->coords[0][1] - t->coords[2][1]; // y0-y2
    float y1_min_y2 = t->coords[1][1] - t->coords[2][1]; // y1-y2

    dxi = 1.0f/((x1_min_x2) * (y0_min_y2) -
                (x0_min_x2) * (y1_min_y2));
    dyi = -dxi;

    t->dzi_dx = dxi * ( (t->zi[1] - t->zi[2]) * (y0_min_y2) -
                        (t->zi[0] - t->zi[2]) * (y1_min_y2) );

    t->duz_dx = dxi * ( (t->uz[1] - t->uz[2]) * (y0_min_y2) -
                        (t->uz[0] - t->uz[2]) * (y1_min_y2) );

    t->dvz_dx = dxi * ( (t->vz[1] - t->vz[2]) * (y0_min_y2) -
                        (t->vz[0] - t->vz[2]) * (y1_min_y2) );

    
    t->dzi_dy = dyi * ( (t->zi[1] - t->zi[2]) * (x0_min_x2) -
                        (t->zi[0] - t->zi[2]) * (x1_min_x2) );

    t->duz_dy = dyi * ( (t->uz[1] - t->uz[2]) * (x0_min_x2) -
                        (t->uz[0] - t->uz[2]) * (x1_min_x2) );

    t->dvz_dy = dyi * ( (t->vz[1] - t->vz[2]) * (x0_min_x2) -
                        (t->vz[0] - t->vz[2]) * (x1_min_x2) );

    
    t->dcz_dx[0] = dxi * ( (t->cz[1][0] - t->cz[2][0]) * (y0_min_y2) -
                           (t->cz[0][0] - t->cz[2][0]) * (y1_min_y2) );

    t->dcz_dx[1] = dxi * ( (t->cz[1][1] - t->cz[2][1]) * (y0_min_y2) -
                           (t->cz[0][1] - t->cz[2][1]) * (y1_min_y2) );

    t->dcz_dx[2] = dxi * ( (t->cz[1][2] - t->cz[2][2]) * (y0_min_y2) -
                           (t->cz[0][2] - t->cz[2][2]) * (y1_min_y2) );

    t->dcz_dx[3] = dxi * ( (t->cz[1][3] - t->cz[2][3]) * (y0_min_y2) -
                           (t->cz[0][3] - t->cz[2][3]) * (y1_min_y2) );

    
    t->dcz_dy[0] = dyi * ( (t->cz[1][0] - t->cz[2][0]) * (x0_min_x2) -
                           (t->cz[0][0] - t->cz[2][0]) * (x1_min_x2) );

    t->dcz_dy[1] = dyi * ( (t->cz[1][1] - t->cz[2][1]) * (x0_min_x2) -
                           (t->cz[0][1] - t->cz[2][1]) * (x1_min_x2) );

    t->dcz_dy[2] = dyi * ( (t->cz[1][2] - t->cz[2][2]) * (x0_min_x2) -
                           (t->cz[0][2] - t->cz[2][2]) * (x1_min_x2) );
    
    t->dcz_dy[3] = dyi * ( (t->cz[1][3] - t->cz[2][3]) * (x0_min_x2) -
                           (t->cz[0][3] - t->cz[2][3]) * (x1_min_x2) );
}



// Public functions --------------------------------------------------------------------------------

void glum_set_clear_color(Context *context, unsigned int color)
{
    context->clear_color = color;
}



void glum_set_draw_color(struct Context *context, unsigned int color)
{
    context->draw_color = color;
}


#ifndef NO_ASM
void glum_clear_asm(unsigned int *fb, unsigned int pixels, unsigned int color);
#endif

void glum_clear(Context *context)
{
    unsigned int *fb;
    unsigned int pixels;
#ifdef NO_ASM
    unsigned int i = 0;
#endif

    pixels = context->viewport_width * context->viewport_height;

    fb = (unsigned int *) context->framebuffer;

#ifdef NO_ASM
    while (pixels--) {

        fb[i++] = context->clear_color;
    }
#else
    // Not much faster (but measurable on amd64 with a core2duo) if compiler unrolls above loop
    glum_clear_asm(fb, pixels, context->clear_color);
#endif

}



void glum_draw_triangle(Context *context, float *triangle)
{
    Triangle t;

    // Copy input triangle into local copy for transformation.
    memcpy(t.coords, triangle, sizeof(float)*3);
    memcpy(t.coords[1], &(triangle[3]), sizeof(float)*3);
    memcpy(t.coords[2], &(triangle[6]), sizeof(float)*3);
    
    memcpy(t.uv_coords, &(triangle[9]), sizeof(float)*2);
    memcpy(t.uv_coords[1], &(triangle[11]), sizeof(float)*2);
    memcpy(t.uv_coords[2], &(triangle[13]), sizeof(float)*2);

    t.coords[0][3] = 1.0f; // Now using homogeneous coordinates, so set w to 1.0f
    t.coords[1][3] = 1.0f;
    t.coords[2][3] = 1.0f;

    
    // Transform by modelview matrix -> view-space
    math_transform_v4(context->modelview[context->modelview_stackpos], t.coords[0]);
    math_transform_v4(context->modelview[context->modelview_stackpos], t.coords[1]);
    math_transform_v4(context->modelview[context->modelview_stackpos], t.coords[2]);


    // Do lighting
    // TODO
    // For now just copy colors over from input triangle.
    memcpy(t.colors,    &(triangle[24]), sizeof(float)*4);
    memcpy(t.colors[1], &(triangle[28]), sizeof(float)*4);
    memcpy(t.colors[2], &(triangle[32]), sizeof(float)*4);


    // Setup interpolants for texture mapping
    setup_interpolants(&t);


    // Clip by user-specified clipping planes
    // TODO


    // Transform by projection matrix -> clip-space
    math_transform_v4(context->projection[context->projection_stackpos], t.coords[0]);
    math_transform_v4(context->projection[context->projection_stackpos], t.coords[1]);
    math_transform_v4(context->projection[context->projection_stackpos], t.coords[2]);


    // Culling/Clipping by frustrum planes 
    // TODO


    // Divide by w -> NDC-space
    if ( !( w_divide(t.coords[0]) && w_divide(t.coords[1]) && w_divide(t.coords[2]) ) )
        // Discard triangle, w <= 0.0001 for one or more vertices.
        return;

    
    // Viewport transform -> screen-space
    math_transform_v4(context->viewport, t.coords[0]);
    math_transform_v4(context->viewport, t.coords[1]);
    math_transform_v4(context->viewport, t.coords[2]);


    // Calculate triangle gradients
    calculate_gradients(&t);


    // Rasterize
    rasterize_triangle(context, &t);
}

// Bresenham line drawing, this is a bit lame and should be replaced by something that handles
// all cases.
// TODO: do transformations
void glum_draw_line(struct Context *context, int x0, int y0, int x1, int y1)
{
    unsigned int *fb, color;
    int x, y;
    float error, deltaerr;

    color = context->draw_color;

    fb = (unsigned int *) context->framebuffer;

    error = 0.0f;
    deltaerr = (float) (y1 - y0) / (float) (x1 - x0);

    y = y0;

    for (x = x0; x < x1; x++) {

        fb[y*context->viewport_width+x] = color;
        error += deltaerr;

        if (abs(error) >= 0.5f) {
            y++;
            error -= 1.0f;
        }
    }
}

