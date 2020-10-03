/* Name, alterations.c, CS 24000, Spring 2020
 * Last updated April 9, 2020
 */

/* Add any includes here */

#include "alterations.h"
#include <malloc.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#define MAX_CHANNELS (16)

/*
 * This function adds a delay to the track given as a parameter
 */

void add_delay(track_t *track, unsigned int delay) {

  /* checks if delay is not 0 */

  if (delay != 0) {

    /* adds delay to track->event_list->event->delta_time */

    track->event_list->event->delta_time = track->event_list->event->delta_time + delay;
  }

} /* add_delay() */

/*
 * This function assigns a channel to the track given as a parameter and
 * changes the bytes of all of its events
 */

void assign_channel_to_events_in_track(track_t *track, uint8_t channel) {

  /* holds the event_node_t struct of track */

  event_node_t *event_listing = track->event_list;

  /* loops until event_listing reaches the end of the list */

  while (event_listing != NULL) {

    /* checks if event_listing->event is a midi event */

    if (event_type(event_listing->event) == MIDI_EVENT_T) {

      /* checks if the status bytes are in the correct range */

      if ((event_listing->event->midi_event.status >= 0x80) &&
          (event_listing->event->midi_event.status <= 0xEF)) {

        /* changes the bytes depending on the bitwise operator */

        event_listing->event->midi_event.status =
                                event_listing->event->midi_event.status & 0xF0;

        /* changes the bytes depending on the bitwise operator */

        event_listing->event->midi_event.status =
                             event_listing->event->midi_event.status | channel;

        /* makes event_listing->event->type the same as */
        /* event_listing->event->midi_event.status */

        event_listing->event->type = event_listing->event->midi_event.status;
      }
    }

    /* moves to the next element in the list */

    event_listing = event_listing->next_event;
  }

} /* assign_channel_to_events_in_track() */

/*
 * This function finds the smallest unused channel in the song
 */

uint8_t find_smallest_unused_channel(song_data_t *song) {

  /* holds the lowest channel number not used in the song */

  uint8_t channel = 0;

  /* holds an array that checks if a channel is being used */

  bool channel_used[MAX_CHANNELS] = {false, false, false, false,
                                     false, false, false, false,
                                     false, false, false, false,
                                     false, false, false, false};

  /* holds the track_node_t struct of song->track_list */

  track_node_t *track_list = song->track_list;

  /* loops until track_list reaches the end of the list */

  while (track_list != NULL) {

    /* holds the event_node_t struct of track_list->track->event_list */

    event_node_t *event_list = track_list->track->event_list;

    /* loops until event_list reaches the end of the list */

    while (event_list != NULL) {

      /* checks if event_list->event is a midi event */

      if (event_type(event_list->event) == MIDI_EVENT_T) {

        /* checks if the status byte is within the correct range */

        if ((event_list->event->midi_event.status >= 0x80) &&
            (event_list->event->midi_event.status <= 0xDF)) {

          /* changes the byte depending on the bitwise operator used */

          channel = event_list->event->midi_event.status & 0x0F;

          /* shows that the current channel is being used */

          channel_used[channel] = true;
        }
      }

      /* moves to the next element in the list */

      event_list = event_list->next_event;
    }

    /* moves to the next element in the list */

    track_list = track_list->next_track;
  }

  /* makes channel the same as MAX_CHANNELS + 1 */

  channel = MAX_CHANNELS + 1;

  /* loops until i is the same as MAX_CHANNEL */

  for (int i = 0; i < MAX_CHANNELS; i++) {

    /* checks if channel_used[i] is false and if channel has not changed */

    if ((channel_used[i] == false) && (channel == (MAX_CHANNELS + 1))){

      /* makes channel the same as (uint8_t) i */

      channel = (uint8_t) i;
    }
  }
  return channel;

} /* find_smallest_unused_channel() */

/*
 * This function creates adds meta event information to the event given as a
 * parameter
 */

