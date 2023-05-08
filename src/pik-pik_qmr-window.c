// #############################################################################

/* pik-pik_qmr-window.c
 *
 * Copyright 2023 Valentin But
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

// #############################################################################

#include "config.h"

#include "pik-pik_qmr-window.h"


#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
#include <gudev/gudev.h>

struct _PikPikQmrWindow
{
    GtkApplicationWindow  parent_instance;

    /* Template widgets */

    //GtkHeaderBar        *header_bar;
    //GtkLabel            *label;

    //GtkMenuButton       *menubtn_file;
    //GtkMenuButton       *menubtn_tools;
    //GtkMenuButton       *menubtn_help;

    GtkDropDown         *dropdown_port;
    GtkStringList       *stringlist_ports;
    GtkButton           *button_reload;

};

G_DEFINE_FINAL_TYPE(PikPikQmrWindow, pik_pik_qmr_window, GTK_TYPE_APPLICATION_WINDOW)

// #############################################################################

static GList *pik_pik_list_serial( PikPikQmrWindow *wnd )
{
    GUdevClient *gudev_client = NULL;
    const gchar *subsystems[2] = { "drm", NULL };

    GUdevEnumerator *gudev_enum = NULL;

    GList *devlist = NULL;
    GList *deventry = NULL;

    // creating gudev client
    gudev_client = g_udev_client_new( subsystems );
    if( gudev_client == NULL )
    {
        // could not create gudev client
        goto cleanup;
    }

    // creating gudev enumerator
    gudev_enum = g_udev_enumerator_new( gudev_client );
    if( gudev_enum == NULL )
    {
        // could not create gudev enumerator
        goto cleanup;
    }

    // all returned devices will match the given subsystem
    g_udev_enumerator_add_match_subsystem( gudev_enum, "tty" );

    // executes the query in enumerator
    devlist = g_udev_enumerator_execute( gudev_enum );
    if( devlist == NULL )
    {
        goto cleanup;
    }

    for( deventry = devlist; deventry != NULL; deventry = deventry->next )
    {
        GUdevDevice *gudevice = G_UDEV_DEVICE(deventry->data);
        const gchar *devnode = g_udev_device_get_device_file( gudevice );

        if( devnode == NULL )
            continue;

        if(
            strncmp( devnode, "/dev/ttyACM", 11 ) != 0 &&
           strncmp( devnode, "/dev/ttyUSB", 11 ) != 0
        )
            continue;

        printf( "devnode = %s\n", devnode );

        //gtk_combo_box_text_append_text( combobox_port, devnode );

        // probe COM port
        //int fd = open( devnode, O_RDWR | O_NONBLOCK );
        //if( fd >= 0 )
        //{
        //    // get serial_info
        //    struct serial_struct serinfo;
        //    if( ioctl( fd, TIOCGSERIAL, &serinfo ) == 0 )
        //    {
        //        // if device type is no PORT_UNKNOWN we accept the port
        //        if( serinfo.type != PORT_UNKNOWN )
        //            printf( "devnode = %s; serinfo.type = %i\n", devnode, serinfo.type );
        //    }
        //
        //    close( fd );
        //}

        //drm_fd = open (devnode, O_RDWR | O_CLOEXEC);
        //if (drm_fd < 0) {
        //    GST_WARNING ("Cannot open device node \"%s\": %s (%d)", devnode, g_strerror (errno), errno);
        //    continue;
        //}
  }

cleanup:

    // clean up device list
    if( devlist != NULL )
    {
        g_list_free_full( devlist, g_object_unref );
        devlist = NULL;
    }

    // clean up gudev enumerator
    if( gudev_enum != NULL )
    {
        g_object_unref( G_OBJECT(gudev_enum) );
        gudev_enum = NULL;
    }

    // clean up gudev client
    if( gudev_client != NULL )
    {
        g_object_unref( G_OBJECT(gudev_client) );
        gudev_client = NULL;
    }

    return NULL;
}

// #############################################################################

static void button_reload_clicked( GtkButton *button, PikPikQmrWindow *wnd )
{
    printf("list of serial devices:\n");
    pik_pik_list_serial( wnd );
}

// #############################################################################

static void pik_pik_qmr_window_class_init( PikPikQmrWindowClass *klass )
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    // extend a widget with XML
    gtk_widget_class_set_template_from_resource( widget_class, "/ee/pik_pik/qmr/pik-pik_qmr-main.ui" );

    //gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, header_bar );
    //gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, label );

    //gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, menubtn_file );
    //gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, menubtn_tools );
    //gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, menubtn_help );

    // binds a child widget defined in a template to the widget_class
    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, dropdown_port );
    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, button_reload );
}

// #############################################################################

static void pik_pik_qmr_window_init( PikPikQmrWindow *self )
{
    // creates and initializes child widgets defined in templates
    gtk_widget_init_template( GTK_WIDGET(self) );

    //
    // connect GCallback functions to a signal of widgets
    //

    g_signal_connect( self->button_reload, "clicked", G_CALLBACK(button_reload_clicked), self );

    // initialize stringlist for ports
    const char *strings[] = { "test1", "test2", "test3", NULL };
    self->stringlist_ports = gtk_string_list_new( strings );

    // attach stringlist to the dropdown
    gtk_drop_down_set_model( self->dropdown_port, G_LIST_MODEL(self->stringlist_ports) );
}

// #############################################################################



