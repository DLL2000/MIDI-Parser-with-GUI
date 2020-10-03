/* Name, ui.c, CS 24000, Spring 2020
 * Last updated April 9, 2020
 */

/* Add any includes here */

#define _GNU_SOURCE
#include "ui.h"
#include "library.h"
#include <assert.h>
#include <ftw.h>
#include <string.h>
#include "alterations.h"


#define MIDDLE_C (64)
#define NOTE_NOT_USED (-1)
#define LIST_LENGTH (29)
#define MIDDLE_OF_LIST (15)
#define MIN_RANGE (1)
#define MAX_RANGE (100)
#define OCTAVE_MAX_RANGE (8)

tree_node_t *g_current_node;
track_t *copy_track(track_t *track);
song_data_t *g_modified_song;

/* 
 * This structure contains all the widgets in GUI
 */

struct ui_widgets {

  /* holds the text shown in the GUI */

  GtkWidget* text_label;

  /* holds the window to add the widget */

  GtkWidget *window;

  /* holds the list of songs shown in the GUI */

  GtkWidget *song_list;

  /* holds the name of the song */

  GtkWidget *file_name;

  /* holds the file path of the song */

  GtkWidget *file_path;

  /* holds the lowest and highest notes in the song */

  GtkWidget *note_range;

  /* holds the original length of the song */

  GtkWidget *original_length;

  /* holds the song visualization of the song */

  GtkWidget *original_song_drawing_area;

  /* holds the modified song visualization of the song */

  GtkWidget *effect_drawing_area;

} g_widgets;

/*
 * This structure contains all the global parameters used among different GUI
 * functions
 */

struct parameters {

  /* holds the current widget number */

  int n;

  /* holds the list of song names */

  tree_node_t *song_library;

  /* holds the lowest note of the song */

  int lowest_note;

  /* holds the highest note of the song */

  int highest_note;

  /* holds the original length of the song */

  int original_length;

  /* holds the time scale modification of the song */

  int time_scale;

  /* holds the speed modification of the song */

  int warp_time;

  /* holds the octave modification to the song */

  int song_octave;

  /* holds the song visualization length of the song */

  int drawing_height;

} g_parameters;

/* 
 * This function copies a modified song into g_current_node
 */

void copy_song() {

  /* checks if g_modified_song is not null */

  if (g_modified_song != NULL) {

    /* frees all memory in the song */

    free_song(g_modified_song);

    /* sets g_modified_song to null */

    g_modified_song = NULL;
  }

  /* allocates space in memory for song_data_t struct */

  g_modified_song = (song_data_t *)malloc(sizeof(song_data_t));

  /* holds the song held in the current node */

  song_data_t *song = g_current_node->song;

  /* makes g_modified_song->path the same as song->path */

  g_modified_song->path = song->path;

  /* makes g_modified_song->format the same as song->format */

  g_modified_song->format = song->format;

  /* makes g_modified_song->num_tracks the same as song->num_tracks */

  g_modified_song->num_tracks = song->num_tracks;

  /* sets g_modified_song->track_list to null */

  g_modified_song->track_list = NULL;

  /* holds the last track in the list */

  track_node_t *last_track_list = NULL;

  /* holds the track list of song */

  track_node_t *song_track_list = song->track_list;

  /* loops until i is the same as song->num_tracks */

  for (int i = 0; i < song->num_tracks; i++) {

    /* allocates space in memory for track_node_t struct */

    track_node_t *new_track_list =
                                  (track_node_t *)malloc(sizeof(track_node_t));

    /* copies information from song_track_list->track into */
    /* new_track_list->track */

    new_track_list->track = copy_track(song_track_list->track);

    /* sets new_track_list->next_track */

    new_track_list->next_track = NULL;

    /* checks if g_modified_song->track_list is null */

    if (g_modified_song->track_list == NULL) {

      /* makes g_modified_song->track_list the same as new_track_list */

      g_modified_song->track_list = new_track_list;           
    }
    else {

      /* makes last_track_list->next_track the same as new_track_list */

      last_track_list->next_track = new_track_list;
    }

    /* makes last_track_list the same as new_track_list */

    last_track_list = new_track_list;

    /* moves to the next element in the list */

    song_track_list = song_track_list->next_track;
  }

} /* copy_song() */

/*
 * This function draws a line in the GUI
 */

void draw_line(GtkDrawingArea *area, cairo_t *cr, int start_x, int start_y, 
                         int end_x, int end_y, GdkRGBA *color) {

  /* colors the visualization in the GUI */

  gdk_cairo_set_source_rgba(cr, color);

  /* sets the line width of the visualization */

  cairo_set_line_width(cr, 1);

  /* sets the starting point of the visualization */

  cairo_move_to(cr, start_x, start_y );

  /* sets the ending point of the visualization */

  cairo_line_to(cr, end_x, end_y);

  /* draws a border for the visualization */

  cairo_stroke(cr);

} /* draw_line() */

/*
 * This function draws a note in the specified area using the given parameters
 * in the GUI
 */

void draw_note(GtkDrawingArea *area, cairo_t *cr, int height, int note,
               int start_time, int end_time, GdkRGBA *color) {

  /* subtracts the highest and lowest notes to get the note range of the song */

  float note_range = (float)(g_parameters.highest_note -
                             g_parameters.lowest_note);

  /* holds the note to be drawn on the GUI */

  float float_note = (float)(g_parameters.highest_note - note);

  /* holds the starting place of the note in the GUI */

  float float_start_time = ((float)start_time) /
                            ((float)g_parameters.time_scale);

  /* holds the ending place of the note in the GUI */

  float float_end_time = ((float)end_time) /
                          ((float)g_parameters.time_scale);

  /* holds the height of the note */

  float float_y = (float_note / note_range) * height;

  /* holds the height of the note in the GUI */

  int y = (int)(float_y);

  /* draws the note into the GUI */

  draw_line(area, cr, (int)(float_start_time), y, (int)(float_end_time), y,
            color);

} /* draw_note() */