event_t *copy_meta_event(event_t *event) {

  /* allocates space in memory for event_t struct */

  event_t *new_event = (event_t *)malloc(sizeof(event_t));

  /* makes new_event->type the same as event->type */

  new_event->type = event->type;

  /* makes new_event->meta_event.name the same as event->meta_event.name */

  new_event->meta_event.name = event->meta_event.name;

  /* makes new_event->meta_event.data_len the same as */
  /* event->meta_event.data_len */

  new_event->meta_event.data_len = event->meta_event.data_len;

  /* sets data to null */

  uint8_t *data = NULL;

  /* checks if new_event->meta_event.data_len is greater than 0 */

  if (new_event->meta_event.data_len > 0) {

    /* allocates space in memory for uint8_t */

    data = (uint8_t *)malloc(event->meta_event.data_len * sizeof(uint8_t));

    /* loops until i is the same as new_event->meta_event.data_len */

    for (int i = 0; i < new_event->meta_event.data_len; i++) {

      /* makes data[i] the same as event->meta_event.data[i] */

      data[i] = event->meta_event.data[i];
    }

    /* makes new_event->meta_event.data the same as data */

    new_event->meta_event.data = data;
  }
  else {

    /* sets new_event->meta_event.data to null */

    new_event->meta_event.data = NULL;
  }
  return new_event;

} /* copy_meta_event() */

/*
 * This function creates adds sys event information to the event given as a
 * parameter
 */

event_t *copy_sys_event(event_t *event) {

  /* allocates space in memory for event_t struct */

  event_t *new_event = (event_t *)malloc(sizeof(event_t));

  /* makes new_event->type the same as event->type */

  new_event->type = event->type;

  /* makes new_event->sys_event.data_len the same as */
  /* event->sys_event.data_len */

  new_event->sys_event.data_len = event->sys_event.data_len;

  /* allocates space in memory for uint8_t */

  uint8_t *data = (uint8_t *)malloc(event->sys_event.data_len * sizeof(uint8_t));

  /* loops until is the same as new_event->sys_event.data_len */

  for (int i = 0; i < new_event->sys_event.data_len; i++) {

    /* makes data[i] the same as event->sys_event.data[i] */

    data[i] = event->sys_event.data[i];
  }

  /* makes new_event->sys_event.data the same as data */

  new_event->sys_event.data = data;
  return new_event;

} /* copy_sys_event() */

/*
 * This function creates adds midi event information to the event given as a
 * parameter
 */

event_t *copy_midi_event(event_t *event) {

  /* allocates space in memory for event_t struct */

  event_t *new_event = (event_t *)malloc(sizeof(event_t));

  /* makes new_event->type the same as event->type */

  new_event->type = event->type;

  /* makes new_event->midi_event.name the same as event->midi_event.name */

  new_event->midi_event.name = event->midi_event.name;

  /* makes new_event->midi_event.status the same as event->midi_event.status */

  new_event->midi_event.status = event->midi_event.status;

  /* makes new_event->midi_event.data_len the same as */
  /* event->midi_event.data_len */

  new_event->midi_event.data_len = event->midi_event.data_len;

  /* allocates space in memory for uint8_t struct */

  uint8_t *data = (uint8_t *)malloc(event->midi_event.data_len * sizeof(uint8_t));

  /* loops until i is the same as new_event->midi_event.data_len */

  for (int i = 0; i < new_event->midi_event.data_len; i++) {

    /* makes data[i] the same as event->midi_event.data[i] */

    data[i] = event->midi_event.data[i];
  }

  /* makes new_event->midi_event.data the same as data */

  new_event->midi_event.data = data;
  return new_event;

} /* copy_midi_event() */

/*
 * This function creates an event_t struct and adds information depending on
 * what type of event it is
 */

