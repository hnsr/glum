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
#include <signal.h>

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <gdk/gdkkeysyms.h>

#include <IL/il.h>
#include <IL/ilu.h>

#include "glum.h"

// Make sure symbols are exported properly for signal autoconnect on Windows.
#ifdef G_OS_WIN32
    #define GLADE_CB __declspec(dllexport)
#else
    #define GLADE_CB
#endif

#define PROGRAMNAME "Glum"
#define VERSION     "0.2"
#define AUTHORS     { "Hans Nieser <h.nieser@xs4all.nl>", NULL }
#define COPYRIGHT   "Copyright (C) 2008 Hans Nieser"
#define WEBSITE     "http://www.aphax.nl"
#define COMMENTS    "Glum, a simple software 3D rasterizer."
#define LICENSE     "This program is free software; you can redistribute it and/or modify it under"\
    " the terms of the GNU General Public License as publi shed by the Free Software Foundation; e"\
    "ither version 2 of the License, or (at your option) any later version.\n\nThis program is dis"\
    "tributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the impli"\
    "ed warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Publ"\
    "ic License for more details.\n\nYou should have received a copy of the GNU General Public Lic"\
    "ense along with this program; if not, write to the Free Software Foundation, Inc., 51 Frankli"\
    "n Street, Fifth Floor, Boston, MA 02110-1301, USA."

// Exit after TIME_LIMIT seconds have passed, useful for benchmarking/profiling.
#define TIME_LIMIT 16.0

#define DEFAULT_PERSPECTIVE_NEAR    0.1f
#define DEFAULT_PERSPECTIVE_FAR     100.0f

/*
#define DEFAULT_FOV             90.0f
#define DEFAULT_CAM_PITCH       0.0f
#define DEFAULT_CAM_YAW         0.0f
#define DEFAULT_CAM_X           0.5f
#define DEFAULT_CAM_Y           0.5f
#define DEFAULT_CAM_Z           0.55f
*/

/*
#define DEFAULT_FOV             90.0f
#define DEFAULT_CAM_PITCH       0.069755f
#define DEFAULT_CAM_YAW         0.069651f
#define DEFAULT_CAM_X           0.503065f
#define DEFAULT_CAM_Y           0.418891f
#define DEFAULT_CAM_Z           0.593934f
*/

/*
// Oblique triangle angle; many but short scans
#define DEFAULT_FOV             25.5f
#define DEFAULT_CAM_PITCH       0.006594f
#define DEFAULT_CAM_YAW         1.370129f
#define DEFAULT_CAM_X           3.261746f
#define DEFAULT_CAM_Y           0.499221f
#define DEFAULT_CAM_Z           0.505290f
*/


// Triangle fills viewport
#define DEFAULT_FOV             59.0f
#define DEFAULT_CAM_PITCH       0.153140f
#define DEFAULT_CAM_YAW         0.296544f
#define DEFAULT_CAM_X           0.762409f
#define DEFAULT_CAM_Y           0.390223f
#define DEFAULT_CAM_Z           0.454588f


GladeXML *gxml;
Context *context;
Camera *camera;

GtkWidget *drawingarea_image;

static int exiting;
static int viewport_mapped;

static int moving_forward;
static int moving_back;
static int moving_left;
static int moving_right;
static int moving_up;
static int moving_down;

static int turning_left;
static int turning_right;
static int turning_up;
static int turning_down;

static float move_speed = 1.0f;
static float turn_speed = 1.0f;

static float aspect_ratio = 1.0f;
static float fov = DEFAULT_FOV;

#define NUM_TEX 4
Texture *textures[NUM_TEX];

static int cur_tex;

