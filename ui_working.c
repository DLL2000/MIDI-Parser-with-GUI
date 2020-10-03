#include <gtk/gtk.h>
#include <string.h>



/* 
 *  called when program starts, creates the whole window
 */

void activate(GtkApplication* app, gpointer user_data)
{

  // Create the window, the container of all other widgets

  GtkWidget* window = gtk_application_window_new(app);

  // Set window title

  gtk_window_set_title(GTK_WINDOW(window), "Hello Purdue");
    
  // show the window

  gtk_widget_show_all(window);
} /* activate() */
