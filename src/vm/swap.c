#include "swap.h"

#include "devices/block.h"
#include "threads/malloc.h"
#include "threads/vaddr.h"
#include "threads/synch.h"
#include "filesys/filesys.h"

#pragma GCC push_options
#pragma GCC optimize ("O0")

#define SECS_IN_PAGE (PGSIZE / BLOCK_SECTOR_SIZE)

struct block *swap_device;
//TODO(keegan): Use a better method than this.
#define SWAP_COUNT 8192
bool* free_swap;

swap_info_t get_free_block(void);

/*! Initialize the swap system. */
void swap_init(void) {
    swap_device = block_get_role(BLOCK_SWAP);

    // TODO(keegan): delete
    free_swap = (bool*)malloc(SWAP_COUNT * sizeof(bool));
    int i;
    for (i=0; i<SWAP_COUNT; i++) {
        free_swap[i] = true;
    }
}

/*! Read a block from swap, specified by info. */
void swap_read(void* kpage, swap_info_t info) {
    filesys_lock();
    int i;
    block_sector_t sector = SECS_IN_PAGE * (uint32_t)info;
    for (i = 0; i < SECS_IN_PAGE; i++) {
        block_read(swap_device,
                   sector + i, 
                   (uint8_t*)kpage + i*BLOCK_SECTOR_SIZE);
    }
    free_swap[info] = true;
    filesys_unlock();
}

/*! Write a page into swap. If successful, sets info to the swap_info
  related to the location of the written page. Returns true on
  success. */
bool swap_write(void* kpage, swap_info_t* info) {
    filesys_lock();
    int i;
    swap_info_t swap_info = get_free_block();
    block_sector_t sector = swap_info * SECS_IN_PAGE;
    for (i = 0; i < SECS_IN_PAGE; i++) {
        block_write(swap_device,
                    sector + i, 
                    (uint8_t*)kpage + i*BLOCK_SECTOR_SIZE);
    }
    *info = swap_info;
    free_swap[swap_info] = false;
    filesys_unlock();
    return true;
}

/*! Return the swap_info_t identifier of a free block of memory. */
swap_info_t get_free_block(void) {
    int i;
    for (i = 0; i < SWAP_COUNT; i++) {
        if (free_swap[i])
            return i;
    }
    PANIC("No free swap blocks");
}

#pragma GCC pop_options
