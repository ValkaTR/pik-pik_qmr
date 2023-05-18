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

// for COM port
#include <glib.h>
#include <glib/gstdio.h>
#include <gudev/gudev.h>
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

// for GtkStringList of ports
#include <glib.h>

enum PIK_PIK_STATE
{
    PIK_PIK_STATE_NONE = 0,
    PIK_PIK_STATE_DISCONNECTED,
    PIK_PIK_STATE_CONNECTED,
    PIK_PIK_STATE_WIP
};

static void pik_pik_set_state( PikPikQmrWindow *wnd, enum PIK_PIK_STATE state );

struct _PikPikQmrWindow
{
    GtkApplicationWindow  parent_instance;

    /* Template widgets */
    GtkLabel            *label_port;
    GtkDropDown         *dropdown_port;
    GtkStringList       *stringlist_ports;
    GtkEntry            *entry_port_other;

    GtkButton           *button_reload;
    GtkButton           *button_connect;
    GtkButton           *button_disconnect;
    GtkButton           *button_poke;
    GtkButton           *button_read;
    GtkButton           *button_write;
    GtkButton           *button_verify;

    GtkLabel            *label_password;
    GtkPasswordEntry    *entry_password;

    char                selected_port[64];      // contains path to COM port
    int                 serial_port;            // serial port file descriptor

    enum PIK_PIK_STATE  state;

    GString             *serial_line;
};

G_DEFINE_FINAL_TYPE(PikPikQmrWindow, pik_pik_qmr_window, GTK_TYPE_APPLICATION_WINDOW)

// port selection functions
static void pik_pik_get_selected_port( PikPikQmrWindow *wnd );
static void pik_pik_clear_ports( PikPikQmrWindow *wnd );
static GList *pik_pik_list_serial( PikPikQmrWindow *wnd );
static void pik_pik_reload_ports( PikPikQmrWindow *wnd );
static void pik_pik_check_other_port_visibility( PikPikQmrWindow *wnd );

// signals

static void pik_pik_dropdown_port_change( GtkDropDown *dropdown, GParamSpec *pspec, PikPikQmrWindow *wnd );
static void pik_pik_entry_port_other_change( GtkDropDown *dropdown, PikPikQmrWindow *wnd );

// buttons
static void pik_pik_button_connect_clicked( GtkButton *button, PikPikQmrWindow *wnd );
static void pik_pik_button_disconnect_clicked( GtkButton *button, PikPikQmrWindow *wnd );
static void pik_pik_button_reload_clicked( GtkButton *button, PikPikQmrWindow *wnd );

// auxiliary
static void pik_pik_errorbox( PikPikQmrWindow *wnd, char *filename );
static void pik_pik_messagebox( PikPikQmrWindow *wnd, const char *title, const char *fmt, ... ) __attribute__ ((format (printf, 3, 4)));

// io
static void pik_pik_parse_line( PikPikQmrWindow *wnd, GString *str );

// #############################################################################

static void pik_pik_set_state( PikPikQmrWindow *wnd, enum PIK_PIK_STATE state )
{

    // void gtk_widget_set_sensitive( GtkWidget* widget, gboolean sensitive );
    //
    // Sets the sensitivity of a widget.
    //
    // A widget is sensitive if the user can interact with it. Insensitive
    // widgets are “grayed out” and the user can’t interact with them.
    // Insensitive widgets are known as “inactive”, “disabled”, or “ghosted” in
    // some other toolkits.

    switch( state )
    {
        default:
        case PIK_PIK_STATE_NONE:
        case PIK_PIK_STATE_DISCONNECTED:
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->label_port), TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->dropdown_port), TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->entry_port_other), TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_reload), TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_connect), TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_disconnect), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_poke), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_read), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_write), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_verify), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->label_password), TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->entry_password), TRUE );
            wnd->state = PIK_PIK_STATE_DISCONNECTED;
            break;

        case PIK_PIK_STATE_CONNECTED:
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->label_port), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->dropdown_port), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->entry_port_other), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_reload), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_connect), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_disconnect), TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_poke), TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_read), TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_write), TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_verify), TRUE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->label_password), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->entry_password), FALSE );
            wnd->state = PIK_PIK_STATE_CONNECTED;
            break;

        case PIK_PIK_STATE_WIP:
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->label_port), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->dropdown_port), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->entry_port_other), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_reload), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_connect), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_disconnect), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_poke), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_read), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_write), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->button_verify), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->label_password), FALSE );
            gtk_widget_set_sensitive( GTK_WIDGET(wnd->entry_password), FALSE );
            wnd->state = PIK_PIK_STATE_WIP;
            break;
    }
}