event_t *copy_event(event_t *event) {

  /* holds the event_t struct that will be copied to track */

  event_t *new_event = {0};

  /* checks if event is a meta event */

  if (event_type(event) == META_EVENT_T) {

    /* copies event into new_event */

    new_event = copy_meta_event(event);
  }

  /* checks if event is a sys event */

  else if (event_type(event) == SYS_EVENT_T) {

    /* copies event into new_event */

    new_event = copy_sys_event(event);
  }
  else {

    /* copies event into new_event */

    new_event = copy_midi_event(event);
  }

  /* makes new_event->delta_time the same as event->delta_time */

  new_event->delta_time = event->delta_time;
  return new_event;

} /* copy_event() */

/*
 * This function creates a track_t struct and copies information from the track
 * given as a parameter
 */

track_t *copy_track(track_t *track) {

  /* allocates space in memory for track_t struct */

  track_t *new_track = (track_t *)malloc(sizeof(track_t));

  /* makes new_track->length the same as track->length */

  new_track->length = track->length;

  /* sets new_track->event_list to null */

  new_track->event_list = NULL;

  /* holds all of the events to be copied to track */

  event_node_t *event_listing = NULL;

  /* holds the event to be added to event_listing */

  event_t *new_event = {0};

  /* holds the current last event in event_listing */

  event_node_t *last_event = NULL;

  /* holds the event_list of track */

  event_node_t *song_event_list = track->event_list;

  /* loops until song_event_list reaches the end of the list */

  while (song_event_list != NULL) {

    /* allocates space in memory for event_node_t struct */

    event_listing = (event_node_t *)malloc(sizeof(event_node_t));

    /* copies song_event_list->event into new_event */

    new_event = copy_event(song_event_list->event);

    /* makes event_listing->event the same as new_event */

    event_listing->event = new_event;

    /* sets event_listing->next_event to null */

    event_listing->next_event = NULL;

    /* checks if new_track->event_list is null */

    if (new_track->event_list == NULL) {

      /* makes new_track->event_list the same as event_listing */

      new_track->event_list = event_listing;
    }
    else {

      /* makes last_event->next_event the same as event_listing */

      last_event->next_event = event_listing;
    }

    /* makes last_event the same as event_listing */

    last_event = event_listing;

    /* moves to the next element in the list */

    song_event_list = song_event_list->next_event;
  }

  /* sets last_event->next_event to null */

  last_event->next_event = NULL;

  return new_track;

} /* copy_track() */

/*
 * This function changes the instrument of an event depending on the uint8_t
 * given as a parameter
 */

int change_single_instrument(event_t *event, uint8_t *instrument) {

  /* changes if event was modified */

  int modification_result = 0;

  /* checks if event is a midi event */

  if (event_type(event) == MIDI_EVENT_T) {

    /* checks if status byte is in the correct range */

    if ((event->midi_event.status >= 0xC0) &&
        (event->midi_event.status <= 0xCF)) {

      /* makes event->midi_event.data[0] the same as *instrument */

      event->midi_event.data[0] = *instrument;

      /* sets modification_result to 1 */

      modification_result = 1;
    }
  }
  return modification_result;

} /* change_single_instrument() */

/*
 * This function changes the instrument of all events in the song depending on
 * the uint8_t given as a parameter
 */

int remap_single_instrument(song_data_t *song, uint8_t instrument) {

  /* holds the amount of events modified in the song */

  int modified_events = 0;

  /* modifies events in song */

  modified_events = apply_to_events(song, (event_func_t) change_single_instrument, &instrument);
  return modified_events;

} /* remap_single_instrument() */

/*
 * This function counts the amount of bytes in the given parameter
 */

