/* Name, parser.c, CS 24000, Spring 2020
 * Last updated March 27, 2020
 */

/* Add any includes here */

#include "parser.h"
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#define TIME_DIVISION_UPPER_MASK (0x8000)
#define TIME_DIVISION_LOWER_MASK (0x7FFF)
#define WORD_MASK_8 (0x00FF)
#define VAR_LENGTH_NEED_TO_READ_ANOTHER_BYTE (0x80)
#define VAR_LENGTH_MASK (0x7F)
#define MIDI_EVENT_LOWER_BOUND (0x80)
#define MIDI_EVENT_UPPER_BOUND (0xEF)

uint8_t g_last_status = 0;

/*
 * This function parses the path given as a parameter into a song_data_t struct
 */

song_data_t *parse_file(const char *path) {

  /* confirms that path is not null */

  assert(path != NULL);

  /* opens the file for reading */

  FILE *read_file = fopen(path, "r");

  /* confirms that read_file is not null */

  assert(read_file != NULL);

  /* moves to the end of the file */

  fseek(read_file, 0, SEEK_END);

  /* holds the total number of bytes in the file */

  int number_of_bytes = ftell(read_file);

  /* moves to the beginning of the file */

  fseek(read_file, 0, SEEK_SET);

  /* allocates space in memory for song_data_t struct */

  song_data_t *song = (song_data_t *)malloc(sizeof(song_data_t));

  /* allocates space in memory for char */

  song->path = (char *)malloc(strlen(path) + 1);

  /* deep copies path into song->path */

  strncpy(song->path, path, strlen(path) + 1);

  /* holds the total number of tracks in the file */

  song->num_tracks = 0;

  /* parses through file and determines the header format of song */

  parse_header(read_file, song);

  /* holds the different tracks in the song */

  song->track_list = NULL;

  /* holds the number of tracks read in the file */

  int num_tracks = 0;

  /* loops until the file is entirely read and all tracks were parsed */

  while ((ftell(read_file) < number_of_bytes) &&
         (num_tracks < song->num_tracks)) {

    /* parses through the file and determines the track format of song */

    parse_track(read_file, song);
    num_tracks++;
  }

  /* confirms that the entier file was read through */

  assert((ftell(read_file) - number_of_bytes) == 0);

  /* closes the file */

  fclose(read_file);

  /* sets read_file to null */

  read_file = NULL;
  return song;

} /* parse_file() */

/*
 * This function reads a MIDI file header chunk from the file and updates the
 * song_data_t struct with the information
 */