/*
 * This function draws a track of a song into the GUI
 */

void draw_track(GtkDrawingArea *area, cairo_t *cr, int height, track_t *track) {

  /* holds an array to mimic all notes */

  int note_start_time[128];

  /* holds the value of the note */

  int note_value;

  /* holds the speed of the note */

  int note_velocity;

  /* loops until i is the same as 128 */

  for (int i = 0; i < 128; i++) {

    /* initializes the array */

    note_start_time[i] = NOTE_NOT_USED;
  }

  /* holds the current time in the song */

  int current_time = 0;

  /* holds the current instrument in the song */

  uint8_t instrument = 0;

  /* holds the event_list of track */

  event_node_t *event_list = track->event_list;

  /* loops until event_list reaches the end of the list */

  while (event_list != NULL) {

    /* holds the current event of event_list */

    event_t *event = event_list->event;

    /* adds event->delta_time to current_time */

    current_time = current_time + event->delta_time;

    /* checks if event is a midi event */

    if (event_type(event) == MIDI_EVENT_T) {

      /* checks if the status byte is in the correct range */

      if ((event->midi_event.status >= 0xC0) &&
          (event->midi_event.status <= 0xCF)) {

        /* makes instrument the same as event->midi_event.data[0] */

        instrument = event->midi_event.data[0];
      }

      /* checks if the status byte is in the correct range */

      else if ((event->midi_event.status >= 0x90) &&
               (event->midi_event.status <= 0x9F)) {

        /* makes note_value the same as event->midi_event.data[0] */

        note_value = event->midi_event.data[0];

        /* makes note_velocity the same as event->midi_event.data[1] */

        note_velocity = event->midi_event.data[1];

        /* checks if note_velocity is 0 */

        if (note_velocity == 0) {

          /* checks if note_start_time[note_value] is not the same as */
          /* NOTE_NOT_USED */

          if (note_start_time[note_value] != NOTE_NOT_USED) {

            /* draws the note on to the GUI */

            draw_note(area, cr, height, note_value,
                      note_start_time[note_value], current_time,
                      COLOR_PALETTE[instrument]);
          }

          /* makes note_start_time[note_value] the same as NOTE_NOT_USED */

          note_start_time[note_value] = NOTE_NOT_USED;
        }
        else {

          /* checks if note_start_time[note_value] is the same as */
          /* NOTE_NOT_USED */

          if (note_start_time[note_value] == NOTE_NOT_USED) {

            /* makes note_start_time[note_value] the same as current_time */

            note_start_time[note_value] = current_time;
          }
        }
      }

      /* checks if status byte is in the correct range */

      else if ((event->midi_event.status >= 0x80) &&
               (event->midi_event.status <= 0x8F)) {

        /* makes note_value the same as event->midi_event.data[0] */

        note_value = event->midi_event.data[0];

        /* checks if note_start_time[note_value] is not the same as */
        /* NOTE_NOT_USED */

        if (note_start_time[note_value] != NOTE_NOT_USED) {

          /* draws the note on to the GUI */

          draw_note(area, cr, height, note_value, note_start_time[note_value],
                    current_time, COLOR_PALETTE[instrument]);          
        }

        /* makes note_start_time[note_value] the same as NOTE_NOT_USED */

        note_start_time[note_value] = NOTE_NOT_USED;
      }
    }

    /* moves to the next element in the list */

    event_list = event_list->next_event;
  }

} /* draw_track() */

/*
 * This function draws all the notes on a track in a song into the GUI
 */

void draw_song(GtkDrawingArea *area, cairo_t *cr, int height, song_data_t *song) {

  /* holds the color used to draw the line */

  GdkRGBA black = {0.0, 0.0, 0.0, 1.0};

  /* holds the track list of song */

  track_node_t *track_list = song->track_list;

  /* draws a line showing middle C's location */

  draw_note(area, cr, height, MIDDLE_C, 0, g_parameters.original_length, &black);

  /* loops until i is the same as song->num_tracks */

  for (int i = 0; i < song->num_tracks; i++) {

    /* draws the track into the GUI */

    draw_track(area, cr, height, track_list->track);

    /* moves to the next element in the list */

    track_list = track_list->next_track;
  }

} /* draw_song() */

/*
 * This function check which note in the song is the lowest note
 */

int checking_events_for_lowest_note(song_data_t *song) {

  /* holds the lowest note in the song */

  int lowest_note = 127;

  /* holds the value of the current note */

  int note_value = 0;

  /* holds the track list of song */

  track_node_t *track_list = song->track_list;

  /* loops until track_list reaches the end of the list */

  while (track_list != NULL) {

    /* holds the event_list of track_list->track */

    event_node_t *event_list = track_list->track->event_list;

    /* loops until event_list reaches the end of the list */

    while (event_list != NULL) {

      /* holds the current event in event_list */

      event_t *event = event_list->event;

      /* checks if event is a midi event */

      if (event_type(event) == MIDI_EVENT_T) {

        /* checks if status byte is in the correct range */

        if (event->midi_event.status <= 0xAF) {

          /* makes note_value the same as event->midi_event.data[0] */

          note_value = event->midi_event.data[0];

          /* checks if note_value is less than lowest_note */

          if (note_value < lowest_note) {

            /* makes lowest_note the same as note_value */

            lowest_note = note_value;
          }
        }
      }

      /* moves to the next element in the list */

      event_list = event_list->next_event;
    }

    /* moves to the next element in the list */

    track_list = track_list->next_track;
  }
  return lowest_note;

} /* checking_event_for_lowest_note() */

/*
 * This function check which note in the song is the highest note
 */