int count_variable_time_bytes(uint32_t time) {

  /* holds number of bytes in parameter */

  int num_bytes = 0;

  /* checks if time is less than 0x80 */

  if (time < 0x80) {

    /* sets num_bytes to 1 */

    num_bytes = 1;
  }

  /* checks if time is less than 0x4000 */

  else if (time < 0x4000){

    /* sets num_bytes to 2 */

    num_bytes = 2;
  }

  /* checks if time is less than 0x200000 */

  else if (time < 0x200000){

    /* sets num_bytes to 3 */

    num_bytes = 3;
  }
  else {

    /* sets num_bytes to 4 */

    num_bytes = 4;
  }
  return num_bytes;

} /* count_variable_time() */

/*
 * This function changes the octave of a note in the event
 */

int change_event_octave(event_t *event, int *octave_amount) {

  /* changes if event was modified */

  int modification_result = 0;

  /* holds the modified note */

  int note_value = 0;

  /* checks if event is a midi event */

  if (event_type(event) == MIDI_EVENT_T) {

    /* checks if status byte is in the correct range */

    if (event->midi_event.status <= 0xAF) {

      /* adds (*octave_amount) * OCTAVE_STEP to event->midi_event.data[0] */

      note_value = event->midi_event.data[0] + (*octave_amount) * OCTAVE_STEP;

      /* checks if note_value is within the correct range */

      if ((note_value >= 0) && (note_value <= 127)) {

        /* makes event->midi_event.data[0] the same as (uint8_t)note_value */

        event->midi_event.data[0] = (uint8_t)note_value;

        /* sets modification_result to 1 */

        modification_result = 1;
      }
    }
  }
  return modification_result;

} /* change_event_octave() */

/*
 * This function changes the delta time of the event given as a parameter
 */

int change_event_time(event_t *event, float *multiplier) {

  /* holds the difference in bytes between the original and modified event */

  int byte_difference = 0;

  /* holds the original amount of bytes in event */

  int original_byte_total = 0;

  /* holds the new amount of bytes in event */

  int new_byte_total = 0;

  /* counts the amount of bytes in the event */

  original_byte_total = count_variable_time_bytes(event->delta_time);

  /* multiplies mulitplier and event->delta_time */

  event->delta_time = event->delta_time * (*multiplier);

  /* counts the amount of bytes in the event */

  new_byte_total = count_variable_time_bytes(event->delta_time);

  /* subtracts new_byte_total and original_byte_total to get difference */

  byte_difference = new_byte_total - original_byte_total;
  return byte_difference;
}

/*
 * This function changes the instrument of the event given as a parameter
 */

int change_event_instrument(event_t *event, remapping_t instrument_table) {

  /* changes if event was modified */

  int modification_result = 0;

  /* checks if event is a midi event */

  if (event_type(event) == MIDI_EVENT_T) {

    /* checks if status byte is in the correct range */

    if ((event->midi_event.status >= 0xC0) &&
        (event->midi_event.status <= 0xCF)) {

      /* makes event->midi_event.data[0] the same as */
      /* instrument_table[event->midi_event.data[0]] */

      event->midi_event.data[0] = instrument_table[event->midi_event.data[0]];

      /* sets modification_result to 1 */

      modification_result = 1;
    }
  }
  return modification_result;

} /* change_event_instrument() */

/*
 * This function changes the note of the event to the given parameter
 */

int change_event_note(event_t *event, remapping_t note_table) {

  /* changes if event was modified */

  int modification_result = 0;

  /* checks if event is a midi event */

  if (event_type(event) == MIDI_EVENT_T) {

    /* checks if status byte is in the correct range */

    if (event->midi_event.status <= 0xAF) {

      /* makes event->midi_event.data[0] the same as */
      /* note_table[event->midi_event.data[0]] */

      event->midi_event.data[0] = note_table[event->midi_event.data[0]];

      /* sets modification_result to 1 */

      modification_result = 1;
    }
  }
  return modification_result;

} /* change_event_note() */

/*
 * This function uses the function given as a parameter and applies it to all
 * events in the song given as a parameter
 */