// Test triangles
float triangles[][36] = {
    #include "../data/test3.tris"
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// Util functions
////////////////////////////////////////////////////////////////////////////////////////////////////

/* Draw some stuff */
static void draw_scene(gdouble tdelta)
{
    glum_clear(context);

    glum_load_identity(context, GLUM_MODELVIEW);
    glum_camera_apply(context, camera);

    glum_draw_triangle(context, triangles[0]);
    glum_draw_triangle(context, triangles[1]);

    /*glum_push_matrix(context, GLUM_MODELVIEW);
        glum_translate(context, -1.0f, 0.0f, 0.0f);
        glum_draw_triangle(context, triangles[2]);
        glum_draw_triangle(context, triangles[3]);
    glum_pop_matrix(context, GLUM_MODELVIEW);*/

    //glum_draw_line(context, 31, 41, 146, 43);
}


/* Initialize rendering context. */
static void context_init(int width, int height)
{
    if (context) return;

    context = glum_context_new(width, height);
    
    textures[0] = glum_texture_from_file("../data/border.png");
    textures[1] = glum_texture_from_file("../data/grid.png");
    textures[2] = glum_texture_from_file("../data/gras.jpg");
    textures[3] = glum_texture_from_file("../data/hitch1.jpg");
    context->texture = textures[cur_tex];

    glum_set_clear_color(context, GLUM_COLOR(0,0,0,255));

    aspect_ratio = (float) width / (float) height;

    glum_perspective(context, fov, aspect_ratio, DEFAULT_PERSPECTIVE_NEAR, DEFAULT_PERSPECTIVE_FAR);

    camera = glum_camera_new(DEFAULT_CAM_X, DEFAULT_CAM_Y, DEFAULT_CAM_Z,
                             DEFAULT_CAM_PITCH, DEFAULT_CAM_YAW);
}


/* Initialize the UI. */
static void ui_init(void)
{
    GtkWidget *window_main;

    gxml = glade_xml_new("../data/glum.glade", NULL, NULL);

    if ( NULL == gxml ) {
        fprintf(stderr, "ERROR: Failed to load glum.glade\n");
        exit(EXIT_FAILURE);
    }

    glade_xml_signal_autoconnect(gxml);

    // Get handle to image drawingarea and match its size request to the current image size.
    drawingarea_image = glade_xml_get_widget(gxml, "drawingarea_image");
    g_assert( NULL != drawingarea_image );
    
    // Already rendering to an offscreen buffer so don't need GTK+'s double buffering (disabling it
    // gives a decent speed up).
    gtk_widget_set_double_buffered(drawingarea_image, FALSE);

    // Set window title.
    window_main = glade_xml_get_widget(gxml, "window_main");
    g_assert( NULL != window_main);
    gtk_window_set_title(GTK_WINDOW(window_main), PROGRAMNAME " " VERSION);
}


static void dump_matrices(void)
{
    glum_dump_matrix(context, GLUM_MODELVIEW);
    glum_dump_matrix(context, GLUM_PROJECTION);
    glum_dump_matrix(context, GLUM_VIEWPORT);
    printf("\n");
}


void sigint_handler(int sig)
{
    exiting = 1;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// UI callbacks
////////////////////////////////////////////////////////////////////////////////////////////////////

// FIXME: map callback isn't called anymore and I don't know why, should probably figure that out if
// I ever want to do anything with this code again. I put some workarounds in the expose/draw CBs
// and initialize the context from there
GLADE_CB gboolean drawingarea_image_map_event_cb(GtkWidget *widget, GdkEvent *event,
    gpointer user_data)
{
    printf("Viewport mapped (%dx%d).\n", widget->allocation.width, widget->allocation.height);

    context_init(widget->allocation.width, widget->allocation.height);

    viewport_mapped = 1;
    return FALSE;
}


GLADE_CB gboolean drawingarea_image_unmap_event_cb(GtkWidget *widget, GdkEvent *event,
    gpointer user_data)
{
    printf("Viewport unmapped.\n");
    viewport_mapped = 0;
    return FALSE;
}


GLADE_CB gboolean window_main_delete_event_cb(GtkWidget *widget, GdkEvent *event,
                                              gpointer user_data)
{
    exiting = 1;

    return TRUE;
}


GLADE_CB gboolean window_main_key_press_event_cb(GtkWidget *widget, GdkEventKey *event,
                                                 gpointer user_data)
{
    switch(event->keyval) {
        
        case GDK_y:
            if (context->fill_mode == GLUM_FILLMODE_TEXBILINEAR) {
                context->fill_mode = GLUM_FILLMODE_TEXNEAREST;

            } else if (context->fill_mode == GLUM_FILLMODE_TEXNEAREST) {
                context->fill_mode = GLUM_FILLMODE_COLOR;

            } else {
                context->fill_mode = GLUM_FILLMODE_TEXBILINEAR;
            }
            break;

        case GDK_h:
            cur_tex++;
            context->texture = textures[cur_tex%NUM_TEX];
            break;

        case GDK_x:
            printf("\n\nResetting camera.\n");
            // TODO:
            break;

        case GDK_p:
            printf("Camera info:\n");
            printf(" fov: %f pitch: %f yaw: %f\n", fov, camera->pitch, camera->yaw);
            printf(" position: %f, %f, %f\n", camera->position[0], camera->position[1],
                                              camera->position[2]);
            break;

        case GDK_t:
            fov += 0.5f;
            glum_load_identity(context, GLUM_PROJECTION);
            glum_perspective(context, fov, aspect_ratio, DEFAULT_PERSPECTIVE_NEAR,
                                                         DEFAULT_PERSPECTIVE_FAR);
            break;

        case GDK_g:
            fov -= 0.5f;
            glum_load_identity(context, GLUM_PROJECTION);
            glum_perspective(context, fov, aspect_ratio, DEFAULT_PERSPECTIVE_NEAR,
                                                         DEFAULT_PERSPECTIVE_FAR);
            break;

        case GDK_w:
            moving_forward = 1;
            break;

        case GDK_a:
            moving_left = 1;
            break;

        case GDK_s:
            moving_back = 1;
            break;

        case GDK_d:
            moving_right = 1;
            break;

        case GDK_r:
            moving_up = 1;
            break;

        case GDK_f:
            moving_down = 1;
            break;

        case GDK_Left:
            turning_left = 1;
            break;

        case GDK_Right:
            turning_right = 1;
            break;

        case GDK_Up:
            turning_up = 1;
            break;

        case GDK_Down:
            turning_down = 1;
            break;

        case GDK_m:
        case GDK_M:
            printf("\n\nDumping matrices:\n" );
            dump_matrices();
            return TRUE;
    }

    return FALSE;
}


GLADE_CB gboolean window_main_key_release_event_cb(GtkWidget *widget, GdkEventKey *event,
                                                   gpointer user_data)
{
    switch(event->keyval) {
        
        case GDK_w:
            moving_forward = 0;
            break;

        case GDK_a:
            moving_left = 0;
            break;

        case GDK_s:
            moving_back = 0;
            break;

        case GDK_d:
            moving_right = 0;
            break;

        case GDK_r:
            moving_up = 0;
            break;

        case GDK_f:
            moving_down = 0;
            break;

        case GDK_Left:
            turning_left = 0;
            break;

        case GDK_Right:
            turning_right = 0;
            break;

        case GDK_Up:
            turning_up = 0;
            break;

        case GDK_Down:
            turning_down = 0;
            break;

    }

    return FALSE;
}


GLADE_CB void drawingarea_image_expose_event_cb(GtkWidget *widget, GdkEventExpose *event,
                                                gpointer *user_data)
{
#ifndef SKIP_GTK
    static GdkGC *gc = NULL;
    
    context_init(widget->allocation.width, widget->allocation.height);
    viewport_mapped = 1;

    if (gc == NULL) {
        gc = gdk_gc_new(widget->window);
    }

    gdk_draw_rgb_32_image(widget->window, gc, 0, 0, context->viewport_width,
        context->viewport_height, GDK_RGB_DITHER_NONE, context->framebuffer,
        context->viewport_width*4);
#endif
}


GLADE_CB gboolean drawingarea_image_configure_event_cb(GtkWidget *widget, GdkEventConfigure *event,
                                                       gpointer user_data)
{
    context_init(widget->allocation.width, widget->allocation.height);
    viewport_mapped = 1;

    glum_context_resize(context, event->width, event->height);

    aspect_ratio = (float) event->width / (float) event->height;

    glum_load_identity(context, GLUM_PROJECTION);
    glum_perspective(context, fov, aspect_ratio, DEFAULT_PERSPECTIVE_NEAR, DEFAULT_PERSPECTIVE_FAR);
    glum_clear(context);

    return FALSE;
}


GLADE_CB void button_quit_clicked_cb(GtkButton *button, gpointer user_data)
{
    exiting = 1;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// Entry point
////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    GTimer *timer;
    int frame_count = 0;
    gdouble tlast = 0.0, tnow = 0.0, tdelta;
    //gint mouse_x, mouse_y;

    // Catch SIGINT so we get a chance to display FPS before exiting.
    signal(SIGINT, sigint_handler);

    // Initialize some libs.
    ilInit();
    iluInit();
    gtk_init(&argc, &argv);


    // Initialise GUI
    ui_init();

    timer = g_timer_new();

    while (1) {

        if (viewport_mapped) {

            tnow = g_timer_elapsed(timer, NULL);
            tdelta = tnow-tlast;
            tlast = tnow;

            //gtk_widget_get_pointer(drawingarea_image, &mouse_x, &mouse_y);

            if (moving_forward)
                glum_camera_move(camera, GLUM_FORWARD, GLUM_CAMERA_RELATIVE, move_speed*tdelta);
            if (moving_back) 
                glum_camera_move(camera, GLUM_BACK, GLUM_CAMERA_RELATIVE, move_speed*tdelta);
            if (moving_left)
                glum_camera_move(camera, GLUM_LEFT, GLUM_CAMERA_RELATIVE, move_speed*tdelta);
            if (moving_right)
                glum_camera_move(camera, GLUM_RIGHT, GLUM_CAMERA_RELATIVE, move_speed*tdelta);
            if (moving_up)
                glum_camera_move(camera, GLUM_UP, GLUM_CAMERA_RELATIVE, move_speed*tdelta);
            if (moving_down)
                glum_camera_move(camera, GLUM_DOWN, GLUM_CAMERA_RELATIVE, move_speed*tdelta);
            if (turning_left)
                glum_camera_turn(camera, GLUM_LEFT, turn_speed*tdelta);
            if (turning_right)
                glum_camera_turn(camera, GLUM_RIGHT, turn_speed*tdelta);
            if (turning_up) 
                glum_camera_turn(camera, GLUM_UP, turn_speed*tdelta);
            if (turning_down)
                glum_camera_turn(camera, GLUM_DOWN, turn_speed*tdelta);

            draw_scene(tdelta);
            frame_count++;

#ifndef SKIP_GTK
            // Send expose for drawingarea.
            gtk_widget_queue_draw(drawingarea_image);
#endif
        }

        // Run GTK+ events (non-blocking).
        while (gtk_events_pending()) gtk_main_iteration_do(FALSE);

#ifdef TIME_LIMIT
            if (tnow >= TIME_LIMIT) exiting = 1;
#endif

        if (exiting) {
            printf("Exiting.\n");
            break;
        }

    }

    tnow = g_timer_elapsed(timer, NULL);

    printf("%d frames rendered in %f seconds (%f frames per second).\n", frame_count, tnow,
        ((gdouble) frame_count)/tnow);

    return EXIT_SUCCESS;
}