void parse_header(FILE *in_file, song_data_t *song) {

  /* holds the chunk id of head */

  char chunk_id[5] = "";

  /* counts number of bytes read from file */

  int bytes_read = 0;

  /* reads in 4 bytes from the file */

  bytes_read = fread(chunk_id, 4, 1, in_file);

  /* confirms that chunk_id is "MThd" */

  assert(strncmp(chunk_id, "MThd", 4) == 0);

  /* holds the bytes read in from the file to be converted */

  uint8_t integers[2];

  /* holds the bytes read in from the file to be converted */

  uint8_t numbers[4];

  /* following freads read in a total of 4 bytes from the file */

  bytes_read = fread(&numbers[0], 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  bytes_read = fread(&numbers[1], 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  bytes_read = fread(&numbers[2], 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  bytes_read = fread(&numbers[3], 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  /* holds the size of the header chunk */

  uint32_t chunk_size = end_swap_32(numbers);

  /* confirms that chunk_size is 6 */

  assert(chunk_size == 6);

  /* holds the format of the song */

  uint16_t chunk_format = 0;

  /* following freads read in a total of 2 bytes from the file */

  bytes_read = fread(&integers[0], 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  bytes_read = fread(&integers[1], 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  /* converts the integers array into a uint16_t format */

  chunk_format = end_swap_16(integers);

  /* confirms that chunk_format is less than or equal to 2 */

  assert(chunk_format <= 2);

  /* makes song->format the same as chunk_format */

  song->format = chunk_format;

  /* confirms that song->format is 2 bytes */

  assert(sizeof(song->format) <= 2);

  /* following freads read in a total of 2 bytes from the file */

  bytes_read = fread(&integers[0], 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  bytes_read = fread(&integers[1], 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  /* holds the number of tracks in the song */

  uint16_t chunk_tracks = end_swap_16(integers);

  /* makes song->num_tracks the same as chunk_tracks */

  song->num_tracks = chunk_tracks;

  /* holds the division of time in the song */

  division_t division = {0};

  /* following freads read in a total of 2 bytes from the file */

  bytes_read = fread(&integers[0], 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  bytes_read = fread(&integers[1], 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  /* time divison bytes in the song */

  int time_division = end_swap_16(integers);

  /* holds the type of time division in the song */

  uint16_t time_division_type = time_division & TIME_DIVISION_UPPER_MASK;

  /* holds the time division of the song */

  uint16_t result = 0;

  /* changes bytes to match use of bitwise operator */

  result = time_division & TIME_DIVISION_LOWER_MASK;

  /* checks if time_division_type is 0 */

  if (time_division_type == 0) {

    /* sets division.uses_tpq to true */

    division.uses_tpq = true;

    /* makes division.ticks_per_qtr the same as result */

    division.ticks_per_qtr = result;
  }
  else{

    /* sets division.uses_tpq to false */

    division.uses_tpq = false;

    /* makes division.frames_per_sec the same as result>>8 */

    division.frames_per_sec = result >> 8;

    /* makes division.ticks_per_frame the same as result & WORD_MASK_8; */

    division.ticks_per_frame = result & WORD_MASK_8;
  }

  /* makes song->division the same as division */

  song->division = division;

} /* parse_header() */

/*
 * This function reads a MIDI file track chunk from the file and updates the
 * song_data_t struct with the information
 */

void parse_track(FILE *in_file, song_data_t *song) {

  /* holds the chunk id of the track */

  char chunk_id[5] = "";

  /* counts number of bytes read from file */

  int bytes_read = 0;

  /* reads in 4 bytes from the file */

  bytes_read = fread(chunk_id, 4, 1, in_file);

  /* confirms that chunk_id is "MTrk" */

  assert(strncmp(chunk_id, "MTrk", 4) == 0);

  /* holds the bytes read in from the file to be converted */

  uint8_t numbers[4];

  /* the following freads read in a total of 4 bytes from the file */

  bytes_read = fread(&numbers[0], 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  bytes_read = fread(&numbers[1], 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  bytes_read = fread(&numbers[2], 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  bytes_read = fread(&numbers[3], 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  /* holds the chunk size of the track */

  uint32_t chunk_size = end_swap_32(numbers);

  /* holds the list of track nodes in the song track */

  track_node_t *track_node = NULL;

  /* allocates space in memoery for track_t struct */

  track_t *new_track = (track_t *)malloc(sizeof(track_t));

  /* makes new_track->length the same as chunk_size */

  new_track->length = chunk_size;

  /* holds the list of event nodes in the song track */

  event_node_t *event_listing = NULL;

  /* holds the new node to be added to the event list */

  event_node_t *new_node = NULL;

  /* holds the tail of the event list */

  event_node_t *last_node = NULL;

  /* changes to true when the track is finished being read */

  bool finish = false;

  /* holds the current event_t struct in the list */

  event_t *current_event = NULL;

  /* loops until the track is finished being read */

  while (finish == false) {

    /* parses the current event in the file */

    current_event = parse_event(in_file);

    /* checks if event_listing is null */

    if (event_listing == NULL) {

      /* allocates space in memory for event_node_t struct */

      new_node = (event_node_t *)malloc(sizeof(event_node_t));

      /* makes event_listing the same as new_node */

      event_listing = new_node;

      /* sets event_list->next_event to null */

      event_listing->next_event = NULL;

      /* makes event_listing->event the same as current_event */

      event_listing->event = current_event;

      /* makes last_node the same as event_listing */

      last_node = event_listing;
    }
    else {

      /* allocates space in memory for event_node_t struct */

      new_node = (event_node_t *)malloc(sizeof(event_node_t));

      /* sets new_node->next_event to null */

      new_node->next_event = NULL;

      /* makes last_node->next_event the same as new_node */

      last_node->next_event = new_node;

      /* makes last_node the same as new_node */

      last_node = new_node;

      /* makes new_node->event the same as current_event */

      new_node->event = current_event;
    }

    /* checks if the event is a meta event and if the name is "End of Track" */

    if ((event_type(new_node->event) == META_EVENT_T) &&
        (strcmp(new_node->event->meta_event.name, "End of Track") == 0)) {
      finish = true;
    }
  }

  /* allocates space in memory for track_node_t struct */

  track_node = (track_node_t *)malloc(sizeof(track_node_t));

  /* sets track_node->next_track to null */

  track_node->next_track = NULL;

  /* makes track_node->track the same as new_track */

  track_node->track = new_track;

  /* makes new_track->event_list the same as event_listing */

  new_track->event_list = event_listing;

  /* holds the tail of the track_node_t list */

  track_node_t *last_track_node = NULL;

  /* checks if song->track_list is null */

  if (song->track_list == NULL) {

    /* makes song->track_list the same as track_node */

    song->track_list = track_node;
  }
  else {

    /* makes last_track_node the same as song->track_list */

    last_track_node = song->track_list;

    /* loops until last_track_node reaches the end of the list */

    while (last_track_node->next_track != NULL) {

      /* moves to the next track in the list */

      last_track_node = last_track_node->next_track;
    }

    /* makes last_track_node->next_track the same as track_node */

    last_track_node->next_track = track_node;
  }

} /* parse_track() */

/*
 * This function reads in information from a file and creates an event_t struct
 * with the parsed information
 */

event_t *parse_event(FILE *in_file) {

  /* allocates space in memory for event_t struct */

  event_t *events = (event_t *)malloc(sizeof(event_t));

  /* counts number of bytes read from file */

  int bytes_read = 0;

  /* holds the delta time of the event */

  uint32_t delta_time = parse_var_len(in_file);

  /* makes events->delta_time the same as delta_time */

  events->delta_time = delta_time;

  /* holds the type of the event */

  uint8_t event_type = 0;

  /* reads in 1 byte from the file */

  bytes_read = fread(&event_type, 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  /* makes events->type the same as event_type */

  events->type = event_type;

  /* checks if event is a meta event */

  if (event_type == META_EVENT) {

    /* parses the meta event in the file */

    events->meta_event = parse_meta_event(in_file);
  }

  /* checks if event is a midi event */

  else if ((event_type >= MIDI_EVENT_LOWER_BOUND) &&
           (event_type <= MIDI_EVENT_UPPER_BOUND)) {

    /* parses the midi event in the file */

    events->midi_event = parse_midi_event(in_file, events->type);
  }

  /* checks if event_type is less than MIDI_EVENT_LOWER_BOUND */

  else if (event_type < MIDI_EVENT_LOWER_BOUND) {

    /* parses the midi event in the file */

    events->midi_event = parse_midi_event(in_file, events->type);
  }

  /* checks if event is a sys event */

  else if ((event_type == SYS_EVENT_1) || (event_type == SYS_EVENT_1)) {

    /* parses the sys event in the file */

    events->sys_event = parse_sys_event(in_file, events->type);
  }
  return events;

} /* parse_event() */

/*
 * This function reads in information from a file and creates a sys_event_t
 * struct with the information
 */

sys_event_t parse_sys_event(FILE *in_file, uint8_t type) {

  /* holds the byte values read from the file */

  uint8_t value = 0;

  /* counts number of bytes read from file */

  int bytes_read = 0;

  /* holds the array of byte values read from the file */

  uint8_t *data = NULL;

  /* holds the length of the sys event */

  uint32_t length = parse_var_len(in_file);

  /* holds the information of the sys_event_t struct */

  sys_event_t sys_event = {0};

  /* makes sys_event.data_len the same as length */

  sys_event.data_len = length;

  /* checks if length is greater than 0 */

  if (length > 0) {

    /* allocates space in memory for uint8_t struct */

    data = (uint8_t *)malloc(length * sizeof(uint8_t));

    /* loops until i is the same as length */

    for (int i = 0; i < length; i++) {

      /* reads in 1 bytes from the file */

      bytes_read = fread(&value, 1, 1, in_file);

      /* confirms that 1 byte was read */

      assert(bytes_read == 1);

      /* makes data[i] the same as value */

      data[i] = value;
    }
  }

  /* makes sys_event.data the same as data */

  sys_event.data = data;
  return sys_event;

} /* parse_sys_event() */


/*
 * This function reads in information from a file and creates a meta_event_t
 * struct with the information
 */

meta_event_t parse_meta_event(FILE *in_file) {

  /* holds the type of meta event */

  uint8_t type = 0;

  /* counts number of bytes read from file */

  int bytes_read = 0;

  /* holds the byte values read from the file */

  uint8_t value = 0;

  /* reads in 1 byte from the file */

  bytes_read = fread(&type, 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  /* holds the array of byte values read from the file */

  uint8_t *data = NULL;

  /* holds the information of the meta_event_t struct */

  meta_event_t meta_event = META_TABLE[type];

  /* confirms that meta_event.name is not null */

  assert(meta_event.name != NULL);

  /* holds the length of the meta event */

  uint32_t length = parse_var_len(in_file);

  /* checks if meta_event.data_len is greater than 0 */

  if (meta_event.data_len > 0) {

    /* confirms that meta_event.data_len is equal to length */

    assert(meta_event.data_len == length);
  }
  else {

    /* makes meta_event.data_len the same as length */

    meta_event.data_len = length;
  }

  /* checks if length is greater than 0 */

  if (length > 0) {

    /* allocates space in memory for uint8_t struct */

    data = (uint8_t *)malloc(length * sizeof(uint8_t));

    /* loops until i is the same as length */

    for (int i = 0; i < length; i++) {

      /* reads in 1 bytes from the file */

      bytes_read = fread(&value, 1, 1, in_file);

      /* confirms that 1 byte was read */

      assert(bytes_read == 1);

      /* makes data[i] the same as value */

      data[i] = value;
    }
  }

  /* makes meta_event.data the same as data */

  meta_event.data = data;
  return meta_event;

} /* parse_meta_event() */

/*
 * This function reads in information from a file and creates a midi_event_t
 * struct with the information
 */

midi_event_t parse_midi_event(FILE *in_file, uint8_t type) {

  /* holds the byte values read from the file */

  uint8_t value = 0;

  /* counts number of bytes read from file */

  int bytes_read = 0;

  /* holds the place the index of the for loop will start reading from */

  int start_index = 0;

  /* checks if type is less than MIDI_EVENT_LOWER_BOUND */

  if (type < MIDI_EVENT_LOWER_BOUND) {

    /* makes value the same as type */

    value = type;

    /* makes type the same as g_last_status */

    type = g_last_status;

    /* makes start_index 1 */

    start_index = 1;
  }

  /* holds the information of the midi_event_t struct */

  midi_event_t midi_event = MIDI_TABLE[type];

  /* confirms that midi_event.name is not null */

  assert(midi_event.name != NULL);

  /* holds the array of byte values read from the file */

  uint8_t *data = malloc(midi_event.data_len);

  /* checks if start_index is 1 */

  if (start_index == 1) {

    /* makes data[0] the same as value */

    data[0] = value;

    /* makes midi_event.status the same as g_last_status */

    midi_event.status = g_last_status;
  }

  /* loops until i is the same as length */

  for (int i = start_index; i < midi_event.data_len; i++) {

    /* reads in 1 bytes from the file */

    bytes_read = fread(&value, 1, 1, in_file);

    /* confirms that 1 byte was read */

    assert(bytes_read == 1);

    /* makes data[i] the same as value */

    data[i] = value;
  }

  /* makes midi_event.data the same as data */

  midi_event.data = data;

  /* makes g_last_status the same as type */

  g_last_status = type;
  return midi_event;

} /* parse_midi_event() */

/*
 * This function reads an integer with a variable length from the file and
 * returns it
 */

uint32_t parse_var_len(FILE *in_file) {

  /* holds the byte values read from the file */

  uint8_t byte_value = 0;

  /* counts number of bytes read from file */

  int bytes_read = 0;

  /* holds the combined total of the byte values read from the file */

  uint32_t value = 0;

  /* reads in 1 byte from the file */

  bytes_read = fread(&byte_value, 1, 1, in_file);

  /* confirms that 1 byte was read */

  assert(bytes_read == 1);

  /* checks if byte_value is greater than or equal to */
  /* VAR_LENGTH_NEED_TO_READ_ANOTHER_BYTE */

  if (byte_value >= VAR_LENGTH_NEED_TO_READ_ANOTHER_BYTE) {

    /* changes bytes to match use of bitwise operator */

    value = byte_value & VAR_LENGTH_MASK;

    /* reads in 1 byte from the file */

    bytes_read = fread(&byte_value, 1, 1, in_file);

    /* confirms that 1 byte was read */

    assert(bytes_read == 1);

    /* checks if byte_value is greater than or equal to */
    /* VAR_LENGTH_NEED_TO_READ_ANOTHER_BYTE */

    if (byte_value >= VAR_LENGTH_NEED_TO_READ_ANOTHER_BYTE) {

      /* changes bytes to match use of bitwise operator */

      value = value << 7 | (byte_value & VAR_LENGTH_MASK);

      /* reads in 1 byte from the file */

      bytes_read = fread(&byte_value, 1, 1, in_file);

      /* confirms that 1 byte was read */

      assert(bytes_read == 1);

      /* checks if byte_value is greater than or equal to */
      /* VAR_LENGTH_NEED_TO_READ_ANOTHER_BYTE */

      if (byte_value >= VAR_LENGTH_NEED_TO_READ_ANOTHER_BYTE) {

        /* reads in 1 byte from the file */

        bytes_read = fread(&byte_value, 1, 1, in_file);

        /* confirms that 1 byte was read */

        assert(bytes_read == 1);

        /* changes bytes to match use of bitwise operator */

        value = value << 7 | (byte_value & VAR_LENGTH_MASK);
      }
      else {

        /* changes bytes to match use of bitwise operator */

        value = value << 7 | byte_value;
      }
    }
    else {

      /* changes bytes to match use of bitwise operator */

      value = value << 7 | byte_value;
    }
  }
  else {

    /* makes value the same as byte_value */

    value = byte_value;
  }
  return value;

} /* parse_var_len() */

/*
 * This function returns a value based on the event_t parameter
 */

uint8_t event_type(event_t *current_event) {

  /* holds what kind event the current event is */

  uint8_t event_type = 0;

  /* checks if the current event is a meta event */

  if (current_event->type == META_EVENT) {

    /* makes event_type the same as META_EVENT_T */

    event_type = META_EVENT_T;
  }

  /* checks if the current event is a sys event */

  else if ((current_event->type == SYS_EVENT_1) ||
           (current_event->type == SYS_EVENT_2)) {

    /* makes event_type the same as SYS_EVENT_T */

    event_type = SYS_EVENT_T;
  }
  else {

    /* makes event_type the same as MIDI_EVENT_T */

    event_type = MIDI_EVENT_T;
  }
  return event_type;

} /* event_type() */

/*
 * This function frees all memory in the song_data_t struct given as a
 * parameter
 */

void free_song(song_data_t *song) {

  /* checks if song is null */

  if (song != NULL) {

    /* holds the list of tracks in the song */

    track_node_t *track_list = song->track_list;

    /* holds the next tract_node_t struct in the list */

    track_node_t *next_track_list = NULL;

    /* loops until track_list reaches the end of the list */

    while (track_list != NULL) {

      /* makes next_track_list the same as track_list->next_track */

      next_track_list = track_list->next_track;

      /* frees all memory in track_list */

      free_track_node(track_list);

      /* moves to the next track in the list */

      track_list = next_track_list;
    }

    /* frees memory in song->path */

    free(song->path);

    /* frees memory in song */

    free(song);
  }

} /* free_song() */

/*
 * This function frees all memory in the track_node_t struct given as a
 * parameter
 */

void free_track_node(track_node_t *node) {

  /* holds the list of events in the track_node_t struct */

  event_node_t *event_list = node->track->event_list;

  /* holds the next event_node_t struct in the list */

  event_node_t *next_event_list = NULL;

  /* loops until event_list reaches the end of the list */

  while (event_list != NULL) {

    /* makes next_event_list the same as event_list->next_event */

    next_event_list = event_list->next_event;

    /* frees all memory in event_list */

    free_event_node(event_list);

    /* moves to the next event in the list */

    event_list = next_event_list;
  }

  /* frees memory in node->track */

  free(node->track);

  /* frees memory in node */

  free(node);

} /* free_track_node() */

/*
 * This function frees all memory in the event_node_t struct given as a
 * parameter
 */

void free_event_node(event_node_t *node) {

  /* checks if node is a meta event */

  if (event_type(node->event) == META_EVENT_T) {

    /* frees memory in node->event->meta_event.data */

    free(node->event->meta_event.data);
  }

  /* checks if node is a midi event */

  else if (event_type(node->event) == MIDI_EVENT_T) {

    /* frees memory in node->event->midi_event.data */

    free(node->event->midi_event.data);
  }
  else {

    /* frees memory in node->event->sys_event.data */

    free(node->event->sys_event.data);
  }

  /* frees memory in node->event */

  free(node->event);

  /* frees memory in node */

  free(node);

} /* free_event_node() */

/*
 * This function takes in a buffer of 2 uint8_t and returns a uint16 with
 * opposite endianness
 */

uint16_t end_swap_16(uint8_t integers[2]) {

  /* holds the result of the bytes changed after using bitwise operators */

  uint16_t result = (integers[0] << 8) | (integers[1]);
  return result;

} /* end_swap_16() */

/*
 * This function takes in a buffer of 4 uint8_t and returns a uint32 with
 * opposite endianness
 */

uint32_t end_swap_32(uint8_t integers[4]) {

  /* holds the result of the bytes changed after using bitwise operators */

  uint32_t result = (integers[0] << 24) | (integers[1] << 16) |
                    (integers[2] << 8) | (integers[3]);
  return result;

} /* end_swap_32() */
