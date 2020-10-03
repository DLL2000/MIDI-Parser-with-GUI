/* Name, library.c, CS 24000, Spring 2020
 * Last updated March 27, 2020
 */

/* Add any includes here */

#define _GNU_SOURCE
#include "library.h"
#include <malloc.h>
#include <assert.h>
#include <ftw.h>
#include <string.h>


tree_node_t *g_song_library = NULL;

/*
 * This function finds the parent of the child node whose song matches the
 * one given as a parameter
 */

tree_node_t *find_grand_parent_pointer(tree_node_t *parent_node,
                                       const char *song_name) {

  /* checks if parent_node is null */

  if (parent_node == NULL) {
    return NULL;
  }

  /* checks if parent_node->left_child is not null */

  if (parent_node->left_child != NULL) {

    /* checks if parent_node->left_child->song_name and song_name are */
    /* the same */

    if (strcmp(parent_node->left_child->song_name, song_name) == 0) {
      return parent_node;
    }
  }

  /* checks if parent_node->right_child is not null */

  if (parent_node->right_child != NULL) {

    /* checks if parent_node->right_child->song_name and song_name are */
    /* the same */

    if (strcmp(parent_node->right_child->song_name, song_name) == 0) {
      return parent_node;
    }
  }

  /* holds the result of using strcmp on the song names */

  int strcmp_result = strcmp(song_name, parent_node->song_name);

  /* checks if strcmp_result is 0 */

  if (strcmp_result == 0) {
    return parent_node;
  }

  /* checks if strcmp_result less than 0 */

  if (strcmp_result < 0) {
    return find_grand_parent_pointer(parent_node->left_child, song_name);
  }
  else {
    return find_grand_parent_pointer(parent_node->right_child, song_name);
  }
  return NULL;

} /* find_grand_parent_pointer() */

/*
 * This function finds the node whose song name matches the one given as a
 * parameter and returns its parent's child pointer
 */

tree_node_t **find_parent_pointer(tree_node_t **parent_node,
                                  const char *song_name) {

  /* checks if *parent_node is null */

  if (*parent_node == NULL) {
    return NULL;
  }

  /* holds the result of using strcmp on the song names */

  int strcmp_result = strcmp(song_name, (*parent_node)->song_name);

  /* checks if strcmp_result is 0 */

  if (strcmp_result == 0) {
    return parent_node;
  }

  /* checks if strcmp_result is less than 0 */

  if (strcmp_result < 0) {
    return find_parent_pointer(&((*parent_node)->left_child), song_name);
  }
  else {
    return find_parent_pointer(&((*parent_node)->right_child), song_name);
  }
  return NULL;

} /* find_parent_pointer() */

/*
 * This function inserts a node into the tree
 */

int tree_insert(tree_node_t **parent_node, tree_node_t *new_node) {

  /* checks if *parent_node is null */

  if (*parent_node == NULL) {

    /* makes *parent_node the same as new_node */

    *parent_node = new_node;
    return INSERT_SUCCESS;
  }

  /* holds the result of using strcmp on the song names */

  int strcmp_result = strcmp(new_node->song_name, (*parent_node)->song_name);

  /* checks if strcmp_result is 0 */

  if (strcmp_result == 0) {
    return DUPLICATE_SONG;
  }

  /* checks if strcmp_result is less than 0 */

  else if (strcmp_result < 0) {

    /* checks if (*parent_node)->left_child is null */

    if ((*parent_node)->left_child == NULL) {

      /* makes (*parent_node)->left_child the same as new_node */

      (*parent_node)->left_child = new_node;
      return INSERT_SUCCESS;
    }
    else {
      return tree_insert(&((*parent_node)->left_child), new_node);
    }
  }
  else {

    /* checks if (*parent_node)->right_child is null */

    if ((*parent_node)->right_child == NULL) {

      /* makes (*parent_node)->right_child the same as new_node */

      (*parent_node)->right_child = new_node;
      return INSERT_SUCCESS;
    }
    else {
      return tree_insert(&((*parent_node)->right_child), new_node);
    }
  }
  return INSERT_SUCCESS;

} /* tree_insert() */

/*
 * This function removes a song in the tree whose name matches the one given as
 * a parameter and reinserts any child nodes they have back into the tree
 */