// #############################################################################

static void pik_pik_errorbox( PikPikQmrWindow *wnd, char *filename )
{
    // Creates a new GtkAlertDialog object.
    // The message will be set to the formatted string resulting from the arguments.
    GtkAlertDialog* dlg = gtk_alert_dialog_new( "Error reading “%s”", filename );

    // Sets the alert block interaction with the parent window while it is presented
    gtk_alert_dialog_set_modal( dlg, TRUE );

    // Sets the button labels for the alert
    const char *choices[] = { "OK", NULL };
    gtk_alert_dialog_set_buttons( dlg, choices );

    // Sets the detail text that will be shown in the alert
    gtk_alert_dialog_set_detail( dlg, g_strerror( errno ) );

    gtk_alert_dialog_choose
    (
        dlg,                // GtkAlertDialog* self
        GTK_WINDOW(wnd),    // GtkWindow* parent — The parent GtkWindow
        NULL,               // GCancellable* cancellable — A GCancellable to cancel the operation
        NULL,               // GAsyncReadyCallback callback — A callback to call when the operation is complete
        NULL                // gpointer user_data — Data to pass to callback
    );

    return;
}

static void pik_pik_messagebox( PikPikQmrWindow *wnd, const char *title, const char *fmt, ... )
{
    // Creates a new GtkAlertDialog object.
    // The message will be set to the formatted string resulting from the arguments.
    GtkAlertDialog *dlg = gtk_alert_dialog_new( "%s", title );

    // Sets the alert block interaction with the parent window while it is presented
    gtk_alert_dialog_set_modal( dlg, TRUE );

    // Sets the button labels for the alert
    const char *choices[] = { "OK", NULL };
    gtk_alert_dialog_set_buttons( dlg, choices );

    // format the message
    va_list arg;
    char buf[1024];
    va_start( arg, fmt );
    g_vsnprintf( buf, sizeof(buf), fmt, arg );
    va_end( arg );

    // Sets the detail text that will be shown in the alert
    gtk_alert_dialog_set_detail( dlg, buf );

    gtk_alert_dialog_choose
    (
        dlg,                // GtkAlertDialog* self
        GTK_WINDOW(wnd),    // GtkWindow* parent — The parent GtkWindow
        NULL,               // GCancellable* cancellable — A GCancellable to cancel the operation
        NULL,               // GAsyncReadyCallback callback — A callback to call when the operation is complete
        NULL                // gpointer user_data — Data to pass to callback
    );

    return;
}

// #############################################################################

static void pik_pik_get_selected_port( PikPikQmrWindow *wnd )
{
    guint n = g_list_model_get_n_items( G_LIST_MODEL(wnd->stringlist_ports) );
    guint x = gtk_drop_down_get_selected( wnd->dropdown_port );

    if( n == x + 1 )
    {
        // "Other..." is selected
        // get data from the entry
        //
        // gtk_editable_get_text - the returned string is owned by GTK and
        // must not be modified or freed.
        //
        const char *str = gtk_editable_get_text( GTK_EDITABLE(wnd->entry_port_other) );
        strncpy( wnd->selected_port, str, sizeof(wnd->selected_port) );
    }
    else
    {
        // some port is selected
        const char* str = gtk_string_list_get_string( GTK_STRING_LIST(wnd->stringlist_ports), x );
        strncpy( wnd->selected_port, str, sizeof(wnd->selected_port) );
    }
}

// #############################################################################

static void pik_pik_clear_ports( PikPikQmrWindow *wnd )
{
    // get number of items and remove one by one
    guint n = g_list_model_get_n_items( G_LIST_MODEL(wnd->stringlist_ports) );
    for( guint i = 0; i < n; i++ )
        gtk_string_list_remove( wnd->stringlist_ports, 0 );
}

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

        // add devnode to the list
        gtk_string_list_append( wnd->stringlist_ports, devnode );

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