int checking_events_for_highest_note(song_data_t *song) {

  /* holds the highest note in the song */

  int highest_note = 0;

  /* holds the value of the current note */

  int note_value = 0;

  /* holds the track list of song */

  track_node_t *track_list = song->track_list;

  /* loops until track_list reaches the end of the list */

  while (track_list != NULL) {

    /* holds the event_list of track_list->track */

    event_node_t *event_list = track_list->track->event_list;

    /* loops until event_list reaches the end of the list */

    while (event_list != NULL) {

      /* holds the current event in event_list */

      event_t *event = event_list->event;

      /* holds the current event in event_list */

      if (event_type(event) == MIDI_EVENT_T) {

        /* checks if status byte is in the correct range */

        if (event->midi_event.status <= 0xAF) {

          /* makes note_value the same as event->midi_event.data[0] */

          note_value = event->midi_event.data[0];

          /* checks if note_value is greater than highest_note */

          if (note_value > highest_note) {

            /* makes highest_note the same as note_value */

            highest_note = note_value;
          }
        }
      }

      /* moves to the next element in the list */

      event_list = event_list->next_event;
    }

    /* moves to the next element in the list */

    track_list = track_list->next_track;
  }
  return highest_note;

} /* checking_events_for_highest_note() */

/*
 * This function adds up the delta time of all events in the given song
 */

uint32_t finding_song_length(song_data_t *song) {

  /* holds the total delta time of the song */

  uint32_t song_length = 0;

  /* holds the total delta time of the events in a track */

  uint32_t time = 0;

  /* holds the track list of song */

  track_node_t *track_list = song->track_list;

  /* loops until track_list reaches the end of the list */

  while (track_list != NULL) {

    /* holds the event_list of track_list->track */

    event_node_t *event_list = track_list->track->event_list;

    /* loops until event_list reaches the end of the list */

    while (event_list != NULL) {

      /* holds the current event in event_list */

      event_t *event = event_list->event;

      /* adds event->delta_time to time */

      time = time + event->delta_time;

      /* checks if time is greater than song_length */

      if (time > song_length) {

        /* makes song_length the same as time */

        song_length = time;
      }

      /* moves to the next element in the list */

      event_list = event_list->next_event;
    }

    /* sets time to 0 */

    time = 0;

    /* moves to the next element in the list */

    track_list = track_list->next_track;
  }
  return song_length;

} /* finding_song_length() */

/*
 * This function finds the node who has the same char * as the parameter
 */

tree_node_t *find_node_pointer(tree_node_t *parent_node,
                                  const char *song_name) {

  /* checks if parent_node is null */

  if (parent_node == NULL) {
    return NULL;
  }

  /* holds the result of using strcmp on the song names */

  int strcmp_result = strcmp(song_name, parent_node->song_name);

  /* checks if strcmp_result is 0 */

  if (strcmp_result == 0) {
    return parent_node;
  }

  /* checks if strcmp_result is less than 0 */

  if (strcmp_result < 0) {
    return find_node_pointer((parent_node->left_child), song_name);
  }
  else {
    return find_node_pointer((parent_node->right_child), song_name);
  }
  return NULL;

} /* find_node_pointer() */

/*
 * This function removes all songs from the GTK_CONTAINER
 */

void erase_song_list_box() {

  /* holds the songs currently shown in the GUI */

  GList *children = gtk_container_get_children(GTK_CONTAINER(g_widgets.song_list));

  /* holds the current widget in the list */

  GList *iter;

  /* loops until iter is null */

  for (iter = children; iter != NULL; iter = g_list_next(iter)) {

    /* erases the widget from the GUI */

    gtk_widget_destroy(GTK_WIDGET(iter->data));
  }

  /* frees memory allocated to children */

  g_list_free(children);

} /* erase_song_list_box() */

/*
 * This function adds the name of a song to the GUI
 */

GtkWidget *add_song_name_to_list_box(char *text) {

  /* creates a label based on the parameter */

  GtkWidget *label = gtk_label_new(text);

  /* adds the label to the GUI */

  gtk_widget_set_halign(label, GTK_ALIGN_START);  
  return label;

} /* add_song_name_to_list() */

/*
 * This function adds a gtk_list_box of songs to the GUI
 */

void add_song_to_list_box(tree_node_t *node, void *data) {

  /* adds a song name to a box */

  GtkWidget *row = add_song_name_to_list_box(node->song_name);

  /* shows the song name in the GUI */

  gtk_list_box_insert(GTK_LIST_BOX(g_widgets.song_list), row, -1);

} /* add_song_to_list_box() */

/*
 * This function takes the songs from g_parameter.song_library and adds them
 * to a GTK_LIST_BOX
 */

void add_songs_from_library_to_song_list_box() {

  /* adds the songs in g_parameters.song_library to a box */

  traverse_in_order(g_parameters.song_library, NULL, (traversal_func_t)add_song_to_list_box);

  /* holds the amount of song names in the box */

  int count = 0;

  /* holds the songs currently shown in the GUI */

  GList *children = gtk_container_get_children(GTK_CONTAINER(g_widgets.song_list));

  /* holds the current widget in the list */

  GList *iter;

  /* holds the current row of the box */

  GtkWidget *row;

  /* loops until iter is null */

  for (iter = children; iter != NULL; iter = g_list_next(iter)) {

    /* increaes count by 1 */

    count++;
  }

  /* loops until i is the same as LIST_LENGTH */

  for (int i = count; i < LIST_LENGTH; i++) {

    /* adds an empty row to the box */

    row = add_song_name_to_list_box("");

    /* shows the row in the GUI */

    gtk_list_box_insert(GTK_LIST_BOX(g_widgets.song_list), row, -1);
  }

  /* shows the box in the GUI */

  gtk_widget_show_all(g_widgets.window);

} /* add_songs_from_library_to_song_list_box() */

/*
 * This function adds a song to the song library in g_parameters
 */

