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

struct _PikPikQmrWindow
{
    GtkApplicationWindow  parent_instance;

    /* Template widgets */
    //GtkHeaderBar        *header_bar;
    GtkLabel            *label;

    GtkMenuButton       *menubtn_file;
    GtkMenuButton       *menubtn_tools;
    GtkMenuButton       *menubtn_help;
};

G_DEFINE_FINAL_TYPE( PikPikQmrWindow, pik_pik_qmr_window, GTK_TYPE_APPLICATION_WINDOW )

// #############################################################################

static void pik_pik_qmr_window_class_init( PikPikQmrWindowClass *klass )
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gtk_widget_class_set_template_from_resource( widget_class, "/ee/pik_pik/qmr/pik-pik_qmr-main.ui" );
    //gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, header_bar );
    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, label );

    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, menubtn_file );
    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, menubtn_tools );
    gtk_widget_class_bind_template_child( widget_class, PikPikQmrWindow, menubtn_help );
}

// #############################################################################

static void pik_pik_qmr_window_init( PikPikQmrWindow *self )
{
    gtk_widget_init_template( GTK_WIDGET(self) );

    /*GMenu *menumodel;
    GMenu *submenu;

    menumodel = g_menu_new( );
    g_menu_append( menumodel, "New", "app.new" );
    g_menu_append( menumodel, "Open", "app.open" );
    g_menu_append( menumodel, "Save As", "app.save" );
    g_menu_append( menumodel, "Quit", "app.quit" );
    gtk_menu_button_set_menu_model( GTK_MENU_BUTTON(self->menu_file), G_MENU_MODEL(menumodel) );

    menumodel = g_menu_new( );
    g_menu_append( menumodel, "Card Tool", "win.card_tool" );
    g_menu_append( menumodel, "Play Tune", "win.play_tune" );
    gtk_menu_button_set_menu_model( GTK_MENU_BUTTON(self->menu_tools), G_MENU_MODEL(menumodel) );
    
    menumodel = g_menu_new( );
    g_menu_append( menumodel, "Manual", "win.manual" );
    g_menu_append( menumodel, "About", "win.about" );
    gtk_menu_button_set_menu_model( GTK_MENU_BUTTON(self->menu_help), G_MENU_MODEL(menumodel) );*/

    

    // attach menumodels from xml file menu buttons on the window
    
    /*GtkBuilder *builder = gtk_builder_new_from_resource( "/ee/pik_pik/qmr/pik-pik_qmr-menu.xml" );
    
    GMenuModel *menumodel = G_MENU_MODEL( gtk_builder_get_object( builder, "menu_file" ) );
    gtk_menu_button_set_menu_model( GTK_MENU_BUTTON(self->menubtn_file), menumodel );

    menumodel = G_MENU_MODEL( gtk_builder_get_object( builder, "menu_tools" ) );
    gtk_menu_button_set_menu_model( GTK_MENU_BUTTON(self->menubtn_tools), menumodel );

    menumodel = G_MENU_MODEL( gtk_builder_get_object( builder, "menu_help" ) );
    gtk_menu_button_set_menu_model( GTK_MENU_BUTTON(self->menubtn_help), menumodel );

    g_object_unref( builder );*/

    gtk_application_window_set_show_menubar( GTK_APPLICATION_WINDOW(self), true );
    
}

// #############################################################################



