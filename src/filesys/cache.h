#ifndef FILESYS_CACHE_H
#define FILESYS_CACHE_H

#include "filesys/inode.h"

void cache_init(struct block* block);
void cache_read(block_sector_t sector, void* buffer, off_t size, off_t offset);
void cache_write(block_sector_t sector, const void *buffer,
                 off_t size, off_t offset);
void cache_flush(void);
void* cache_pin(block_sector_t sector);
void cache_unpin (void* addr);

#endif /* filesys/cache.h */

