#include "listing.h"

#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fileinfo.h"
#include "format.h"
#include "sort.h"

static int should_include(const char *name, const LsOptions *options)
{
    if (options->show_all) {
        return 1;
    }
    if (options->almost_all) {
        return strcmp(name, ".") != 0 && strcmp(name, "..") != 0;
    }
    return name[0] != '.';
}

static char *join_path(const char *directory, const char *name)
{
    size_t dir_len = strlen(directory);
    size_t name_len = strlen(name);
    int need_slash = dir_len > 0 && directory[dir_len - 1] != '/';
    char *result = malloc(dir_len + (size_t)need_slash + name_len + 1);

    if (result == NULL) {
        return NULL;
    }

    memcpy(result, directory, dir_len);
    if (need_slash) {
        result[dir_len++] = '/';
    }
    memcpy(result + dir_len, name, name_len + 1);
    return result;
}

static int read_directory(const char *path, const LsOptions *options, FileList *entries)
{
    DIR *dir;
    struct dirent *entry;
    int status = 0;

    dir = opendir(path);
    if (dir == NULL) {
        fprintf(stderr, "myls: %s: %s\n", path, strerror(errno));
        return 1;
    }

    for (;;) {
        char *full_path;

        errno = 0;
        entry = readdir(dir);
        if (entry == NULL) {
            if (errno != 0) {
                fprintf(stderr, "myls: %s: %s\n", path, strerror(errno));
                status = 1;
            }
            break;
        }

        if (!should_include(entry->d_name, options)) {
            continue;
        }

        full_path = join_path(path, entry->d_name);
        if (full_path == NULL) {
            fprintf(stderr, "myls: out of memory\n");
            status = 1;
            break;
        }

        if (file_list_push(entries, full_path, entry->d_name, 0) != 0) {
            status = 1;
        }
        free(full_path);
    }

    if (closedir(dir) == -1) {
        fprintf(stderr, "myls: %s: %s\n", path, strerror(errno));
        status = 1;
    }

    return status;
}

static uint64_t sum_blocks(const FileList *entries)
{
    size_t i;
    uint64_t total = 0;

    for (i = 0; i < entries->size; ++i) {
        if (entries->items[i].st.st_blocks > 0) {
            total += (uint64_t)entries->items[i].st.st_blocks;
        }
    }
    return total;
}

static int list_recursive_child(const char *path, const LsOptions *options,
                                int *printed_any_section)
{
    FileList entries;
    size_t i;
    int status;

    file_list_init(&entries);
    status = read_directory(path, options, &entries);
    sort_file_list(&entries, options);

    if (*printed_any_section) {
        putchar('\n');
    }
    printf("%s:\n", path);
    *printed_any_section = 1;

    if (options->long_format || (options->show_blocks && isatty(STDOUT_FILENO))) {
        print_total_blocks(sum_blocks(&entries), options);
    }

    for (i = 0; i < entries.size; ++i) {
        print_file_info(&entries.items[i], options);
    }

    for (i = 0; i < entries.size; ++i) {
        const FileInfo *item = &entries.items[i];

        if (S_ISDIR(item->st.st_mode) && !item->is_symlink &&
            strcmp(item->name, ".") != 0 && strcmp(item->name, "..") != 0) {
            if (list_recursive_child(item->path, options, printed_any_section) != 0) {
                status = 1;
            }
        }
    }

    file_list_free(&entries);
    return status;
}

static int add_operand(FileList *files, FileList *directories, const char *path,
                       const LsOptions *options)
{
    FileList temporary;
    const FileInfo *item;
    int follow = !options->directory_as_file;
    int result = 0;

    file_list_init(&temporary);
    if (file_list_push(&temporary, path, path, follow) != 0) {
        file_list_free(&temporary);
        return 1;
    }

    item = &temporary.items[0];
    if (S_ISDIR(item->st.st_mode) && !options->directory_as_file) {
        if (file_list_push(directories, path, path, follow) != 0) {
            result = 1;
        }
    } else {
        if (file_list_push(files, path, path, follow) != 0) {
            result = 1;
        }
    }

    file_list_free(&temporary);
    return result;
}

static int list_one_directory(const char *path, const LsOptions *options,
                              int header, int *printed_any_section)
{
    FileList entries;
    size_t i;
    int status;

    file_list_init(&entries);
    status = read_directory(path, options, &entries);
    sort_file_list(&entries, options);

    if (header) {
        if (*printed_any_section) {
            putchar('\n');
        }
        printf("%s:\n", path);
        *printed_any_section = 1;
    }

    if (options->long_format || (options->show_blocks && isatty(STDOUT_FILENO))) {
        print_total_blocks(sum_blocks(&entries), options);
    }

    for (i = 0; i < entries.size; ++i) {
        print_file_info(&entries.items[i], options);
    }

    if (!header && entries.size > 0) {
        *printed_any_section = 1;
    }

    if (options->recursive) {
        for (i = 0; i < entries.size; ++i) {
            const FileInfo *item = &entries.items[i];

            if (S_ISDIR(item->st.st_mode) && !item->is_symlink &&
                strcmp(item->name, ".") != 0 && strcmp(item->name, "..") != 0) {
                if (list_recursive_child(item->path, options, printed_any_section) != 0) {
                    status = 1;
                }
            }
        }
    }

    file_list_free(&entries);
    return status;
}

int list_operands(int argc, char *argv[], int first_operand, const LsOptions *options)
{
    FileList files;
    FileList directories;
    size_t i;
    int status = 0;
    int printed_any_section = 0;

    file_list_init(&files);
    file_list_init(&directories);

    if (first_operand >= argc) {
        if (file_list_push(&directories, ".", ".", 1) != 0) {
            file_list_free(&files);
            file_list_free(&directories);
            return 1;
        }
    } else {
        for (i = (size_t)first_operand; i < (size_t)argc; ++i) {
            if (add_operand(&files, &directories, argv[i], options) != 0) {
                status = 1;
            }
        }
    }

    sort_file_list(&files, options);
    sort_file_list(&directories, options);

    for (i = 0; i < files.size; ++i) {
        print_file_info(&files.items[i], options);
    }
    if (files.size > 0) {
        printed_any_section = 1;
    }

    for (i = 0; i < directories.size; ++i) {
        int header = (files.size + directories.size > 1) || options->recursive;

        if (list_one_directory(directories.items[i].path, options, header,
                               &printed_any_section) != 0) {
            status = 1;
        }
    }

    file_list_free(&files);
    file_list_free(&directories);
    return status;
}
