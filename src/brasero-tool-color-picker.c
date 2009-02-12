/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * brasero
 * Copyright (C) Philippe Rouquier 2008 <bonfire-app@wanadoo.fr>
 * 
 * brasero is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * brasero is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>

#include "brasero-tool-color-picker.h"

typedef struct _BraseroToolColorPickerPrivate BraseroToolColorPickerPrivate;
struct _BraseroToolColorPickerPrivate
{
	GtkWidget *dialog;

	GtkWidget *label;
	GtkWidget *icon;

	GdkColor color;
};

enum {
	COLOR_SET_SIGNAL,
	LAST_SIGNAL,
};
static guint tool_color_picker_signals[LAST_SIGNAL] = { 0 };


#define BRASERO_TOOL_COLOR_PICKER_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), BRASERO_TYPE_TOOL_COLOR_PICKER, BraseroToolColorPickerPrivate))

G_DEFINE_TYPE (BraseroToolColorPicker, brasero_tool_color_picker, GTK_TYPE_TOOL_BUTTON);

void
brasero_tool_color_picker_set_text (BraseroToolColorPicker *self,
				    const gchar *text)
{
	BraseroToolColorPickerPrivate *priv;

	priv = BRASERO_TOOL_COLOR_PICKER_PRIVATE (self);
	gtk_label_set_text_with_mnemonic (GTK_LABEL (priv->label), text);
}

void
brasero_tool_color_picker_get_color (BraseroToolColorPicker *self,
				     GdkColor *color)
{
	BraseroToolColorPickerPrivate *priv;

	priv = BRASERO_TOOL_COLOR_PICKER_PRIVATE (self);
	color->red = priv->color.red;
	color->green = priv->color.green;
	color->blue = priv->color.blue;
}

void
brasero_tool_color_picker_set_color (BraseroToolColorPicker *self,
				     GdkColor *color)
{
	BraseroToolColorPickerPrivate *priv;

	priv = BRASERO_TOOL_COLOR_PICKER_PRIVATE (self);
	priv->color.red = color->red;
	priv->color.green = color->green;
	priv->color.blue = color->blue;

	gtk_widget_queue_draw (GTK_WIDGET (self));
}

static gboolean
brasero_tool_color_picker_expose (GtkWidget *widget,
				  GdkEventExpose *event,
				  BraseroToolColorPicker *self)
{
	BraseroToolColorPickerPrivate *priv;
	cairo_t *ctx;

	priv = BRASERO_TOOL_COLOR_PICKER_PRIVATE (self);

	ctx = gdk_cairo_create (GDK_DRAWABLE (widget->window));
	gdk_cairo_set_source_color (ctx, &priv->color);
	cairo_rectangle (ctx,
			 widget->allocation.x,
			 widget->allocation.y,
			 widget->allocation.width,
			 widget->allocation.height);
	cairo_fill (ctx);
	cairo_stroke (ctx);
	cairo_destroy (ctx);

	return FALSE;
}

static void
brasero_tool_color_picker_cancel_clicked (GtkWidget *widget,
					  BraseroToolColorPicker *self)
{
	BraseroToolColorPickerPrivate *priv;

	priv = BRASERO_TOOL_COLOR_PICKER_PRIVATE (self);

	gtk_widget_destroy (priv->dialog);
	priv->dialog = NULL;
}

static void
brasero_tool_color_picker_ok_clicked (GtkWidget *widget,
				      BraseroToolColorPicker *self)
{
	BraseroToolColorPickerPrivate *priv;
	GtkColorSelection *selection;

	priv = BRASERO_TOOL_COLOR_PICKER_PRIVATE (self);

	selection = GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (priv->dialog)->colorsel);
	gtk_color_selection_get_current_color (selection, &priv->color);
	gtk_widget_destroy (priv->dialog);
	priv->dialog = NULL;

	g_signal_emit (self,
		       tool_color_picker_signals[COLOR_SET_SIGNAL],
		       0);
}

static void
brasero_tool_color_picker_clicked (BraseroToolColorPicker *self,
				   gpointer NULL_data)
{
	GtkWidget *dialog;
	GtkWidget *toplevel;
	GtkColorSelection *selection;
	BraseroToolColorPickerPrivate *priv;

	priv = BRASERO_TOOL_COLOR_PICKER_PRIVATE (self);

	dialog = gtk_color_selection_dialog_new (_("Pick a Color"));
	selection = GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dialog)->colorsel);
	gtk_color_selection_set_current_color (selection, &priv->color);

	toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));
	if (toplevel && GTK_IS_WINDOW (toplevel)) {
		gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (toplevel));
		gtk_window_set_modal (GTK_WINDOW (dialog), gtk_window_get_modal (GTK_WINDOW (toplevel)));
	}

	g_signal_connect (GTK_COLOR_SELECTION_DIALOG (dialog)->ok_button,
			  "clicked",
			  G_CALLBACK (brasero_tool_color_picker_ok_clicked),
			  self);
	g_signal_connect (GTK_COLOR_SELECTION_DIALOG (dialog)->cancel_button,
			  "clicked",
			  G_CALLBACK (brasero_tool_color_picker_cancel_clicked),
			  self);
	g_signal_connect (dialog,
			  "destroy",
			  G_CALLBACK (brasero_tool_color_picker_cancel_clicked),
			  self);

	priv->dialog = dialog;
	gtk_window_present (GTK_WINDOW (dialog));
}

static void
brasero_tool_color_picker_init (BraseroToolColorPicker *object)
{
	BraseroToolColorPickerPrivate *priv;
	GtkWidget *frame;

	priv = BRASERO_TOOL_COLOR_PICKER_PRIVATE (object);

	g_signal_connect (object,
			  "clicked",
			  G_CALLBACK (brasero_tool_color_picker_clicked),
			  NULL);

	priv->label = gtk_label_new_with_mnemonic ("");
	gtk_widget_show (priv->label);
	gtk_tool_button_set_label_widget (GTK_TOOL_BUTTON (object), priv->label);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT);
	gtk_widget_show (frame);

	priv->icon = gtk_alignment_new (0.5, 0.5, 1., 1.);
	gtk_widget_show (priv->icon);
	g_signal_connect (priv->icon,
			  "expose-event",
			  G_CALLBACK (brasero_tool_color_picker_expose),
			  object);
	gtk_container_add (GTK_CONTAINER (frame), priv->icon);

	gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (object), frame);
}

static void
brasero_tool_color_picker_finalize (GObject *object)
{
	G_OBJECT_CLASS (brasero_tool_color_picker_parent_class)->finalize (object);
}

static void
brasero_tool_color_picker_class_init (BraseroToolColorPickerClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (BraseroToolColorPickerPrivate));

	object_class->finalize = brasero_tool_color_picker_finalize;

	tool_color_picker_signals[COLOR_SET_SIGNAL] =
		g_signal_new ("color_set",
		              G_OBJECT_CLASS_TYPE (klass),
		              G_SIGNAL_RUN_FIRST,
		              0,
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0,
		              G_TYPE_NONE);
}

GtkWidget *
brasero_tool_color_picker_new (void)
{
	return g_object_new (BRASERO_TYPE_TOOL_COLOR_PICKER, NULL);
}