int remove_song_from_tree(tree_node_t **root_node, const char *song_name) {

  /* holds the song to be removed from the list */

  tree_node_t **song_node = NULL;

  /* holds the parent of the song to be removed from the list */

  tree_node_t *parent_of_song_node = NULL;

  /* holds the left_child of the song to be removed from the list */

  tree_node_t *left_node = NULL;

  /* holds the right_child of the song to be removed from the list */

  tree_node_t *right_node = NULL;

  /* a temporary variable to hold tree_node_t values */

  tree_node_t *temp_song_node = NULL;

  /* a temporary variable to hold tree_node_t values */

  tree_node_t *temp_root_node = NULL;

  /* finds the song to be removed from the tree */

  song_node = find_parent_pointer(root_node, song_name);
  if (song_node == NULL) {
    return SONG_NOT_FOUND;
  }

  /* checks if *song_node is the head */

  if (*song_node == *root_node) {

    /* makes temp_root_node the same as *(root_node) */

    temp_root_node = *(root_node);

    /* makes left_node the same as temp_root_node->left_child */

    left_node = temp_root_node->left_child;

    /* makes right_node the same as temp_root_node->right_child */

    right_node = temp_root_node->right_child;

    /* frees memory in temp_root_node */

    free_node(temp_root_node);

    /* sets *root_node to null */

    *root_node = NULL;

    /* checks if left_node is null */

    if (left_node != NULL) {

      /* reinserts the left_node into the tree */

      tree_insert(root_node, left_node);

      /* reinserts the right_node into the tree */

      tree_insert(root_node, right_node);
    }
    else {

      /* reinserts the right_node into the tree */

      tree_insert(root_node, right_node);
    }
    return DELETE_SUCCESS;
  }

  /* finds the parents of the song to be removed */

  parent_of_song_node = find_grand_parent_pointer(*root_node, song_name);

  /* checks if parent_of_song_node->left_child is not null and matches the */
  /* song to be removed */

  if ((parent_of_song_node->left_child != NULL) &&
      (parent_of_song_node->left_child == *song_node)) {

    /* makes temp_song_node the same as parent_of_song_node->left_child */

    temp_song_node = parent_of_song_node->left_child;

    /* sets parent_of_song_node->left_child to null */

    parent_of_song_node->left_child = NULL;
  }
  else {

    /* makes temp_song_node the same as parent_of_song_node->right_child */

    temp_song_node = parent_of_song_node->right_child;

    /* sets parent_of_song_node->right_child to null */

    parent_of_song_node->right_child = NULL;
  }

  /* makes left_node the same as temp_song_node->left_child */

  left_node = temp_song_node->left_child;

  /* makes right_node the same as temp_song_node->right_child */

  right_node = temp_song_node->right_child;

  /* frees memory in temp_song_node */

  free_node(temp_song_node);

  /* checks if left_node is not null */

  if (left_node != NULL) {

    /* reinserts left_node into the tree */

    tree_insert(root_node, left_node);
  }

  /* checks if right_node is not null */

  if (right_node != NULL) {

    /* reinserts right_node into the tree */

    tree_insert(root_node, right_node);
  }
  return DELETE_SUCCESS;

} /* remove_song_from_tree() */

/*
 * This function frees all memory in the given node
 */

void free_node(tree_node_t *node) {

  /* sets node->song_name to null */

  node->song_name = NULL;

  /* frees memory in node->song */

  free_song(node->song);

  /* sets node->left_child to null */

  node->left_child = NULL;

  /* sets node->right_child to null */

  node->right_child = NULL;

  /* frees memory in node */

  free(node);

  /* sets node to null */

  node = NULL;

} /* free_node() */

/*
 * This function prints the song name of the given node into a file
 */

void print_node(tree_node_t *node, FILE *write_file) {

  /* checks if node is null */

  if (node == NULL) {

    /* prints "null" to the file */

    fprintf(write_file, "null\n");
  }
  else {

    /* prints the song name to the file */

    fprintf(write_file, "%s\n", node->song_name);
  }

} /* print_node() */

/*
 * This function traverses through the tree using preorder traversal
 */

void traverse_pre_order(tree_node_t *node, void *data,
                        traversal_func_t my_function) {

  /* checks if node is not null */

  if (node != NULL) {

    /* sets current information of node */

    my_function(node, data);

    /* recursion used to traverse through left_child */

    traverse_pre_order(node->left_child, data, my_function);

    /* recursion used to traverse through right_child */

    traverse_pre_order(node->right_child, data, my_function);
  }

} /* traverse_pre_order() */

/*
 * This function traverses through the tree using inorder traversal
 */

void traverse_in_order(tree_node_t *node, void *data,
                       traversal_func_t my_function) {

  /* checks if node is not null */

  if (node != NULL) {

    /* recursion is used to traverse through node->left_child */

    traverse_in_order(node->left_child, data, my_function);

    /* sets current information of node */

    my_function(node, data);

    /* recursion is used to traverse through node->right_child */

    traverse_in_order(node->right_child, data, my_function);
  }

} /* traverse_in_order() */

/*
 * This function traverses through the tree using postorder traversal
 */

void traverse_post_order(tree_node_t *node, void *data,
                         traversal_func_t my_function) {

  /* checks if node is null */

  if (node != NULL) {

    /* recursion is used to traverse through node->left_child */

    traverse_post_order(node->left_child, data, my_function);

    /* recursion is used to traverse through node->right_child */

    traverse_post_order(node->right_child, data, my_function);

    /* sets current information of node */

    my_function(node, data);
  }

} /* traverse_post_order() */

/*
 * This function frees all nodes in the library
 */

void free_library(tree_node_t *library) {

  /* checks if library is not null */

  if (library != NULL) {

    /* checks if library->left_child is not null */

    if (library->left_child != NULL) {

      /* recursion is used to traverse through library->left_child */

      free_library(library->left_child);
    }

    /* checks if library->right_child is not null */

    if (library->right_child != NULL) {

      /* recursion is used to traverse through library->right_child */

      free_library(library->right_child);
    }

    /* frees memory in library */

    free_node(library);
  }

} /* free_library() */

/*
 * This function prints the names of the songs into a file in sorted order
 */

void write_song_list(FILE *in_file, tree_node_t *song_list) {

  /* traverses list using inorder traversal and writes songs to a file */

  traverse_in_order(song_list, in_file, (traversal_func_t) print_node);

} /* write_song_list() */

/*
 * This function takes a song path and inserts the song name into
 * g_song_library
 */

int tree_insert_file(const char *path, const struct stat *sptr, int type) {

  /* holds the result of whether a song was successfully inserted into */
  /* g_song_library */

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

      result_code = tree_insert(&g_song_library, new_node);
    }
  }
  return result_code;

} /* tree_insert_file() */

/*
 * This function adds all MIDI files in the directory to g_song_library
 */

void make_library(const char *directory) {

  /* sets g_song_library to null */

  g_song_library = NULL;

  /* inserts songs from directory into g_song_library */

  int result = ftw(directory, tree_insert_file, 7);

  /* confirms that result is not the same as DUPLICATE_SONG */

  assert(result != DUPLICATE_SONG);

} /* make_library() */
