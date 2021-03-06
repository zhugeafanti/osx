/* gtkmenuitempeer.c -- Native implementation of GtkMenuItemPeer
   Copyright (C) 1999 Free Software Foundation, Inc.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */


#include "gtkpeer.h"
#include "gnu_java_awt_peer_gtk_GtkCheckboxMenuItemPeer.h"

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkCheckboxMenuItemPeer_create
  (JNIEnv *env, jobject obj, jstring label)
{
  GtkWidget *widget;
  const char *str;

  /* Create global reference and save it for future use */
  NSA_SET_GLOBAL_REF (env, obj);

  str = (*env)->GetStringUTFChars (env, label, NULL);

  gdk_threads_enter ();
  
  widget = gtk_check_menu_item_new_with_label (str);
  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (widget), 1);
  gtk_widget_show (widget);

  gdk_threads_leave ();

  (*env)->ReleaseStringUTFChars (env, label, str);

  NSA_SET_PTR (env, obj, widget);
}

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkCheckboxMenuItemPeer_setState
  (JNIEnv *env, jobject obj, jboolean state)
{
  void *ptr;

  ptr = NSA_GET_PTR (env, obj);
  
  gdk_threads_enter ();
  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (ptr), state);
  gdk_threads_leave ();
}