void add_song_to_library(char *path) {

  /* allocates space in memory for tree_node_t struct */

  tree_node_t *new_node = (tree_node_t *)malloc(sizeof(tree_node_t));

  /* parses path and adds the information to new_node->song */

  new_node->song = parse_file(path);

  /* gets the length of only the song name in path */

  int song_length = strlen(basename(path));

  /* holds the length of new_node->song->path */

  int path_length = strlen(new_node->song->path);

  /* makes new_node->song_name the same as */
  /* new_node->song->path + path_length - song_length */

  new_node->song_name = new_node->song->path + path_length - song_length;

  /* sets new_node->left_child to null */

  new_node->left_child = NULL;

  /* sets new_node->right_child to null */

  new_node->right_child = NULL;

  /* inserts new_node into the tree */

  tree_insert(&(g_parameters.song_library), new_node);

} /* add_song_to_library() */

/*
 * This function takes a song path and inserts the song name into
 * g_parameters.song_library
 */

int tree_insert_file_from_gui(const char *path, const struct stat *sptr, int type) {

  /* holds the result of whether a song was successfully inserted into */
  /* g_parameters.song_library */

  int result_code = INSERT_SUCCESS;

  /* checks if type is not FTW_D */

  if (type != FTW_D) {

    /* holds the length of path */

    int song_length = strlen(path);

    /* checks if the last 3 characters of path are 'mid' */

    if ((path[song_length - 3] == 'm') &&
        (path[song_length - 2] == 'i') &&
        (path[song_length - 1] == 'd')) {

      /* allocates space in memory for tree_node_t struct */

      tree_node_t *new_node = (tree_node_t *)malloc(sizeof(tree_node_t));

      /* parses path and adds the information to new_node->song */

      new_node->song = parse_file(path);

      /* gets the length of only the song name in path */

      song_length = strlen(basename(path));

      /* holds the length of new_node->song->path */

      int path_length = strlen(new_node->song->path);

      /* makes new_node->song_name the same as */
      /* new_node->song->path + path_length - song_length */

      new_node->song_name = new_node->song->path + path_length - song_length;

      /* sets new_node->left_child to null */

      new_node->left_child = NULL;

      /* sets new_node->right_child to null */

      new_node->right_child = NULL;

      /* attempts to insert the node into the tree */

      result_code = tree_insert(&g_parameters.song_library, new_node);
    }
  }
  return result_code;

}/* tree_insert_file_from_gui() */

/*
 * This function adds all MIDI files in the directory to
 * g_parameters.song_library
 */

void make_library_from_folder(const char *directory) {

  /* inserts songs from directory into g_parameters.song_library */

  int result = ftw(directory, tree_insert_file_from_gui, 7);

  /* confirms that result is not the same as DUPLICATE_SONG */

  assert(result != DUPLICATE_SONG);

} /* make_library_from_folder() */

/*
 * This function updates the song list in the GUI
 */

void update_song_list() {

} /* update_song_list() */

/*
 * This function updates the song visualization in the GUI
 */

void update_drawing_area() {

  /* draws the visualization of the original song */

  gtk_widget_queue_draw(g_widgets.original_song_drawing_area);

  /* draws the visualization of the modified song */

  gtk_widget_queue_draw(g_widgets.effect_drawing_area);

} /* update_drawing_area() */

/*
 * This function updates the contents of the GUI
 */

void update_info() {

  /* gets the lowest and highest notes of the song along with its length */

  range_of_song(g_parameters.song_library->song, &g_parameters.lowest_note,
                &g_parameters.highest_note, &g_parameters.original_length);

  /* allocates space in memory for char * */

  char *name_title = (char *)malloc(strlen(g_current_node->song_name) + 12);

  /* copies the string into name_title */

  strcpy(name_title, "File name: ");

  /* adds g_current_node->song_name to the end of name_title */

  strcat(name_title, g_current_node->song_name);

  /* adds name_title to the GUI */

  gtk_label_set_text((GtkLabel *)g_widgets.file_name, name_title);

  /* holds the path of g_current_node->song */

  const char *full_path = g_current_node->song->path;

  /* allocates space in memory for char * */

  char *path_title = (char *)malloc(strlen(full_path) + 12);

  /* copies the string into path_title */

  strcpy(path_title, "Full path: ");

  /* adds full_path to the end of path_title */

  strcat(path_title, full_path);

  /* adds path_title to the GUI */

  gtk_label_set_text((GtkLabel *)g_widgets.file_path, path_title);

  /* holds the note range of the song */

  char range[20];

  /* copies the string into range */

  sprintf(range, "Note range: [%d, %d]", g_parameters.lowest_note,
                                         g_parameters.highest_note);

  /* adds range to the GUI */

  gtk_label_set_text((GtkLabel *)g_widgets.note_range, range);

  /* holds the length of the song */

  char length[30];

  /* copies the string into length */

  sprintf(length, "Original length: %d", g_parameters.original_length);

  /* adds length to the GUI */

  gtk_label_set_text((GtkLabel *)g_widgets.original_length, length);

  /* frees memory allocated to name_title */

  free(name_title);

  /* frees memory allocated to path_title */

  free(path_title);

} /* update_info() */

/*
 * This function modifies a song and saves the result in g_modified_song
 */

void update_song() {

  /* copies the current song into g_modified_song */

  copy_song();

  /* changes the speed of the song */

  warp_time(g_modified_song, (float)(g_parameters.warp_time));

  /* changes the modified song visualization in the GUI */

  update_drawing_area();

} /* update_song() */

/*
 * This function finds the highest and lowests notes in the song along with its
 * length
 */

void range_of_song(song_data_t *song, int *lowest_note, int *highest_note,
                   int *original_length) {

  /* holds a pointer to the lowest note in the song */

  *lowest_note = checking_events_for_lowest_note(g_current_node->song);

  /* holds a pointer to the highest note in the song */

  *highest_note = checking_events_for_highest_note(g_current_node->song);

  /* holds a pointer to the length of the song */

  *original_length = finding_song_length(g_current_node->song);

} /* range_of_song() */

/*
 * This function activates the GUI
 */