int apply_to_events(song_data_t *song, event_func_t my_function, void *data) {

  /* holds the amount of events modified in song */

  int return_value = 0;

  /* holds the track_node_t struct of song->track_list */

  track_node_t *track_list = song->track_list;

  /* loops until track_list reaches the end of the list */

  while (track_list != NULL) {

    /* holds the event_node_t struct of track_list->track->event_list */

    event_node_t *event_list = track_list->track->event_list;

    /* loops until event_list reaches the end of the list */

    while (event_list != NULL) {

      /* adds my_function(event_list->event, data) to return_value */

      return_value = return_value + my_function(event_list->event, data);

      /* moves to the next element in the list */

      event_list = event_list->next_event;
    }

    /* moves to the next element in the list */

    track_list = track_list->next_track;
  }
  return return_value;

} /* apply_to_events() */

/*
 * This function changes the octave of all events in the song
 */

int change_octave(song_data_t *song, int octave_amount) {

  /* holds the amount of events modified in song */

  int modified_events = 0;

  /* checks if octave_amount is not 0 */

  if (octave_amount != 0) {

    /* modifies events in song */

    modified_events = apply_to_events(song, (event_func_t) change_event_octave, &octave_amount);
  }
  return modified_events;

} /* change_octave() */

/*
 * This function changes the length of the song by the float given as a
 * parameter
 */

int warp_time(song_data_t *song, float multiplier) {

  /* holds the total difference in bytes from the whole song */

  int total_byte_difference = 0;

  /* holds the difference in bytes from the track */

  int byte_difference = 0;

  /* allocates space in memory for song_data_t struct */

  song_data_t *temp_song = (song_data_t *)malloc(sizeof(song_data_t));

  /* makes temp_song->path the same as song->path */

  temp_song->path = song->path;

  /* makes temp_song->format the same as song->format */

  temp_song->format = song->format;

  /* sets temp_song->num_tracks to 1 */

  temp_song->num_tracks = 1;

  /* allocates space in memory for track_node_t struct */

  track_node_t *temp_track_list = (track_node_t *)malloc(sizeof(track_node_t));

  /* sets temp_track_list->next_track to null */

  temp_track_list->next_track = NULL;

  /* makes temp_song->track_list the same as temp_track_list */

  temp_song->track_list = temp_track_list;

  /* holds the track_node_t struct of song->track_list */

  track_node_t *song_track = song->track_list;

  /* holds a pointer to float parameter */

  float *multiplier_ptr = &multiplier;

  /* loops until i is the same as song->num_tracks */

  for (int i = 0; i < song->num_tracks; i++) {

    /* makes temp_track_list->track the same as song_track->track */

    temp_track_list->track = song_track->track;

    /* gets the difference in bytes between the original and new event */

    byte_difference = apply_to_events(temp_song,
                             (event_func_t) change_event_time, multiplier_ptr);

    /* adds byte_difference to temp_track_list->track->length */

    temp_track_list->track->length = temp_track_list->track->length +
                                     byte_difference;

    /* adds byte_difference to total_byte_difference */

    total_byte_difference = total_byte_difference + byte_difference;

    /* moves to the next element in the list */

    song_track = song_track->next_track;
  }

  /* frees memory allocated to temp_song */

  free(temp_song);

  /* sets temp_song to null */

  temp_song = NULL;

  /* frees memory allocated to temp_track_list */

  free(temp_track_list);

  temp_track_list = NULL;

  return total_byte_difference;

} /* warp_time() */

/*
 * This function changes all the instruments in the song to the given parameter
 */

int remap_instruments(song_data_t *song, remapping_t instrument_table) {

  /* holds the amount of events modified in song */

  int modified_events = 0;

  /* modifies events in song */

  modified_events = apply_to_events(song, (event_func_t) change_event_instrument, instrument_table);
  return modified_events;

} /* remap_instruments() */

/*
 * This function changes all notes in the song to the given parameter
 */

