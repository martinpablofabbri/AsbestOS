#include "swap.h"
#include "threads/synch.h"

#define SWAP_SIZE 4096 // TODO:???

static struct block *swap_block;

static struct lock sw_lock;

void swap_lock(void);
void swap_unlock(void);

void swap_init() {
    // Create block device
    swap_block = new struct block;
    swap_block->type = BLOCK_SWAP;
    swap_block->name = block_type_name(BLOCK_SWAP);
    swap_block->size = SWAP_SIZE;

    // Set swap_block->read_cnt/write_cnt
    swap_block->read_cnt = 0;
    swap_block->write_cnt = 0;
}

/*! copies the page at kaddr into the frame.*/
void swap_read(void *kaddr, void *frame) {
    swap_lock();

    // TODO:Do IO access
    // block_read(swap_block, (block_sector_t) kaddr, )

    swap_unlock();
}

/*! Returns the location in swap where the data was written to.*/
void * swap_write(void *data) {
    swap_lock();

    // TODO:Do IO access

    swap_unlock();
}

void swap_lock(void) {
    lock_aquire(&sw_lock);
}

void swap_unlock(void) {
    lock_release(&sw_lock);
}
