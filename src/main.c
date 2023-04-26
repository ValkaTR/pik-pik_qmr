// #############################################################################

/* main.c
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

#include <glib/gi18n.h>

#include "pik-pik_qmr-application.h"

// #############################################################################

int main( int argc, char *argv[] )
{
    g_autoptr(PikPikQmrApplication) app = NULL;
    int ret;

    bindtextdomain( GETTEXT_PACKAGE, LOCALEDIR );
    bind_textdomain_codeset( GETTEXT_PACKAGE, "UTF-8" );
    textdomain( GETTEXT_PACKAGE );

    app = pik_pik_qmr_application_new( "ee.pik_pik.qmr", G_APPLICATION_DEFAULT_FLAGS );
    ret = g_application_run( G_APPLICATION(app), argc, argv );

    return ret;
}

// #############################################################################

//
// TEST CODE #1
// very important
//

/*static void quit_activated( GSimpleAction *action, GVariant *parameter, GApplication *application )
{
  g_application_quit( application );
}

static void app_activate( GApplication *application )
{
    GtkApplication *app = GTK_APPLICATION(application);
    GtkWidget *win = gtk_application_window_new( GTK_APPLICATION(app) );
    gtk_window_set_title( GTK_WINDOW(win), "menu1" );
    gtk_window_set_default_size( GTK_WINDOW(win), 400, 300 );

    gtk_application_window_set_show_menubar( GTK_APPLICATION_WINDOW(win), TRUE );
    gtk_window_present( GTK_WINDOW(win) );
}

static void app_startup( GApplication *application )
{
    GtkApplication *app = GTK_APPLICATION (application);

    GSimpleAction *act_quit = g_simple_action_new ("quit", NULL);
    g_action_map_add_action( G_ACTION_MAP(app), G_ACTION(act_quit) );
    g_signal_connect( act_quit, "activate", G_CALLBACK(quit_activated), application );

    // manual menu population
    //GMenu *menubar = g_menu_new( );
    //GMenuItem *menu_item_menu = g_menu_item_new( "Menu", NULL );
    //GMenu *menu = g_menu_new( );
    //GMenuItem *menu_item_quit = g_menu_item_new( "Quit", "app.quit" );
    //g_menu_append_item( menu, menu_item_quit );
    //g_object_unref( menu_item_quit );
    //g_menu_item_set_submenu( menu_item_menu, G_MENU_MODEL(menu) );
    //g_menu_append_item( menubar, menu_item_menu );
    //g_object_unref( menu_item_menu );
    //gtk_application_set_menubar( GTK_APPLICATION(app), G_MENU_MODEL(menubar) );

    GtkBuilder *builder = gtk_builder_new_from_resource( "/ee/pik_pik/qmr/pik-pik_qmr-menu.xml" );
    GMenuModel *menumodel = G_MENU_MODEL( gtk_builder_get_object( builder, "menu_main" ) );
    gtk_application_set_menubar( GTK_APPLICATION(app), menumodel );
    g_object_unref( builder );

}

#define APPLICATION_ID "ee.pik_pik.qmr"

int main( int argc, char **argv )
{
    GtkApplication *app;
    int stat;

    app = gtk_application_new( APPLICATION_ID, G_APPLICATION_DEFAULT_FLAGS );
    g_signal_connect( app, "startup", G_CALLBACK(app_startup), NULL );
    g_signal_connect( app, "activate", G_CALLBACK(app_activate), NULL );

    stat = g_application_run( G_APPLICATION(app), argc, argv );
    g_object_unref( app );
    return stat;
}*/

// #############################################################################
//
// TEST CODE #2
// another important test code
//

/*static void print_hello( GtkWidget *widget, gpointer  data )
{
  g_print ("Hello World\n");
}

static void activate( GtkApplication *app, gpointer user_data )
{
    GtkWindow *window;
    GtkWindow *app_widget;
    GtkWidget *button;

    app_widget = GTK_WINDOW(gtk_application_window_new( app ));
    window = GTK_WINDOW(app_widget);
    gtk_window_set_title( window, "Window" );
    gtk_window_set_default_size( window, 200, 200 );

    button = gtk_button_new_with_label( "Hello World" );
    g_signal_connect( button, "clicked", G_CALLBACK(print_hello), NULL );
    gtk_window_set_child( window, button );

    gtk_window_present( window );
}

int main( int argc, char *argv[] )
{

    GtkApplication *app;
    int status;

    app = gtk_application_new( "ee.pik_pik.qmr", G_APPLICATION_DEFAULT_FLAGS );
    g_signal_connect( app, "activate", G_CALLBACK(activate), NULL );
    status = g_application_run( G_APPLICATION(app), argc, argv );
    g_object_unref( app );

    return status;


}*/

// #############################################################################