static void pik_pik_reload_ports( PikPikQmrWindow *wnd )
{
    // load list of ports
    pik_pik_clear_ports( wnd );
    pik_pik_list_serial( wnd );

    // append default "other" selection
    gtk_string_list_append( wnd->stringlist_ports, "Other..." );

    // default selection: first
    gtk_drop_down_set_selected( wnd->dropdown_port, 0 );
}

// #############################################################################

static void pik_pik_button_reload_clicked( GtkButton *button, PikPikQmrWindow *wnd )
{
    pik_pik_reload_ports( wnd );
    pik_pik_check_other_port_visibility( wnd );
    pik_pik_get_selected_port( wnd );
}

// #############################################################################

static void pik_pik_button_connect_clicked( GtkButton *button, PikPikQmrWindow *wnd )
{
    // Open the serial port
    wnd->serial_port = g_open( wnd->selected_port, O_RDWR | O_NONBLOCK );

    // Check for errors
    if( wnd->serial_port < 0 )
    {
        //printf( "Error %i from open: %s\n", errno, strerror( errno ) );
        pik_pik_errorbox( wnd, wnd->selected_port );
        return;
    }

#ifdef G_OS_UNIX
    // Create new termios struct, we call it 'tty' for convention
    struct termios tty;

    // Read in existing settings, and handle any error
    if( tcgetattr( wnd->serial_port, &tty ) != 0 )
    {
        g_close( wnd->serial_port, NULL );
        //printf( "Error %i from tcgetattr: %s\n", errno, strerror( errno ) );
        pik_pik_errorbox( wnd, wnd->selected_port );
        return;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

    tty.c_cc[VTIME] = 50;    // Wait for up to 5s (50 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 9600
    cfsetispeed( &tty, B9600 );
    cfsetospeed( &tty, B9600 );

    // Save tty settings, also checking for error
    if( tcsetattr( wnd->serial_port, TCSANOW, &tty ) != 0 )
    {
        g_close( wnd->serial_port, NULL );
        //printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        pik_pik_errorbox( wnd, wnd->selected_port );
        return;
    }
#endif

    pik_pik_set_state( wnd, PIK_PIK_STATE_CONNECTED );
}

// #############################################################################

static void pik_pik_button_disconnect_clicked( GtkButton *button, PikPikQmrWindow *wnd )
{
    if( wnd->serial_port >= 0 )
    {
        // close the file descriptor, so that it no longer refers to any file
        // and may be reused
        g_close( wnd->serial_port, NULL );
    }

    pik_pik_set_state( wnd, PIK_PIK_STATE_DISCONNECTED );
}

// #############################################################################

static void pik_pik_button_poke_clicked( GtkButton *button, PikPikQmrWindow *wnd )
{
    // poke available only when connected to board
    if( wnd->state != PIK_PIK_STATE_CONNECTED )
        return;

    if( wnd->serial_port == -1 )
        return;

    //pik_pik_messagebox( wnd, "Message", "test: %i", 5 );

    const char *version_msg = "pik_pik_version\r\n";

    write( wnd->serial_port, version_msg, strlen(version_msg) );
}

// #############################################################################

static void pik_pik_qmr_window_class_init( PikPikQmrWindowClass *klass )
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    // extend a widget with XML
    gtk_widget_class_set_template_from_resource( widget_class, "/ee/pik_pik/qmr/pik-pik_qmr-main.ui" );

    // binds a child widget defined in a template to the widget_class
    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, label_port );
    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, dropdown_port );
    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, entry_port_other );

    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, button_reload );
    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, button_connect );
    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, button_disconnect );
    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, button_poke );
    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, button_read );
    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, button_write );
    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, button_verify );

    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, label_password );
    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, entry_password );
}

// #############################################################################

static void pik_pik_check_other_port_visibility( PikPikQmrWindow *wnd )
{
    guint n = g_list_model_get_n_items( G_LIST_MODEL(wnd->stringlist_ports) );
    guint x = gtk_drop_down_get_selected( wnd->dropdown_port );
    if( n == x + 1 )
    {
        // "Other..." is selected
        // other field needed
        gtk_widget_set_visible( GTK_WIDGET(wnd->entry_port_other), TRUE );
    }
    else
    {
        // some port is selected
        // other field is not needed
        gtk_widget_set_visible( GTK_WIDGET(wnd->entry_port_other), FALSE );
    }
}

// #############################################################################

