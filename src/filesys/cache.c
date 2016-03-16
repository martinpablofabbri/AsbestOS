#include "cache.h"

struct block* fs_block;

/*! Initialize the cache. */
void cache_init(struct block* block) {
    fs_block = block;
}

/*! Read a block from the cache, bringing the page in from the disk if
  necessary. */
void cache_read(block_sector_t sector, void* buffer) {
    block_read(fs_block, sector, buffer);
}

/*! Write information to the cache. */
void cache_write(block_sector_t sector, const void *buffer) {
    block_write(fs_block, sector, buffer);
}
