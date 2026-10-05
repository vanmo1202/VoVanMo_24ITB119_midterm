#ifndef OPTIONS_H
#define OPTIONS_H

typedef enum {
    TIME_MODIFICATION,
    TIME_STATUS_CHANGE,
    TIME_ACCESS
} TimeMode;

typedef enum {
    SORT_NAME,
    SORT_SIZE,
    SORT_TIME,
    SORT_NONE
} SortMode;

typedef enum {
    PRINT_RAW,
    PRINT_QUESTION
} NamePrintMode;

typedef struct {
    int show_all;          /* -a */
    int almost_all;        /* -A */
    int directory_as_file; /* -d */
    int classify;          /* -F */
    int human_readable;    /* -h */
    int show_inode;        /* -i */
    int kilobytes;         /* -k */
    int long_format;       /* -l or -n */
    int numeric_ids;       /* -n */
    int recursive;         /* -R */
    int reverse;           /* -r */
    int show_blocks;       /* -s */
    TimeMode time_mode;    /* default / -c / -u */
    SortMode sort_mode;    /* default / -S / -t / -f */
    NamePrintMode name_mode; /* -q / -w and terminal default */
} LsOptions;

void options_init(LsOptions *options);
int parse_options(int argc, char *argv[], LsOptions *options, int *first_operand);

#endif
