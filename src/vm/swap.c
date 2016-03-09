#include "swap.h"

#include "devices/block.h"
#include "threads/vaddr.h"
#pragma GCC push_options
#pragma GCC optimize ("O0")

#define SECS_IN_PAGE (PGSIZE / BLOCK_SECTOR_SIZE)

struct block *swap_device;
static swap_info_t max_si;

swap_info_t get_free_block(void);

/*! Initialize the swap system. */
void swap_init(void) {
    swap_device = block_get_role(BLOCK_SWAP);

    // TODO(keegan): delete
    max_si = 0;
}

/*! Read a block from swap, specified by info. */
void swap_read(void* kpage, swap_info_t info) {
    int i;
    for (i = 0; i < SECS_IN_PAGE; i++) {
        block_read(swap_device,
                   (block_sector_t)info + i, 
                   (uint8_t*)kpage + i*BLOCK_SECTOR_SIZE);
    }
    return true;
}

/*! Write a page into swap. If successful, sets info to the swap_info
  related to the location of the written page. Returns true on
  success. */
bool swap_write(void* kpage, swap_info_t* info) {
    int i;
    swap_info_t swap_info = get_free_block();
    for (i = 0; i < SECS_IN_PAGE; i++) {
        block_write(swap_device,
                    (block_sector_t)swap_info + i, 
                    (uint8_t*)kpage + i*BLOCK_SECTOR_SIZE);
    }
    *info = swap_info;
    return true;
}

/*! Return the swap_info_t identifier of a free block of memory. */
swap_info_t get_free_block(void) {
    swap_info_t retval = max_si;
    max_si += SECS_IN_PAGE;
    return retval;
}

#pragma GCC pop_options
