#include "format.h"

#include <ctype.h>
#include <grp.h>
#include <inttypes.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

static char type_char(mode_t mode)
{
    if (S_ISREG(mode)) return '-';
    if (S_ISDIR(mode)) return 'd';
    if (S_ISLNK(mode)) return 'l';
    if (S_ISBLK(mode)) return 'b';
    if (S_ISCHR(mode)) return 'c';
    if (S_ISSOCK(mode)) return 's';
    if (S_ISFIFO(mode)) return 'p';
#ifdef S_ISWHT
    if (S_ISWHT(mode)) return 'w';
#endif
    return '?';
}

static void mode_string(mode_t mode, char output[11])
{
    output[0] = type_char(mode);
    output[1] = (mode & S_IRUSR) ? 'r' : '-';
    output[2] = (mode & S_IWUSR) ? 'w' : '-';
    output[3] = (mode & S_ISUID) ? ((mode & S_IXUSR) ? 's' : 'S')
                                  : ((mode & S_IXUSR) ? 'x' : '-');
    output[4] = (mode & S_IRGRP) ? 'r' : '-';
    output[5] = (mode & S_IWGRP) ? 'w' : '-';
    output[6] = (mode & S_ISGID) ? ((mode & S_IXGRP) ? 's' : 'S')
                                  : ((mode & S_IXGRP) ? 'x' : '-');
    output[7] = (mode & S_IROTH) ? 'r' : '-';
    output[8] = (mode & S_IWOTH) ? 'w' : '-';
    output[9] = (mode & S_ISVTX) ? ((mode & S_IXOTH) ? 't' : 'T')
                                  : ((mode & S_IXOTH) ? 'x' : '-');
    output[10] = '\0';
}

static void human_size(uint64_t bytes, char *buffer, size_t size)
{
    static const char units[] = "BKMGTPE";
    double value = (double)bytes;
    size_t unit = 0;

    while (value >= 1024.0 && unit + 1 < sizeof(units) - 1) {
        value /= 1024.0;
        ++unit;
    }

    if (unit == 0) {
        snprintf(buffer, size, "%lluB", (unsigned long long)bytes);
    } else if (value >= 10.0) {
        snprintf(buffer, size, "%.0f%c", value, units[unit]);
    } else {
        snprintf(buffer, size, "%.1f%c", value, units[unit]);
    }
}

static uint64_t block_unit_bytes(const LsOptions *options)
{
    const char *env;
    char *end;
    unsigned long long value;
    uint64_t multiplier = 1;

    if (options->kilobytes) {
        return 1024;
    }

    env = getenv("BLOCKSIZE");
    if (env == NULL || *env == '\0') {
        return 512;
    }

    value = strtoull(env, &end, 10);
    if (end == env || value == 0) {
        return 512;
    }

    if (*end != '\0' && end[1] == '\0') {
        switch (tolower((unsigned char)*end)) {
        case 'k': multiplier = 1024ULL; break;
        case 'm': multiplier = 1024ULL * 1024ULL; break;
        case 'g': multiplier = 1024ULL * 1024ULL * 1024ULL; break;
        default: return 512;
        }
    } else if (*end != '\0') {
        return 512;
    }

    if (value > UINT64_MAX / multiplier) {
        return 512;
    }
    return (uint64_t)value * multiplier;
}

uint64_t display_block_count(uint64_t blocks_512, const LsOptions *options)
{
    uint64_t bytes = blocks_512 * 512ULL;
    uint64_t unit = block_unit_bytes(options);

    return (bytes + unit - 1) / unit;
}

void print_total_blocks(uint64_t blocks_512, const LsOptions *options)
{
    if (options->human_readable) {
        char text[32];
        human_size(blocks_512 * 512ULL, text, sizeof(text));
        printf("total %s\n", text);
    } else {
        printf("total %llu\n",
               (unsigned long long)display_block_count(blocks_512, options));
    }
}

static void print_name(const char *name, NamePrintMode mode)
{
    const char *p = name;
    mbstate_t state;

    if (mode == PRINT_RAW) {
        fputs(name, stdout);
        return;
    }

    memset(&state, 0, sizeof(state));
    while (*p != '\0') {
        wchar_t wc;
        size_t length = mbrtowc(&wc, p, MB_CUR_MAX, &state);

        if (length == (size_t)-1 || length == (size_t)-2) {
            putchar('?');
            ++p;
            memset(&state, 0, sizeof(state));
            continue;
        }
        if (length == 0) {
            break;
        }

        if (iswprint(wc)) {
            fwrite(p, 1, length, stdout);
        } else {
            putchar('?');
        }
        p += length;
    }
}