static void pik_pik_dropdown_port_change( GtkDropDown *dropdown, GParamSpec *pspec, PikPikQmrWindow *wnd )
{
    pik_pik_get_selected_port( wnd );
    pik_pik_check_other_port_visibility( wnd );
}

static void pik_pik_entry_port_other_change( GtkDropDown *dropdown, PikPikQmrWindow *wnd )
{
    pik_pik_get_selected_port( wnd );
}

// #############################################################################

static void pik_pik_parse_line( PikPikQmrWindow *wnd, GString *str )
{
    // skip prompt echo
    if( strncmp( str->str, "device>", 7 ) == 0 )
        return;

    // skip empty lines
    if( str->str[0] == '\0' )
        return;

    // display version
    if( strncmp( str->str, "response_pik_pik_version ", 25 ) == 0 )
    {
        pik_pik_messagebox( wnd, "Device Version", "Data: %s", &str->str[25] );
    }


}

// #############################################################################

// The given function is called repeatedly until it returns G_SOURCE_REMOVE or
// FALSE, at which point the timeout is automatically destroyed and the function
// will not be called again. The first call to the function will be at the end
// of the first interval.

static gint pik_pik_serial_reader( PikPikQmrWindow *wnd )
{
    // read only when connected
    if( wnd->state != PIK_PIK_STATE_CONNECTED )
        return TRUE;

    if( wnd->serial_port == -1 )
        return TRUE;

    char buf[256];

    // try to read data from serial port
    ssize_t bytes_read = read( wnd->serial_port, buf, sizeof(buf) );

    // exit if no data
    if( bytes_read == 0 || bytes_read == -1 )
        return TRUE;

    for( int i = 0; i < bytes_read; i++ )
    {
        // skip CR characters
        if( buf[i] == '\r' )
            continue;

        // parse the line when LF character is received
        if( buf[i] == '\n' )
        {
            pik_pik_parse_line( wnd, wnd->serial_line );

            // clear string buffer
            g_string_erase( wnd->serial_line, 0, -1 );

            continue;
        }

        // add character to string buffer
        g_string_append_c( wnd->serial_line, buf[i] );
    }

    return TRUE;
}

// #############################################################################

static void pik_pik_qmr_window_init( PikPikQmrWindow *self )
{
    // creates and initializes child widgets defined in templates
    gtk_widget_init_template( GTK_WIDGET(self) );

    //
    // connect GCallback functions to a signal of widgets
    //

    // buttons
    g_signal_connect( self->button_connect, "clicked", G_CALLBACK(pik_pik_button_connect_clicked), self );
    g_signal_connect( self->button_disconnect, "clicked", G_CALLBACK(pik_pik_button_disconnect_clicked), self );
    g_signal_connect( self->button_reload, "clicked", G_CALLBACK(pik_pik_button_reload_clicked), self );
    g_signal_connect( self->button_poke, "clicked", G_CALLBACK(pik_pik_button_poke_clicked), self );

    // initialize stringlist for ports
    const char *strings[] = { "test1", "test2", "test3", NULL };
    self->stringlist_ports = gtk_string_list_new( strings );

    // attach stringlist to the dropdown
    gtk_drop_down_set_model( self->dropdown_port, G_LIST_MODEL(self->stringlist_ports) );

    pik_pik_reload_ports( self );
    pik_pik_check_other_port_visibility( self );
    pik_pik_get_selected_port( self );
    pik_pik_set_state( self, PIK_PIK_STATE_DISCONNECTED );

    // default file descriptor: no file opened
    self->serial_port = -1;

    // default state
    self->state = PIK_PIK_STATE_DISCONNECTED;

    // prepare buffer for serial data lines
    self->serial_line = g_string_new( "" );
    if( self->serial_line == NULL )
        g_error( "Unable to allocate memory for serial_line string" );

    // start serial reader
    g_timeout_add( 250, G_SOURCE_FUNC(pik_pik_serial_reader), self );

    // connect dropdown_port selection change signal to
    // pik_pik_dropdown_port_change callback function
    g_signal_connect( self->dropdown_port, "notify::selected", G_CALLBACK(pik_pik_dropdown_port_change), self );

    // connect entry_port_other text change signal to
    // pik_pik_entry_port_other_change callback function
    g_signal_connect( self->entry_port_other, "changed", G_CALLBACK(pik_pik_entry_port_other_change), self );
}

// #############################################################################