void activate(GtkApplication* app, gpointer user_data) {

  /* holds the areas to add buttons */

  GtkWidget *grid;

  /* holds the row to add buttons to */

  GtkWidget *row;

  /* sets g_parameters.song_library to NULL */

  g_parameters.song_library = NULL;

  /* sets g_parameters.warp_time to 1 */

  g_parameters.warp_time = 1;

  /* sets g_parameters.drawing_height to -1 */

  g_parameters.drawing_height = -1;

  /* sets g_current_note to null */

  g_current_node = NULL;

  /* sets g_modified_song to null */

  g_modified_song = NULL;

  /* sets parameters of gtk_init to null */

  gtk_init(NULL, NULL);

  /* creates the window for the GUI */

  g_widgets.window = gtk_application_window_new(app);

  /* sets a title for the GUI */

  gtk_window_set_title(GTK_WINDOW(g_widgets.window), "MIDI Library");

  /* creates a grid to add buttons to */

  grid = gtk_grid_new();

  /* sets the spacing of the columns in the grid */

  gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

  /* sets the spacing of the rows in the grid */

  gtk_grid_set_row_spacing (GTK_GRID(grid), 0);

  /* creates a button that adds a song */

  GtkWidget* add_song_button = gtk_button_new_with_label("Add Song From File");

  /* connects the button to the GUI */

  g_signal_connect(add_song_button, "clicked", G_CALLBACK(add_song_cb),
                   g_widgets.window);

  /* attaches button to the grid */

  gtk_grid_attach(GTK_GRID(grid), add_song_button, 1, 1, 1, 1);

  /* creates a button that loads a directory */

  GtkWidget* load_song_button =
                              gtk_button_new_with_label("Load from Directory");

  /* connects the button to the GUI */

  g_signal_connect(load_song_button, "clicked",
                   G_CALLBACK(load_songs_cb), NULL);

  /* attaches button to the grid */

  gtk_grid_attach(GTK_GRID(grid), load_song_button, 2, 1, 1, 1);

  /* makes g_widgets.song_list the same as gtk_list_box_new() */

  g_widgets.song_list = gtk_list_box_new();

  /* attaches the box to the grid */

  gtk_grid_attach(GTK_GRID(grid), g_widgets.song_list, 1, 2, 2, 29);

  /* loops until i is the same as 29 */

  for (int i = 0; i < LIST_LENGTH; i++) {

    /* checks if i is not the same as MIDDLE_OF_LIST */

    if (i != MIDDLE_OF_LIST) {

      /* makes row an empty string */

      row = add_song_name_to_list_box("");
    }
    else {

      /* adds a string to current row */

      row = add_song_name_to_list_box("             Load files to start");
    }

    /* inserts current row to bottom of list box */

    gtk_list_box_insert (GTK_LIST_BOX(g_widgets.song_list), row, -1);
  }

  /* connects button to GUI */

  g_signal_connect(g_widgets.song_list, "row-selected",
                   G_CALLBACK(song_selected_cb), NULL);

  /* creates a button that adds a search bar to the GUI */

  GtkWidget* search_bar = gtk_search_entry_new();

  /* attaches button to grid */

  gtk_grid_attach(GTK_GRID(grid), search_bar, 1, 32, 2, 1);

  /* adds current grid to GUI */

  gtk_container_add(GTK_CONTAINER(g_widgets.window), GTK_WIDGET(grid));

  /* sets g_widgets.file_name to the given string */

  g_widgets.file_name = gtk_label_new("Select a file from list to start...");

  /* sets the position of g_widgets.file_name in the GUI */

  gtk_widget_set_halign(g_widgets.file_name, GTK_ALIGN_START);

  /* attaches g_widgets.file_name to the grid */

  gtk_grid_attach(GTK_GRID(grid), g_widgets.file_name, 4, 1, 5, 1);

  /* sets g_widgets.file_path to an empty string */

  g_widgets.file_path = gtk_label_new("");

  /* sets the position of g_widgets.file_path in the GUI */

  gtk_widget_set_halign(g_widgets.file_path, GTK_ALIGN_START);

  /* attaches g_widgets.file_path to the grid */

  gtk_grid_attach(GTK_GRID(grid), g_widgets.file_path, 4, 2, 5, 1); 

  /* sets g_widgets.note_range to an empty string */

  g_widgets.note_range = gtk_label_new("");

  /* sets the position of g_widgets.note_range in the GUI */

  gtk_widget_set_halign(g_widgets.note_range, GTK_ALIGN_START); 

  /* attaches g_widgets.note_range to the grid */

  gtk_grid_attach(GTK_GRID(grid), g_widgets.note_range, 4, 3, 2, 1); 

  /* sets g_widgets.original_length to an empty string */

  g_widgets.original_length = gtk_label_new("");

  /* sets the position of g_widgets.original_length in the GUI */

  gtk_widget_set_halign(g_widgets.original_length, GTK_ALIGN_START);

  /* attaches g_widgets.original_length to the grid */

  gtk_grid_attach(GTK_GRID (grid), g_widgets.original_length, 4, 4, 2, 1); 

  /* creates an empty column */

  GtkWidget* blank_column = gtk_label_new(" ");

  /* attaches black_column to the grid */

  gtk_grid_attach(GTK_GRID(grid), blank_column, 10, 1, 1, 1);

  /* creates an empty column */

  GtkWidget* blank_column_2 = gtk_label_new(" ");

  /* attaches black_column_2 to the grid */

  gtk_grid_attach(GTK_GRID(grid), blank_column_2, 3, 1, 1, 1);

  /* sets g_parameters.time_scale to 1 */

  g_parameters.time_scale = 1;

  /* sets t_scale to the current string */

  GtkWidget* t_scale = gtk_label_new("T scale:");

  /* attaches t_scale to the grid */

  gtk_grid_attach(GTK_GRID(grid), t_scale, 5, 5, 1, 1);

  /* creates a button with a value that can change within a certain range */

  GtkWidget *time_spin_button = gtk_spin_button_new_with_range(MIN_RANGE,
                                MAX_RANGE, MIN_RANGE);

  /* attaches time_spin_button to grid */

  gtk_grid_attach(GTK_GRID(grid), time_spin_button, 6, 5, 3, 1);

  /* sets the initial value of time_spin_button */

  gtk_spin_button_set_value((GtkSpinButton *)time_spin_button, MIN_RANGE);

  /* connects the button to the GUI */

  g_signal_connect(time_spin_button, "value_changed",
                   G_CALLBACK(time_scale_cb), NULL);

  /* creates the frame to hold the original song visualization */

  GtkWidget *original_song_frame = gtk_frame_new("Original song");

  /* sets the frame used for original_song_frame */

  gtk_frame_set_shadow_type(GTK_FRAME(original_song_frame), GTK_SHADOW_IN);

  /* makes g_widgets.original_song_drawing_area the same as */
  /* gtk_drawing_area_new() */

  g_widgets.original_song_drawing_area = gtk_drawing_area_new();

  /* creates window that can be scrolled through */

  GtkWidget *original_song_drawing_window =
                                          gtk_scrolled_window_new (NULL, NULL);

  /* sets the scroll bar to stay in the position left by the user */

  gtk_scrolled_window_set_policy(
                             GTK_SCROLLED_WINDOW(original_song_drawing_window),
                             GTK_POLICY_ALWAYS, GTK_POLICY_NEVER);

  /* adds g_widgets.original_song_drawing_area to */
  /* original_song_drawing_window */

  gtk_container_add(GTK_CONTAINER(original_song_drawing_window),
                   g_widgets.original_song_drawing_area);

  /* connects button to the grid */

  g_signal_connect(G_OBJECT(g_widgets.original_song_drawing_area), "draw",
                   G_CALLBACK(draw_cb), NULL);

  /* adds original_song_drawing_window to original_song_frame */

  gtk_container_add(GTK_CONTAINER(original_song_frame),
                    original_song_drawing_window);

  /* attaches original_song_frame to grid */

  gtk_grid_attach(GTK_GRID(grid), original_song_frame, 4, 6, 5, 10);

  /* creates the frame to hold the modified song visualization */

  GtkWidget *effect_frame = gtk_frame_new ("After effect");

  /* sets the frame used for original_song_frame */

  gtk_frame_set_shadow_type(GTK_FRAME(effect_frame), GTK_SHADOW_IN);

  /* makes g_widgets.effect_drawing_area the same as gtk_drawing_area_new() */

  g_widgets.effect_drawing_area = gtk_drawing_area_new();

  /* creates window that can be scrolled through */

  GtkWidget *effect_drawing_window = gtk_scrolled_window_new (NULL, NULL);

  /* sets the scroll bar to stay in the position left by the user */

  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(effect_drawing_window),
                                 GTK_POLICY_ALWAYS, GTK_POLICY_NEVER);

  /* adds g_widgets.effect_drawing_area to effect_drawing_window */

  gtk_container_add(GTK_CONTAINER(effect_drawing_window),
                    g_widgets.effect_drawing_area);

  /* connects button to GUI */

  g_signal_connect(G_OBJECT(g_widgets.effect_drawing_area), "draw",
                   G_CALLBACK(draw_cb), NULL);

  /* adds effect_drawing_window to effect_frame */

  gtk_container_add(GTK_CONTAINER(effect_frame), effect_drawing_window);

  /* attaches effect_frame to grid */

  gtk_grid_attach(GTK_GRID(grid), effect_frame, 4, 17, 5, 10);

  /* creates a label with the given string */

  GtkWidget* warp_label = gtk_label_new("Warp Time:");

  /* attaches the label to the grid */

  gtk_grid_attach(GTK_GRID(grid), warp_label, 5, 28, 1, 1);

  /* creates a button with a value that can change within a certain range */

  GtkWidget *warp_spin_button = gtk_spin_button_new_with_range(MIN_RANGE,
                                MAX_RANGE, MIN_RANGE);

  /* attaches the button to the grid */

  gtk_grid_attach(GTK_GRID(grid), warp_spin_button, 6, 28, 3, 1);

  /* sets the inital value of the button */

  gtk_spin_button_set_value((GtkSpinButton *)warp_spin_button, MIN_RANGE);

  /* connects the button to the GUI */

  g_signal_connect(warp_spin_button, "value_changed", G_CALLBACK(warp_time_cb), NULL);

  /* creates a label with the given string */
  
  GtkWidget* octave_label = gtk_label_new("Change Octave:");

  /* attaches the label to the grid */

  gtk_grid_attach(GTK_GRID(grid), octave_label, 5, 29, 1, 1);

  /* creates a button with a value that can change within a certain range */ 

  GtkWidget *octave_spin_button = gtk_spin_button_new_with_range(MIN_RANGE,
                                  OCTAVE_MAX_RANGE, MIN_RANGE);

  /* attaches the button to the grid */

  gtk_grid_attach(GTK_GRID(grid), octave_spin_button, 6, 29, 3, 1);

  /* sets the inital value of the button */

  gtk_spin_button_set_value((GtkSpinButton *)octave_spin_button, 1);

  /* connects the button to the GUI */

  g_signal_connect(octave_spin_button, "value_changed", G_CALLBACK(song_octave_cb), NULL);

  /* creates a label with the given string */

  GtkWidget* instrument_label = gtk_label_new("Remap instruments:");

  /* attaches the label to the grid */

  gtk_grid_attach(GTK_GRID(grid), instrument_label, 5, 30, 1, 1);

  /* creates a drop down box for the widget */

  GtkWidget* instrument_combo_box = gtk_combo_box_text_new();

  /* attaches the box to the grid */

  gtk_grid_attach(GTK_GRID(grid), instrument_combo_box, 6, 30, 3, 1);

  /* creates a label with the given string */ 

  GtkWidget* note_label = gtk_label_new("Remap notes:");

  /* attaches the label to the grid */

  gtk_grid_attach(GTK_GRID(grid), note_label, 5, 31, 1, 1);

  /* creates a drop down box for the widget */

  GtkWidget* note_combo_box = gtk_combo_box_text_new();

  /* attaches the box to the grid */

  gtk_grid_attach(GTK_GRID(grid), note_combo_box, 6, 31, 3, 1);

  /* creates a button with with a label using the given string */

  GtkWidget* save_button = gtk_button_new_with_label("    Save Song       ");

  /* attaches the button to the grid */

  gtk_grid_attach(GTK_GRID(grid), save_button, 4, 32, 2, 1);

  /* creates a button with with a label using the given string */

  GtkWidget* remove_button = gtk_button_new_with_label("   Remove Song      ");

  /* attaches the button to the grid */

  gtk_grid_attach(GTK_GRID(grid), remove_button, 6, 32, 3, 1);

  /* creates a line with an empty string */

  GtkWidget* blank_line = gtk_label_new(" ");

  /* attaches blank_line to the grid */

  gtk_grid_attach(GTK_GRID(grid), blank_line, 1, 33, 9, 1);

  /* has everything attached to g_widgets.window appear on the GUI */

  gtk_widget_show_all(g_widgets.window);
 
} /* activate() */

