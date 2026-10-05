#ifndef FORMAT_H
#define FORMAT_H

#include <stdint.h>

#include "fileinfo.h"
#include "options.h"

void print_file_info(const FileInfo *info, const LsOptions *options);
void print_total_blocks(uint64_t blocks_512, const LsOptions *options);
uint64_t display_block_count(uint64_t blocks_512, const LsOptions *options);

#endif