int remap_notes(song_data_t *song, remapping_t note_table) {

  /* holds the amount of events modified in song */

  int modified_events = 0;

  /* modifies events in song */

  modified_events = apply_to_events(song, (event_func_t) change_event_note, note_table);
  return modified_events;

} /* remap_notes() */

/*
 * This function copies a track in the song and modifies its information with
 * the given parameters and is added to the song
 */

void add_round(song_data_t *song, int track_index, int octave_amount,
               unsigned int delay, uint8_t instrument) {

  /* confirms that track_index is less than song->num_tracks */

  assert(track_index <= song->num_tracks);

  /* confirms that song->format is not 2 */

  assert(song->format != 2);

  /* holds the lowest channel number not used in song */

  uint8_t channel = find_smallest_unused_channel(song);

  /* confirms that channel is not (MAX_CHANNELS + 1) */

  assert(channel != (MAX_CHANNELS + 1));

  /* allocates space in memory for song_data_t struct */

  song_data_t *temp_song = (song_data_t *)malloc(sizeof(song_data_t));

  /* makes temp_song->path the same as song->path */

  temp_song->path = song->path;

  /* makes temp_song->format the same as song->format */

  temp_song->format = song->format;

  /* sets temp_song->num_tracks to 1 */

  temp_song->num_tracks = 1;

  /* allocates space in memory for track_node_t struct */

  track_node_t *new_track_list = (track_node_t *)malloc(sizeof(track_node_t));

  /* sets new_track_list->track to null */

  new_track_list->track = NULL;

  /* sets new_track_list->next_track to null */

  new_track_list->next_track = NULL;

  /* holds the track_node_t struct of song->track_list */

  track_node_t *temp_track_list = song->track_list;

  /* counts the number of tracks passed in while loop */

  int track_count = 0;

  /* loops while track_count is not the same as track_index */

  while (track_count != track_index) {

    /* moves to the next element in the list */

    temp_track_list = temp_track_list->next_track;

    /* increases track_count by 1 */

    track_count++;
  }

  /* copies temp_track_list->track into new_track_list->track */

  new_track_list->track = copy_track(temp_track_list->track);

  /* makes temp_song->track_list the same as new_track_list */

  temp_song->track_list = new_track_list;

  /* assigns the new track a channel number */

  assign_channel_to_events_in_track(new_track_list->track, channel);

  /* adds a delay to the new track */

  add_delay(new_track_list->track, delay);

  /* changes the octave of all notes in new track */

  change_octave(temp_song, octave_amount);

  /* changes all instruments in track */

  remap_single_instrument(temp_song, instrument);

  /* holds the track_node_t struct of song->track_list */

  track_node_t *track_list = song->track_list;

  /* loops until track_list reaches the end of the list */

  while (track_list->next_track != NULL) {

    /* moves to the next element in the list */

    track_list = track_list->next_track;
  }

  /* sets new_track_list->next_track to null */

  new_track_list->next_track = NULL;

  /* makes track_list->next_track the same as new_track_list */

  track_list->next_track = new_track_list;

  /* increases song->num_tracks by 1 */

  song->num_tracks++;

  /* frees memory allocated to temp_song */

  free(temp_song);

  /* sets temp_song to null */

  temp_song = NULL;

} /* add_round() */

/*
 * Function called prior to main that sets up random mapping tables
 */

void build_mapping_tables()
{
  for (int i = 0; i <= 0xFF; i++) {
    I_BRASS_BAND[i] = 61;
  }

  for (int i = 0; i <= 0xFF; i++) {
    I_HELICOPTER[i] = 125;
  }

  for (int i = 0; i <= 0xFF; i++) {
    N_LOWER[i] = i;
  }
  //  Swap C# for C
  for (int i = 1; i <= 0xFF; i += 12) {
    N_LOWER[i] = i-1;
  }
  //  Swap F# for G
  for (int i = 6; i <= 0xFF; i += 12) {
    N_LOWER[i] = i+1;
  }
} /* build_mapping_tables() */