/*
 * This function creates a button in the GUI that adds a selected song
 */

void add_song_cb(GtkButton *widget, gpointer user_data) {

  /* create a dialog box that has the user selected a file */

  GtkWidget* song = gtk_file_chooser_dialog_new("Choose a Song",
                                                 GTK_WINDOW(user_data),
                                                 GTK_FILE_CHOOSER_ACTION_OPEN,
                                                 ("Cancel"),
                                                 GTK_RESPONSE_CANCEL,
                                                 ("Open"),
                                                 GTK_RESPONSE_ACCEPT,
                                                 NULL);

  /* holds the name of the song */

  char *file_name;

  /* holds the file chosen by the user */

  GtkFileChooser *file;

  /* holds the response given after choosing the file */

  gint response = gtk_dialog_run(GTK_DIALOG(song));

  /* checks if response is the same as GTK_RESPONSE_ACCEPT */

  if (response == GTK_RESPONSE_ACCEPT) {

    /* makes file the same as GTK_FILE_CHOOSER(song) */

    file = GTK_FILE_CHOOSER(song);

    /* makes file_name the same as gtk_file_chooser_get_filename(file) */

    file_name = gtk_file_chooser_get_filename(file);

    /* adds file_name to the song list */

    add_song_to_library(file_name);

    /* erases all songs currently seen in the GUI */

    erase_song_list_box();

    /* adds the new song to the GUI */

    add_songs_from_library_to_song_list_box();
  }

  /* closes the dialog box */

  gtk_widget_destroy(song);

} /* add_song_cb() */

