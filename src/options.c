#include "options.h"

#include <stdio.h>
#include <unistd.h>

void options_init(LsOptions *options)
{
    options->show_all = 0;
    options->almost_all = (geteuid() == 0); /* manual: always set for super-user */
    options->directory_as_file = 0;
    options->classify = 0;
    options->human_readable = 0;
    options->show_inode = 0;
    options->kilobytes = 0;
    options->long_format = 0;
    options->numeric_ids = 0;
    options->recursive = 0;
    options->reverse = 0;
    options->show_blocks = 0;
    options->time_mode = TIME_MODIFICATION;
    options->sort_mode = SORT_NAME;
    options->name_mode = isatty(STDOUT_FILENO) ? PRINT_QUESTION : PRINT_RAW;
}

int parse_options(int argc, char *argv[], LsOptions *options, int *first_operand)
{
    int ch;

    if (options == NULL || first_operand == NULL) {
        return -1;
    }

    options_init(options);
    opterr = 0;
    optind = 1;

    while ((ch = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (ch) {
        case 'A':
            options->almost_all = 1;
            break;
        case 'a':
            options->show_all = 1;
            break;
        case 'c':
            options->time_mode = TIME_STATUS_CHANGE;
            break;
        case 'd':
            options->directory_as_file = 1;
            options->recursive = 0; /* -d and -R: last one wins */
            break;
        case 'F':
            options->classify = 1;
            break;
        case 'f':
            options->sort_mode = SORT_NONE;
            break;
        case 'h':
            options->human_readable = 1;
            options->kilobytes = 0; /* -h and -k: last one wins */
            break;
        case 'i':
            options->show_inode = 1;
            break;
        case 'k':
            options->kilobytes = 1;
            options->human_readable = 0; /* -h and -k: last one wins */
            break;
        case 'l':
            options->long_format = 1;
            options->numeric_ids = 0; /* -l and -n: last one wins */
            break;
        case 'n':
            options->long_format = 1;
            options->numeric_ids = 1;
            break;
        case 'q':
            options->name_mode = PRINT_QUESTION;
            break;
        case 'R':
            options->recursive = 1;
            options->directory_as_file = 0; /* -R and -d: last one wins */
            break;
        case 'r':
            options->reverse = 1;
            break;
        case 'S':
            options->sort_mode = SORT_SIZE;
            break;
        case 's':
            options->show_blocks = 1;
            break;
        case 't':
            options->sort_mode = SORT_TIME;
            break;
        case 'u':
            options->time_mode = TIME_ACCESS;
            break;
        case 'w':
            options->name_mode = PRINT_RAW;
            break;
        case '?':
        default:
            if (optopt != 0) {
                fprintf(stderr, "myls: unknown option -- %c\n", optopt);
            } else {
                fprintf(stderr, "myls: invalid option\n");
            }
            fprintf(stderr, "usage: myls [-AacdFfhiklnqRrSstuw] [file ...]\n");
            return -1;
        }
    }

    *first_operand = optind;
    return 0;
}
