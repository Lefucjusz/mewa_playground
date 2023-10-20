/*
 * dir.h
 *
 *  Created on: Sep 28, 2023
 *      Author: lefucjusz
 */

#pragma once

#include "list.h"
#include "fatfs.h"

typedef struct list_t dir_list_t;
typedef struct list_node_t dir_entry_t;

void dir_init(const char *root_path);

/* Returns true if currently at top directory */
bool dir_is_top(void);

int dir_enter(const char *name);
int dir_return(void);

const char *dir_get_fs_path(void);

dir_list_t *dir_list(void);

/* Returns previous element, if there's no such, returns last (looped list) */
dir_entry_t *dir_get_prev(dir_list_t *list, dir_entry_t *current);

/* Returns next element, if there's no such, returns first (looped list) */
dir_entry_t *dir_get_next(dir_list_t *list, dir_entry_t *current);

/* Returns file descriptor or NULL if entry invalid */
FILINFO *dir_get_fd(dir_entry_t *entry);

/* Returns element with given name or NULL if not found */
dir_entry_t *dir_find_entry(dir_list_t *list, const char *name);

void dir_list_free(dir_list_t *list);
