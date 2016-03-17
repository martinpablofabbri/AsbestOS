#ifndef FILESYS_FREE_MAP_H
#define FILESYS_FREE_MAP_H

#include <stdbool.h>
#include <stddef.h>
#include "devices/block.h"
#include "filesys/inode.h"

void free_map_init(void);
void free_map_read(void);
void free_map_create(void);
void free_map_open(void);
void free_map_close(void);

bool free_map_allocate(size_t, block_sector_t *);
void free_map_release(block_sector_t, size_t);
bool free_map_index_allocate(int sectors, struct inode_disk* inode_disk_);

#endif /* filesys/free-map.h */

