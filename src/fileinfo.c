#include "fileinfo.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *duplicate_string(const char *text)
{
    size_t len;
    char *copy;

    if (text == NULL) {
        return NULL;
    }

    len = strlen(text) + 1;
    copy = malloc(len);
    if (copy != NULL) {
        memcpy(copy, text, len);
    }
    return copy;
}

void file_list_init(FileList *list)
{
    list->items = NULL;
    list->size = 0;
    list->capacity = 0;
}

void file_list_free(FileList *list)
{
    size_t i;

    if (list == NULL) {
        return;
    }

    for (i = 0; i < list->size; ++i) {
        free(list->items[i].path);
        free(list->items[i].name);
    }
    free(list->items);
    file_list_init(list);
}

int file_list_push(FileList *list, const char *path, const char *name, int follow_symlink)
{
    struct stat lst;
    struct stat st;
    FileInfo *grown;
    size_t new_capacity;
    char *path_copy;
    char *name_copy;

    if (lstat(path, &lst) == -1) {
        fprintf(stderr, "myls: %s: %s\n", path, strerror(errno));
        return -1;
    }

    st = lst;
    if (follow_symlink && S_ISLNK(lst.st_mode)) {
        if (stat(path, &st) == -1) {
            /* If the target cannot be followed, keep symlink information. */
            st = lst;
        }
    }

    if (list->size == list->capacity) {
        new_capacity = list->capacity == 0 ? 16 : list->capacity * 2;
        grown = realloc(list->items, new_capacity * sizeof(*grown));
        if (grown == NULL) {
            fprintf(stderr, "myls: out of memory\n");
            return -1;
        }
        list->items = grown;
        list->capacity = new_capacity;
    }

    path_copy = duplicate_string(path);
    name_copy = duplicate_string(name);
    if (path_copy == NULL || name_copy == NULL) {
        free(path_copy);
        free(name_copy);
        fprintf(stderr, "myls: out of memory\n");
        return -1;
    }

    list->items[list->size].path = path_copy;
    list->items[list->size].name = name_copy;
    list->items[list->size].st = st;
    list->items[list->size].is_symlink = S_ISLNK(lst.st_mode);
    ++list->size;
    return 0;
}

const struct timespec *file_info_time(const FileInfo *info, TimeMode mode)
{
    switch (mode) {
    case TIME_STATUS_CHANGE:
        return &info->st.st_ctim;
    case TIME_ACCESS:
        return &info->st.st_atim;
    case TIME_MODIFICATION:
    default:
        return &info->st.st_mtim;
    }
}