static char classify_suffix(const FileInfo *info)
{
    mode_t mode = info->st.st_mode;

    if (info->is_symlink && S_ISLNK(mode)) return '@';
    if (S_ISDIR(mode)) return '/';
#ifdef S_ISWHT
    if (S_ISWHT(mode)) return '%';
#endif
    if (S_ISSOCK(mode)) return '=';
    if (S_ISFIFO(mode)) return '|';
    if (S_ISREG(mode) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH))) return '*';
    return '\0';
}

static void print_owner(uid_t uid, int numeric)
{
    struct passwd *pwd;

    if (!numeric && (pwd = getpwuid(uid)) != NULL) {
        printf("%s", pwd->pw_name);
    } else {
        printf("%ju", (uintmax_t)uid);
    }
}

static void print_group(gid_t gid, int numeric)
{
    struct group *grp;

    if (!numeric && (grp = getgrgid(gid)) != NULL) {
        printf("%s", grp->gr_name);
    } else {
        printf("%ju", (uintmax_t)gid);
    }
}

static void print_long(const FileInfo *info, const LsOptions *options)
{
    char modes[11];
    char date_buffer[64];
    char size_buffer[32];
    const struct timespec *selected_time;
    struct tm tm_value;

    mode_string(info->st.st_mode, modes);
    selected_time = file_info_time(info, options->time_mode);

    if (localtime_r(&selected_time->tv_sec, &tm_value) == NULL ||
        strftime(date_buffer, sizeof(date_buffer), "%b %e %H:%M", &tm_value) == 0) {
        snprintf(date_buffer, sizeof(date_buffer), "??? ?? ??:??");
    }

    printf("%s %ju ", modes, (uintmax_t)info->st.st_nlink);
    print_owner(info->st.st_uid, options->numeric_ids);
    putchar(' ');
    print_group(info->st.st_gid, options->numeric_ids);
    putchar(' ');

    if (S_ISCHR(info->st.st_mode) || S_ISBLK(info->st.st_mode)) {
        printf("%u,%u ", major(info->st.st_rdev), minor(info->st.st_rdev));
    } else if (options->human_readable) {
        human_size((uint64_t)info->st.st_size, size_buffer, sizeof(size_buffer));
        printf("%s ", size_buffer);
    } else {
        printf("%jd ", (intmax_t)info->st.st_size);
    }

    printf("%s ", date_buffer);
}

static void print_link_target(const FileInfo *info, const LsOptions *options)
{
    char *buffer;
    ssize_t length;
    size_t size = info->st.st_size > 0 ? (size_t)info->st.st_size + 1 : 256;

    buffer = malloc(size + 1);
    if (buffer == NULL) {
        return;
    }

    for (;;) {
        length = readlink(info->path, buffer, size);
        if (length < 0) {
            free(buffer);
            return;
        }
        if ((size_t)length < size) {
            buffer[length] = '\0';
            break;
        }

        size *= 2;
        {
            char *grown = realloc(buffer, size + 1);
            if (grown == NULL) {
                free(buffer);
                return;
            }
            buffer = grown;
        }
    }

    printf(" -> ");
    print_name(buffer, options->name_mode);
    free(buffer);
}

void print_file_info(const FileInfo *info, const LsOptions *options)
{
    char suffix = '\0';

    if (options->show_inode) {
        printf("%ju ", (uintmax_t)info->st.st_ino);
    }

    if (options->show_blocks) {
        if (options->human_readable) {
            char text[32];
            human_size((uint64_t)info->st.st_blocks * 512ULL, text, sizeof(text));
            printf("%s ", text);
        } else {
            printf("%llu ", (unsigned long long)display_block_count(
                       (uint64_t)info->st.st_blocks, options));
        }
    }

    if (options->long_format) {
        print_long(info, options);
    }

    print_name(info->name, options->name_mode);

    if (options->classify) {
        suffix = classify_suffix(info);
        if (suffix != '\0') {
            putchar(suffix);
        }
    }

    if (options->long_format && info->is_symlink && S_ISLNK(info->st.st_mode)) {
        print_link_target(info, options);
    }

    putchar('\n');
}
