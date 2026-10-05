#ifndef FILEINFO_H
#define FILEINFO_H

#include <stddef.h>
#include <sys/stat.h>

#include "options.h"

typedef struct {
    char *path;
    char *name;
    struct stat st;
    int is_symlink;
} FileInfo;

typedef struct {
    FileInfo *items;
    size_t size;
    size_t capacity;
} FileList;

void file_list_init(FileList *list);
void file_list_free(FileList *list);
int file_list_push(FileList *list, const char *path, const char *name, int follow_symlink);
const struct timespec *file_info_time(const FileInfo *info, TimeMode mode);

#endif