/*
 * This function creates a button that loads a directory of songs into the GUI
 */

void load_songs_cb(GtkButton *widget, gpointer user_data) {

  /* create a dialog box that has the user selected a folder */

  GtkWidget* directory = gtk_file_chooser_dialog_new("Choose a Directory",
                                         (GtkWindow *) g_widgets.window,
                                         GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                         ("Cancel"),
                                         GTK_RESPONSE_CANCEL,
                                         ("Open"),
                                         GTK_RESPONSE_ACCEPT,
                                         NULL);

  /* holds the name of the folder */

  char *folder_name;

  /* holds the folder chosen by the user */

  GtkFileChooser *folder;

  /* holds the response given after choosing the folder */

  gint response = gtk_dialog_run(GTK_DIALOG(directory));

  /* checks if response is the same as GTK_RESPONSE_ACCEPT */

  if (response == GTK_RESPONSE_ACCEPT) {

    /* makes folder the same as GTK_FILE_CHOOSER(song) */

    folder = GTK_FILE_CHOOSER(directory);

    /* makes file_name the same as gtk_file_chooser_get_filename(folder) */

    folder_name = gtk_file_chooser_get_filename(folder);

    /* creates a song library using the files in folder */

    make_library_from_folder(folder_name);

    /* erases all songs currently seen in the GUI */ 

    erase_song_list_box();

    /* adds the new songs to the GUI */

    add_songs_from_library_to_song_list_box();
  }

  /* closes the dialog box */

  gtk_widget_destroy(directory);

} /* load_songs_cb() */

/*
 * This function updates the GUI to show information about the selected song
 */

void song_selected_cb(GtkListBox *song_list_box, GtkListBoxRow *song_row) {

  /* checks if song_row is not null */

  if (song_row != NULL) {

    /* creates a label using song_row */

    GtkWidget *label = gtk_bin_get_child(GTK_BIN(song_row));

    /* holds the song name held in song_row */

    const char *text;

    /* gets the text held in label */

    text = gtk_label_get_label(GTK_LABEL(label));

    /* checks if text is not an empty string and if the strings do not match */

    if ((text[0] != '\0') &&
        (strcmp(text,"             Load files to start") != 0)) {

      /* sets g_current_node to the song found in song_row */

      g_current_node = find_node_pointer(g_parameters.song_library, text);

      /* updates the information in the GUI */

      update_info();

      /* updates the song shown in the GUI */

      update_song();
    }
  }

} /* song_selected_cb() */

/*
 * This function creates a button that updates the song list depending on the
 * searched keyword
 */

void search_bar_cb(GtkSearchBar * bar, gpointer data) {

} /* search_bar_cb() */

/*
 * This function creates a button that changes the time scale of the song
 */

void time_scale_cb(GtkSpinButton *button, gpointer data) {

  /* changes g_parameters.time_scale to the value received from the button */

  g_parameters.time_scale = gtk_spin_button_get_value(button);

  /* updates the song visualization in the GUI */

  update_drawing_area();

} /* time_scale_cb() */

