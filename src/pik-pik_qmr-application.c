// #############################################################################

/* pik-pik_qmr-application.c
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

#include "pik-pik_qmr-application.h"
#include "pik-pik_qmr-window.h"

struct _PikPikQmrApplication
{
  GtkApplication parent_instance;
};

G_DEFINE_TYPE( PikPikQmrApplication, pik_pik_qmr_application, GTK_TYPE_APPLICATION )

// #############################################################################
//
//  NEW
// 
PikPikQmrApplication *pik_pik_qmr_application_new( const char *application_id, GApplicationFlags  flags )
{
    g_return_val_if_fail( application_id != NULL, NULL );

    return g_object_new( PIK_PIK_QMR_TYPE_APPLICATION, "application-id", application_id, "flags", flags, NULL );
}

// #############################################################################
//
// ACTIVATE
// 
static void pik_pik_qmr_application_activate( GApplication *app )
{
    GtkWindow *window;

    g_assert( PIK_PIK_QMR_IS_APPLICATION( app ) );

    window = gtk_application_get_active_window( GTK_APPLICATION( app ) );
    if( window == NULL )
        window = g_object_new( PIK_PIK_QMR_TYPE_WINDOW, "application", app, NULL );
    
    gtk_window_present( window );

    
  /*GMenu *menubar = g_menu_new( );
  GMenuItem *menu_item_menu = g_menu_item_new( "Menu", NULL );
  GMenu *menu = g_menu_new( );
  GMenuItem *menu_item_quit = g_menu_item_new( "Quit", "app.quit" );
  g_menu_append_item( menu, menu_item_quit );
  g_object_unref( menu_item_quit );
  g_menu_item_set_submenu( menu_item_menu, G_MENU_MODEL (menu) );
  g_menu_append_item( menubar, menu_item_menu );
  g_object_unref( menu_item_menu );*/

        // attach menumodels from xml file menu buttons on the window
    
    GtkBuilder *builder = gtk_builder_new_from_resource( "/ee/pik_pik/qmr/pik-pik_qmr-menu.xml" );
    GMenuModel *menubar = G_MENU_MODEL( gtk_builder_get_object( builder, "menu_file" ) );
    g_object_unref( builder );

  gtk_application_set_menubar( GTK_APPLICATION(app), G_MENU_MODEL(menubar) );
}

// #############################################################################
//
// STARTUP
// 
static void pik_pik_qmr_application_startup( GApplication *app )
{
    g_assert( PIK_PIK_QMR_IS_APPLICATION(app) );

    //GtkWindow *window = gtk_application_get_active_window( GTK_APPLICATION(app) );

    // attach menumodels from xml file menu buttons on the window
    
    GtkBuilder *builder = gtk_builder_new_from_resource( "/ee/pik_pik/qmr/pik-pik_qmr-menu.xml" );
 
    GMenuModel *menumodel = G_MENU_MODEL( gtk_builder_get_object( builder, "menu_file" ) );

    gtk_application_set_menubar( GTK_APPLICATION(app), menumodel );

    g_object_unref( builder );
}

// #############################################################################

static void pik_pik_qmr_application_class_init( PikPikQmrApplicationClass *klass )
{
    GApplicationClass *app_class = G_APPLICATION_CLASS(klass);

    app_class->activate = pik_pik_qmr_application_activate;

    // wtf?    
    // (pik-pik_qmr:2): GLib-GIO-CRITICAL **: 22:45:47.643: GApplication subclass 'PikPikQmrApplication' failed to chain up on ::startup (from start of override function)
    //app_class->startup = pik_pik_qmr_application_startup;
}

// #############################################################################

static void pik_pik_qmr_application_about_action( GSimpleAction *action, GVariant *parameter, gpointer user_data )
{
    static const char *authors[] = { "Valentin the Degu", NULL };
    PikPikQmrApplication *self = user_data;
    GtkWindow *window = NULL;

    g_assert( PIK_PIK_QMR_IS_APPLICATION(self) );

    window = gtk_application_get_active_window( GTK_APPLICATION(self) );

    gtk_show_about_dialog(
        window,
        "program-name", "pik-pik_qmr",
        "logo-icon-name", "ee.pik_pik.qmr",
        "authors", authors,
        "version", "0.1.0",
        "copyright", "© 2023 Valentin the Degu",
        NULL
    );
}

// #############################################################################

static void pik_pik_qmr_application_quit_action( GSimpleAction *action, GVariant *parameter, gpointer user_data )
{
    PikPikQmrApplication *self = user_data;

    g_assert( PIK_PIK_QMR_IS_APPLICATION(self) );

    g_application_quit( G_APPLICATION(self) );
}

// #############################################################################
//
// INIT 
// 

static const GActionEntry app_actions[] =
{
    { "quit", pik_pik_qmr_application_quit_action },
    { "about", pik_pik_qmr_application_about_action },
};

static void pik_pik_qmr_application_init( PikPikQmrApplication *self )
{
    g_action_map_add_action_entries(
        G_ACTION_MAP(self),
        app_actions,
        G_N_ELEMENTS(app_actions),
        self
    );

    gtk_application_set_accels_for_action(
        GTK_APPLICATION(self),
        "app.quit",
        (const char *[]) { "<primary>q", NULL }
    );
}

// #############################################################################


