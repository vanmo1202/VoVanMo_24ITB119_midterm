#include "sort.h"

#include <stdlib.h>
#include <string.h>

static const LsOptions *current_options;

static int compare_timespec_desc(const struct timespec *a, const struct timespec *b)
{
    if (a->tv_sec != b->tv_sec) {
        return a->tv_sec > b->tv_sec ? -1 : 1;
    }
    if (a->tv_nsec != b->tv_nsec) {
        return a->tv_nsec > b->tv_nsec ? -1 : 1;
    }
    return 0;
}

static int compare_file_info(const void *left, const void *right)
{
    const FileInfo *a = left;
    const FileInfo *b = right;
    int result = 0;

    switch (current_options->sort_mode) {
    case SORT_SIZE:
        if (a->st.st_size != b->st.st_size) {
            result = a->st.st_size > b->st.st_size ? -1 : 1;
        }
        break;
    case SORT_TIME:
        result = compare_timespec_desc(
            file_info_time(a, current_options->time_mode),
            file_info_time(b, current_options->time_mode));
        break;
    case SORT_NAME:
        break;
    case SORT_NONE:
        return 0;
    }

    if (result == 0) {
        result = strcmp(a->name, b->name);
    }

    if (current_options->reverse) {
        result = -result;
    }
    return result;
}

void sort_file_list(FileList *list, const LsOptions *options)
{
    if (list == NULL || options == NULL || list->size < 2 || options->sort_mode == SORT_NONE) {
        return;
    }

    current_options = options;
    qsort(list->items, list->size, sizeof(list->items[0]), compare_file_info);
}
