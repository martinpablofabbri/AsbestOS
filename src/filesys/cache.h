#ifndef FILESYS_CACHE_H
#define FILESYS_CACHE_H

#include "filesys/inode.h"

void cache_init(struct block* block);
void cache_read(block_sector_t sector, void* buffer);
void cache_write(block_sector_t sector, const void *buffer);
void cache_flush(void);

#endif /* filesys/cache.h */