/*
 * This function draws out an area in the GUI to hold the song visualization
 */

gboolean draw_cb(GtkDrawingArea *area, cairo_t *cr, gpointer data) {

  /* holds the width of the drawing area */

  int width;

  /* holds the height of the drawing area */

  int height;

  /* holds the style used for the drawing area */

  GtkStyleContext *context;

  /* holds the color used to fill the drawing area */

  GdkRGBA white = {1.0, 1.0, 1.0, 1.0};

  /* checks if g_parameters.drawing_height is -1 */

  if (g_parameters.drawing_height == -1) {

    /* makes g_parameters.drawing_height the same as */
    /* gtk_widget_get_allocated_height( */
    /* (GtkWidget *)g_widgets.original_song_drawing_area) */

    g_parameters.drawing_height = gtk_widget_get_allocated_height(
                            (GtkWidget *)g_widgets.original_song_drawing_area);
  }

  /* makes context the same as */
  /* gtk_widget_get_style_context((GtkWidget *)area) */

  context = gtk_widget_get_style_context((GtkWidget *)area);

  /* makes width the same as */
  /* gtk_widget_get_allocated_width((GtkWidget *)area) */

  width = gtk_widget_get_allocated_width((GtkWidget *)area);

  /* makes height the same as */
  /* gtk_widget_get_allocated_height((GtkWidget *)area) */

  height = gtk_widget_get_allocated_height ((GtkWidget *) area);

  /* sets the background of the drawing area */

  gtk_render_background(context, cr, 0, 0, width, height);

  /* sets the shape of the drawing area */

  cairo_rectangle(cr, 0, 0, width, height);

  /* sets the color of the drawing area */

  gdk_cairo_set_source_rgba (cr, &white);

  /* fills the drawing area  */

  cairo_fill(cr);

  /* checks if g_current_node is not null */

  if (g_current_node != NULL) {

    /* checks if area is the same as */
    /* (GtkDrawingArea *)g_widgets.original_song_drawing_area */

    if (area == (GtkDrawingArea *)g_widgets.original_song_drawing_area) {

      /* holds the width of the drawing area */

      int content_width = g_parameters.original_length / g_parameters.time_scale;

      /* sets the size of the drawing area */

      gtk_widget_set_size_request((GtkWidget *)area, content_width,
                                  g_parameters.drawing_height);

      /* makes height the same as */
      /* gtk_widget_get_allocated_height((GtkWidget *)area) */

      height = gtk_widget_get_allocated_height((GtkWidget *)area);

      /* creates the song visualization of the original song */

      draw_song(area, cr, height, g_current_node->song);
    }
  }

  /* checks if g_modified_song is not null */

  if (g_modified_song != NULL) {

    /* checks if area is the same as */
    /* (GtkDrawingArea *)g_widgets.effect_drawing_area */

    if (area == (GtkDrawingArea *)g_widgets.effect_drawing_area) {

      /* holds the width of the drawing area */

      int content_width = g_parameters.original_length/g_parameters.time_scale;

      /* sets the size of the drawing area */

      gtk_widget_set_size_request((GtkWidget *)area,content_width,
                                  g_parameters.drawing_height);

      /* makes height the same as */
      /* gtk_widget_get_allocated_height((GtkWidget *)area) */

      height = gtk_widget_get_allocated_height ((GtkWidget *) area);

      /* creates the song visualization of the modified song */

      draw_song(area,cr,height,g_modified_song);
    }
  }

 return FALSE;

} /* draw_cb() */

/*
 * This function creats a button that updates the speed of the song
 */

void warp_time_cb(GtkSpinButton *button, gpointer data) {

} /* warp_time_cb() */

/*
 * This function creates a button that updates the octave of all notes in the
 * song
 */

void song_octave_cb(GtkSpinButton *button, gpointer data) {

} /* song_octave_cb() */

/*
 * This function changes the instruments used in the song to a selected
 * instrument
 */

void instrument_map_cb(GtkComboBoxText *box, gpointer data) {

} /* instrument_map_cb() */

/*
 * This function remaps a chosen note in the GUI
 */

void note_map_cb(GtkComboBoxText *box, gpointer data) {

} /* note_map_cb() */

/*
 * This function creates a button that saves a modified song in the GUI to the
 * file it originally came from
 */

void save_song_cb(GtkButton *button, gpointer data) {

} /* save_song_cb() */

/*
 * This function creates a button that removes a selected song from the song
 * list
 */

void remove_song_cb(GtkButton *button, gpointer data) {

} /* remove_song_cb() */

/*
 * Function called prior to main that sets up the instrument to color mapping
 */

void build_color_palette()
{
  static GdkRGBA palette[16];	

  memset(COLOR_PALETTE, 0, sizeof(COLOR_PALETTE));
  char* color_specs[] = {
    // Piano, red
    "#ff0000",
    // Chromatic percussion, brown
    "#8b4513",
    // Organ, purple
    "#800080",
    // Guitar, green
    "#00ff00",
    // Bass, blue
    "#0000ff",
    // Strings, cyan
    "#00ffff",
    // Ensemble, teal
    "#008080",
    // Brass, orange
    "#ffa500",
    // Reed, magenta
    "#ff00ff",
    // Pipe, yellow
    "ffff00",
    // Synth lead, indigo
    "#4b0082",
    // Synth pad, dark slate grar
    "#2f4f4f",
    // Synth effects, silver
    "#c0c0c0",
    // Ehtnic, olive
    "#808000",
    // Percussive, silver
    "#c0c0c0",
    // Sound effects, gray
    "#808080",
  };

  for (int i = 0; i < 16; ++i) {
    gdk_rgba_parse(&palette[i], color_specs[i]);
    for (int j = 0; j < 8; ++j) {
      COLOR_PALETTE[i * 8 + j] = &palette[i];
    }
  }
} /* build_color_palette() */
